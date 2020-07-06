/*------------------------------GPL---------------------------------------//
// This file is part of HMDF.
//
// (c) 2015-2020 Zachary Cobell
//
// HMDF is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// HMDF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HMDF.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------*/
#include "hmdf.h"

#include <netcdf.h>

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/config/warning_disable.hpp"
#include "boost/spirit/include/phoenix.hpp"
#include "boost/spirit/include/qi.hpp"
#include "cdate.h"
#include "logging.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

Hmdf::Hmdf() : m_success(false), m_null(true), m_dimension(0), m_epsg(4326) {}

std::vector<std::string> Hmdf::headerData() const { return m_headerData; }

void Hmdf::setHeaderData(const std::vector<std::string> &headerData) {
  m_headerData = headerData;
}

bool Hmdf::success() const { return m_success; }

bool Hmdf::null() const { return m_null; }

int Hmdf::dimension() const { return m_dimension; }

size_t Hmdf::nStations() const { return this->m_stations.size(); }

void Hmdf::operator<<(const Station &s) { this->m_stations.push_back(s); }

void Hmdf::operator<<(const std::vector<Station> &s) {
  this->m_stations.insert(this->m_stations.end(), s.begin(), s.end());
}

void Hmdf::sanitize() {
  for (auto &s : this->m_stations) {
    s.sanitize();
  }
}

Hmdf::iterator Hmdf::begin() noexcept { return this->m_stations.begin(); }

Hmdf::const_iterator Hmdf::cbegin() const noexcept {
  return this->m_stations.cbegin();
}

Hmdf::iterator Hmdf::end() noexcept { return this->m_stations.end(); }

Hmdf::const_iterator Hmdf::cend() const noexcept {
  return this->m_stations.cend();
}

std::string Hmdf::getFileExtension(const std::string &filename) {
  size_t pos = filename.rfind('.');
  if (pos != std::string::npos) {
    return filename.substr(pos);
  }
  return std::string();
}

Station *Hmdf::station(size_t index) {
  assert(index < this->m_stations.size());
  return &this->m_stations[index];
}

int Hmdf::setStation(size_t index, Station &s) {
  assert(index < this->m_stations.size());
  this->m_stations[index] = s;
  return 0;
}

int Hmdf::addStation(Station &s) {
  this->m_stations.push_back(s);
  return 0;
}

int Hmdf::reproject(int epsg) {
  for (auto &s : this->m_stations) {
    s.reproject(epsg);
  }
  return 0;
}

int Hmdf::resize(size_t n) {
  this->m_stations.resize(n);
  return 0;
}

int Hmdf::read(const std::string &filename, const CDate &coldstart,
               const std::string &stationFile) {
  int ierr;
  switch (this->getFiletype(filename)) {
    case AdcircAscii:
      ierr = this->readAdcircAscii(filename, stationFile, coldstart);
      break;
    case AdcircNetCDF:
      ierr = this->readAdcircNetCDF(filename, coldstart);
      break;
    case Delft3D:
      ierr = this->readDelft3D(filename);
      break;
    case DFlowFM:
      ierr = this->readDFlowFM(filename);
      break;
    case IMEDS:
      ierr = this->readImeds(filename);
      break;
    case NETCDF:
      ierr = this->readgenericNetCDF(filename);
      break;
    default:
      ierr = 1;
      hmdf_throw_exception("Unknown file type. Cannot read to HMDF structure.");
      break;
  }
  return ierr;
}

int Hmdf::write(const std::string &filename) { return 0; }

Hmdf::FileType Hmdf::getFiletype(const std::string &filename) {
  std::string ext = Hmdf::getFileExtension(filename);
  boost::algorithm::to_lower(ext);
  if (ext == ".61" || ext == ".62" || ext == ".71" || ext == ".72") {
    return Hmdf::AdcircAscii;
  } else if (ext == ".nc") {
    return Hmdf::AdcircNetCDF;
  } else if (ext == ".imeds") {
    return Hmdf::IMEDS;
  } else if (ext == ".dat" || ext == ".def") {
    return Hmdf::Delft3D;
  }
  return Hmdf::None;
}

size_t Hmdf::readAdcircStationFile(const std::string &filename,
                                   std::vector<double> &x,
                                   std::vector<double> &y) {
  std::fstream f(filename);
  if (f.bad()) {
    hmdf_throw_exception("Could not open the ADCIRC station file");
    return 1;
  }

  std::string line;
  std::getline(f, line);
  size_t nsta = stoull(line);
  x.reserve(nsta);
  y.reserve(nsta);
  for (size_t i = 0; i < nsta; ++i) {
    std::getline(f, line);
    double xx, yy;
    bool noerr =
        qi::phrase_parse(line.begin(), line.end(),
                         (qi::double_[phoenix::ref(xx) = qi::_1] >> "," >>
                          qi::double_[phoenix::ref(yy) = qi::_1]),
                         ascii::space);
    if (!noerr) {
      hmdf_throw_exception("Error reading ADCIRC station file: " + line);
      return -1;
    }
    x.push_back(xx);
    y.push_back(yy);
  }

  f.close();

  return nsta;
}

int Hmdf::readAdcircAscii(const std::string &filename,
                          const std::string &stationFile,
                          const CDate &coldstart) {
  std::vector<double> x, y;
  size_t nsta = this->readAdcircStationFile(stationFile, x, y);

  std::fstream f(filename);
  if (!f.good()) {
    hmdf_throw_exception("Could not open ADCIRC data file");
    return 1;
  }

  std::string line;

  //...Read fort.61 header
  std::getline(f, line);
  this->m_headerData.push_back(line);

  //...Read the metadata line
  size_t nstep, nsta_file, dim, dit;
  double dt;
  std::getline(f, line);
  bool noerr = qi::phrase_parse(line.begin(), line.end(),
                                (qi::int_[phoenix::ref(nstep) = qi::_1] >>
                                 qi::int_[phoenix::ref(nsta_file) = qi::_1] >>
                                 qi::double_[phoenix::ref(dt) = qi::_1] >>
                                 qi::int_[phoenix::ref(dit) = qi::_1] >>
                                 qi::int_[phoenix::ref(dim) = qi::_1]),
                                ascii::space);
  if (!noerr) {
    hmdf_throw_exception("Error reading the ADCIRC ASCII header line");
    return 1;
  }

  if (nsta_file != nsta) {
    hmdf_throw_exception(
        "The number of stations in the station file and ADCIRC file do not "
        "match.");
    return 1;
  }

  this->m_stations.resize(nsta);
  for (size_t i = 0; i < nsta; ++i) {
    this->m_stations[i] = Station(i, x[i], y[i]);
    this->m_stations[i].allocate(nstep);
  }

  //...Read the body of the file
  for (size_t i = 0; i < nstep; ++i) {
    std::getline(f, line);
    double t;
    size_t it;
    noerr = qi::phrase_parse(line.begin(), line.end(),
                             (qi::double_[phoenix::ref(t) = qi::_1] >>
                              qi::int_[phoenix::ref(it) = qi::_1]),
                             ascii::space);
    CDate d = coldstart;
    d.addSeconds(t);
    for (size_t j = 0; j < nsta; ++j) {
      std::getline(f, line);
      double v1, v2, v3;
      size_t idx;
      if (dim == 1) {
        noerr = qi::phrase_parse(line.begin(), line.end(),
                                 (qi::int_[phoenix::ref(idx) = qi::_1] >>
                                  qi::double_[phoenix::ref(v1) = qi::_1]),
                                 ascii::space);
        if (v1 <= -9999) v1 = Timepoint::nullValue();
        this->m_stations[j] << Timepoint(d, v1);
      } else if (dim == 2) {
        noerr = qi::phrase_parse(line.begin(), line.end(),
                                 (qi::int_[phoenix::ref(idx) = qi::_1] >>
                                  qi::double_[phoenix::ref(v1) = qi::_1] >>
                                  qi::double_[phoenix::ref(v2) = qi::_1]),
                                 ascii::space);
        if (v1 <= -9999) v1 = Timepoint::nullValue();
        if (v2 <= -9999) v2 = Timepoint::nullValue();
        this->m_stations[j] << Timepoint(d, v1, v2);
      } else if (dim == 3) {
        noerr = qi::phrase_parse(line.begin(), line.end(),
                                 (qi::int_[phoenix::ref(idx) = qi::_1] >>
                                  qi::double_[phoenix::ref(v1) = qi::_1] >>
                                  qi::double_[phoenix::ref(v2) = qi::_1] >>
                                  qi::double_[phoenix::ref(v3) = qi::_1]),
                                 ascii::space);
        if (v1 <= -9999) v1 = Timepoint::nullValue();
        if (v2 <= -9999) v2 = Timepoint::nullValue();
        if (v3 <= -9999) v3 = Timepoint::nullValue();
        this->m_stations[j] << Timepoint(d, v1, v2, v3);
      }
    }
  }

  f.close();

  return 0;
}

void Hmdf::ncCheck(const int retcode) {
  if (retcode == NC_NOERR) return;
  hmdf_throw_exception(nc_strerror(retcode));
}

int Hmdf::getAdcircVariableId(const int ncid, int &varid1, int &varid2) {
  const std::array<std::string, 4> vlist = {"zeta", "u-vel", "pressure",
                                            "windx"};
  for (auto &v : vlist) {
    int ierr = nc_inq_varid(ncid, v.c_str(), &varid1);
    if (ierr != NC_NOERR) continue;
    if (v == "u-vel") {
      ierr = nc_inq_varid(ncid, "v-vel", &varid2);
      return 2;
    } else if (v == "windx") {
      ierr = nc_inq_varid(ncid, "windy", &varid2);
      return 2;
    }
    return 1;
  }
  return 0;
}

int Hmdf::readAdcircNetCDF(const std::string &filename,
                           const CDate &coldstart) {
  int ncid;
  int dimid_time, dimid_nsta, dimid_namelen;
  int varid_time, varid_x, varid_y, varid_staname;
  ncCheck(nc_open(filename.c_str(), NC_NOWRITE, &ncid));
  ncCheck(nc_inq_dimid(ncid, "time", &dimid_time));
  ncCheck(nc_inq_dimid(ncid, "station", &dimid_nsta));
  ncCheck(nc_inq_dimid(ncid, "namelen", &dimid_namelen));
  ncCheck(nc_inq_varid(ncid, "time", &varid_time));
  ncCheck(nc_inq_varid(ncid, "x", &varid_x));
  ncCheck(nc_inq_varid(ncid, "y", &varid_y));
  ncCheck(nc_inq_varid(ncid, "station_name", &varid_staname));

  size_t nsnap;
  ncCheck(nc_inq_dimlen(ncid, dimid_time, &nsnap));
  std::unique_ptr<double[]> t(new double[nsnap]);
  ncCheck(nc_get_var(ncid, varid_time, t.get()));
  std::vector<CDate> date(nsnap);
  for (size_t i = 0; i < nsnap; ++i) {
    date[i] = coldstart + t[i];
  }
  t.reset();

  size_t nsta;
  ncCheck(nc_inq_dimlen(ncid, dimid_nsta, &nsta));
  std::unique_ptr<double[]> x(new double[nsta]);
  std::unique_ptr<double[]> y(new double[nsta]);
  ncCheck(nc_get_var(ncid, varid_x, x.get()));
  ncCheck(nc_get_var(ncid, varid_y, y.get()));

  size_t stanamelen;
  ncCheck(nc_inq_dimlen(ncid, dimid_namelen, &stanamelen));
  std::unique_ptr<char[]> stnname(new char[nsta * stanamelen]);
  ncCheck(nc_get_var(ncid, varid_staname, stnname.get()));
  std::vector<std::string> n;
  for (size_t i = 0; i < nsta; ++i) {
    std::string a(stnname.get() + stanamelen * i,
                  stnname.get() + (i + 1) * stanamelen);
    n.push_back(a);
  }

  //...Get the varid's
  int varid1, varid2;
  int nvar = this->getAdcircVariableId(ncid, varid1, varid2);
  if (nvar == 0) {
    hmdf_throw_exception("No valid variables found in ADCIRC netCDF file");
    return 1;
  }

  for (size_t i = 0; i < nsta; ++i) {
    this->m_stations.push_back(Station(i, x[i], y[i], nvar));
    this->m_stations.back().allocate(nsnap);
    this->m_stations.back().setName(n[i]);
  }

  for (size_t i = 0; i < nsta; ++i) {
    size_t istart[2] = {0, i};
    size_t icount[2] = {nsnap, 1};
    if (nvar == 1) {
      std::unique_ptr<double[]> d1(new double[nsnap]);
      ncCheck(nc_get_varm(ncid, varid1, istart, icount, nullptr, nullptr,
                          d1.get()));
      for (size_t j = 0; j < nsnap; j++) {
        this->m_stations[i] << Timepoint(date[j], d1[j]);
      }
    } else if (nvar == 2) {
      std::unique_ptr<double[]> d1(new double[nsnap]);
      std::unique_ptr<double[]> d2(new double[nsnap]);
      ncCheck(nc_get_varm(ncid, varid1, istart, icount, nullptr, nullptr,
                          d1.get()));
      ncCheck(nc_get_varm(ncid, varid2, istart, icount, nullptr, nullptr,
                          d2.get()));
      for (size_t j = 0; j < nsnap; j++) {
        this->m_stations[i] << Timepoint(date[j], d1[j], d2[j]);
      }
    }
  }

  ncCheck(nc_close(ncid));
  return 0;
}

void Hmdf::splitString(std::string &data, std::vector<std::string> &fresult) {
  boost::trim_if(data, boost::is_any_of(" ,"));
  boost::algorithm::split(fresult, data, boost::is_any_of(" ,"),
                          boost::token_compress_on);
  return;
}

std::string Hmdf::sanitizeString(const std::string &a) {
  std::string b = a;
  boost::algorithm::trim(b);
  boost::algorithm::replace_all(b, "\t", " ");
  b.erase(std::remove(b.begin(), b.end(), '\r'), b.end());
  return b;
}

bool Hmdf::splitStringHmdfFormat(const std::string &data, int &year, int &month,
                                 int &day, int &hour, int &minute, int &second,
                                 double &value) {
  bool r = qi::phrase_parse(data.begin(), data.end(),
                            qi::int_[phoenix::ref(year) = qi::_1] >>
                                qi::int_[phoenix::ref(month) = qi::_1] >>
                                qi::int_[phoenix::ref(day) = qi::_1] >>
                                qi::int_[phoenix::ref(hour) = qi::_1] >>
                                qi::int_[phoenix::ref(minute) = qi::_1] >>
                                qi::int_[phoenix::ref(second) = qi::_1] >>
                                qi::double_[phoenix::ref(value) = qi::_1],
                            qi::space);
  if (!r) {
    r = qi::phrase_parse(data.begin(), data.end(),
                         qi::int_[phoenix::ref(year) = qi::_1] >>
                             qi::int_[phoenix::ref(month) = qi::_1] >>
                             qi::int_[phoenix::ref(day) = qi::_1] >>
                             qi::int_[phoenix::ref(hour) = qi::_1] >>
                             qi::int_[phoenix::ref(minute) = qi::_1] >>
                             qi::double_[phoenix::ref(value) = qi::_1],
                         qi::space);
    second = 0;
  }

  return r;
}

int Hmdf::readImeds(const std::string &filename) {
  std::ifstream fid(filename.c_str());
  if (fid.bad()) return 1;

  //...Read Header
  std::string templine;
  std::getline(fid, templine);
  this->m_headerData.push_back(templine);
  std::getline(fid, templine);
  this->m_headerData.push_back(templine);
  std::getline(fid, templine);
  this->m_headerData.push_back(templine);

  //...Read Body
  std::getline(fid, templine);

  size_t id = 0;
  while (!fid.eof()) {
    templine = Hmdf::sanitizeString(templine);

    std::vector<std::string> templist;
    Hmdf::splitString(templine, templist);

    Station s(id, stod(templist[2]), stod(templist[1]), 1, 4326);
    s.setName(templist[0]);

    while (!fid.eof()) {
      std::getline(fid, templine);
      int year, month, day, hour, minute, second;
      double value;
      bool status = Hmdf::splitStringHmdfFormat(templine, year, month, day,
                                                hour, minute, second, value);
      if (status) {
        CDate d(year, month, day, hour, minute, second);
        if (value <= -9999) value = Timepoint::nullValue();
        s << Timepoint(d, value);
      } else {
        break;
      }
    }
    this->m_stations.push_back(s);
  }
  return 0;
}

int Hmdf::readgenericNetCDF(const std::string &filename) {}

int Hmdf::readDelft3D(const std::string &filename) {}

int Hmdf::readDFlowFM(const std::string &filename) {}
