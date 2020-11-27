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
#include <limits>
#include <memory>
#include <utility>

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/config/warning_disable.hpp"
#include "boost/format.hpp"
#include "boost/spirit/include/phoenix.hpp"
#include "boost/spirit/include/qi.hpp"
#include "date.h"
#include "logging.h"
#include "nefis_defines.h"
#include "nefisseriesmetadata.h"

extern "C" {
#include "btps.h"
#include "nefis.h"
}

#define MAX_NEFIS_CEL_DIM 100
#define MAX_NEFIS_DESC 64
#define MAX_NEFIS_DIM 5
#define MAX_NEFIS_NAME 16
#define MAX_NEFIS_TYPE 8

using namespace Hmdf;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

HmdfData::HmdfData(const std::string &filename, const Date &coldstart,
                   const std::string &stationFile)
    : m_filename(std::move(filename)),
      m_coldstart(coldstart),
      m_stationFile(std::move(stationFile)),
      m_success(false),
      m_null(true),
      m_dimension(0),
      m_epsg(4326) {}

std::vector<std::string> HmdfData::headerData() const { return m_headerData; }

void HmdfData::setHeaderData(const std::vector<std::string> &headerData) {
  m_headerData = headerData;
}

bool HmdfData::success() const { return m_success; }

bool HmdfData::null() const { return m_null; }

int HmdfData::dimension() const { return m_dimension; }

size_t HmdfData::nStations() const { return this->m_stations.size(); }

void HmdfData::operator<<(const Station &s) { this->m_stations.push_back(s); }

void HmdfData::operator<<(const std::vector<Station> &s) {
#ifndef USE_QT_TYPES
  this->m_stations.insert(this->m_stations.end(), s.begin(), s.end());
#else
  this->m_stations.append(s);
#endif
}

void HmdfData::sanitize() {
  for (auto &s : this->m_stations) {
    s.sanitize();
  }
}

HmdfData::iterator HmdfData::begin() noexcept {
  return this->m_stations.begin();
}

HmdfData::const_iterator HmdfData::cbegin() const noexcept {
  return this->m_stations.cbegin();
}

HmdfData::iterator HmdfData::end() noexcept { return this->m_stations.end(); }

HmdfData::const_iterator HmdfData::cend() const noexcept {
  return this->m_stations.cend();
}

std::string HmdfData::getFileExtension(const std::string &filename) {
  size_t pos = filename.rfind('.');
  if (pos != std::string::npos) {
    return filename.substr(pos);
  }
  return std::string();
}

std::string HmdfData::getFileBasename(const std::string &filename) {
  size_t pos = filename.rfind(".");
  if (pos != std::string::npos) {
    return std::string(filename.begin(), filename.begin() + pos);
  }
  return std::string();
}

Station *HmdfData::station(size_t index) {
  assert(index < this->m_stations.size());
  return &this->m_stations[index];
}

int HmdfData::setStation(size_t index, Station &s) {
  assert(index < this->m_stations.size());
  this->m_stations[index] = s;
  return 0;
}

int HmdfData::moveStation(Station s) {
  this->m_stations.push_back(std::move(s));
  return 0;
}

int HmdfData::addStation(const Station &s) {
  this->m_stations.push_back(s);
  return 0;
}

void HmdfData::deleteStation(const size_t index) {
  this->m_stations.erase(this->m_stations.begin() + index);
}

int HmdfData::reproject(int epsg) {
  for (auto &s : this->m_stations) {
    s.reproject(epsg);
  }
  return 0;
}

void HmdfData::setEpsg(int epsg) {
  this->m_epsg = epsg;
  for (auto &s : this->m_stations) {
    s.setEpsg(epsg);
  }
}

int HmdfData::resize(size_t n) {
  this->m_stations.resize(n);
  return 0;
}

int HmdfData::read() {
  int ierr;

  std::fstream fl(this->m_filename);
  if (fl.bad()) {
    hmdf_throw_exception("The file " + this->m_filename + " does not exist.");
  }

  switch (HmdfData::getFiletype(this->m_filename)) {
    case AdcircAscii:
      ierr = this->readAdcircAscii();
      break;
    case AdcircNetCDF:
      ierr = this->readAdcircNetCDF();
      break;
    case Delft3D:
      ierr = this->readNefisHeader();
      break;
    case DFlowFM:
      ierr = this->readDflowFmHeader();
      break;
    case IMEDS:
      ierr = this->readImeds();
      break;
    case NETCDF:
      ierr = this->readGenericNetcdf();
      break;
    default:
      ierr = 1;
      hmdf_throw_exception("Unknown file type. Cannot read to HMDF structure.");
      break;
  }
  return ierr;
}

int HmdfData::write(const std::string &filename) { return 0; }

HmdfData::FileType HmdfData::getFiletype(const std::string &filename) {
  std::string ext = HmdfData::getFileExtension(filename);
  boost::algorithm::to_lower(ext);
  if (ext == ".61" || ext == ".62" || ext == ".71" || ext == ".72") {
    return HmdfData::AdcircAscii;
  }
  if (ext == ".nc") {
    return HmdfData::checkNetcdfType(filename);
  } else if (ext == ".imeds") {
    return HmdfData::IMEDS;
  } else if (ext == ".dat" || ext == ".def") {
    return HmdfData::Delft3D;
  }
  return HmdfData::None;
}

size_t HmdfData::readAdcircStationFile(const std::string &filename,
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
    double xx;
    double yy;
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

int HmdfData::readAdcircAscii() {
  std::vector<double> x;
  std::vector<double> y;
  size_t nsta = this->readAdcircStationFile(this->m_stationFile, x, y);

  std::fstream f(this->m_filename);
  if (!f.good()) {
    hmdf_throw_exception("Could not open ADCIRC data file");
    return 1;
  }

  std::string line;

  //...Read fort.61 header
  std::getline(f, line);
  this->m_headerData.push_back(line);

  //...Read the metadata line
  size_t nstep;
  size_t nsta_file;
  size_t dim;
  size_t dit;
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
    Date d(this->m_coldstart);
    d.addSeconds(t);
    for (size_t j = 0; j < nsta; ++j) {
      std::getline(f, line);
      double v1;
      double v2;
      double v3;
      size_t idx;
      if (dim == 1) {
        noerr = qi::phrase_parse(line.begin(), line.end(),
                                 (qi::int_[phoenix::ref(idx) = qi::_1] >>
                                  qi::double_[phoenix::ref(v1) = qi::_1]),
                                 ascii::space);
        if (v1 <= -9999) {
          v1 = Timepoint::nullValue();
        }
        this->m_stations[j] << Timepoint(d, v1);
      } else if (dim == 2) {
        noerr = qi::phrase_parse(line.begin(), line.end(),
                                 (qi::int_[phoenix::ref(idx) = qi::_1] >>
                                  qi::double_[phoenix::ref(v1) = qi::_1] >>
                                  qi::double_[phoenix::ref(v2) = qi::_1]),
                                 ascii::space);
        if (v1 <= -9999) {
          v1 = Timepoint::nullValue();
        }
        if (v2 <= -9999) {
          v2 = Timepoint::nullValue();
        }
        this->m_stations[j] << Timepoint(d, v1, v2);
      } else if (dim == 3) {
        noerr = qi::phrase_parse(line.begin(), line.end(),
                                 (qi::int_[phoenix::ref(idx) = qi::_1] >>
                                  qi::double_[phoenix::ref(v1) = qi::_1] >>
                                  qi::double_[phoenix::ref(v2) = qi::_1] >>
                                  qi::double_[phoenix::ref(v3) = qi::_1]),
                                 ascii::space);
        if (v1 <= -9999) {
          v1 = Timepoint::nullValue();
        }
        if (v2 <= -9999) {
          v2 = Timepoint::nullValue();
        }
        if (v3 <= -9999) {
          v3 = Timepoint::nullValue();
        }
        this->m_stations[j] << Timepoint(d, v1, v2, v3);
      }
    }
  }

  f.close();

  return 0;
}

void HmdfData::ncCheck(const int retcode, const int fid) {
  if (retcode == NC_NOERR) {
    return;
  }
  if (fid != -1) {
    ncclose(fid);
  }
  hmdf_throw_exception(nc_strerror(retcode));
}

void HmdfData::nefCheck(const int retcode, int fid) {
  if (retcode == 0) {
    return;
  }
  if (fid != -1) {
    Clsnef(&fid);
  }
  char neferr[1024];
  Neferr(2, neferr);
  hmdf_throw_exception("Internal error in NEFIS library: Code " +
                       std::to_string(retcode));
}

int HmdfData::getAdcircVariableId(const int ncid, int &varid1, int &varid2) {
  const std::array<std::string, 4> vlist = {"zeta", "u-vel", "pressure",
                                            "windx"};
  for (auto &v : vlist) {
    int ierr = nc_inq_varid(ncid, v.c_str(), &varid1);
    if (ierr != NC_NOERR) {
      continue;
    }
    if (v == "u-vel") {
      ierr = nc_inq_varid(ncid, "v-vel", &varid2);
      return 2;
    }
    if (v == "windx") {
      ierr = nc_inq_varid(ncid, "windy", &varid2);
      return 2;
    }
    return 1;
  }
  return 0;
}

std::string HmdfData::getStationFile() const { return m_stationFile; }

void HmdfData::setStationFile(const std::string &stationFile) {
  m_stationFile = stationFile;
}

Date HmdfData::getColdstart() const { return m_coldstart; }

void HmdfData::setColdstart(const Date &coldstart) { m_coldstart = coldstart; }

std::string HmdfData::getFilename() const { return m_filename; }

void HmdfData::setFilename(const std::string &filename) {
  m_filename = filename;
}

int HmdfData::readAdcircNetCDF() {
  int ncid;
  int dimid_time;
  int dimid_nsta;
  int dimid_namelen;
  int varid_time;
  int varid_x;
  int varid_y;
  int varid_staname;
  ncCheck(nc_open(this->m_filename.c_str(), NC_NOWRITE, &ncid));
  ncCheck(nc_inq_dimid(ncid, "time", &dimid_time), ncid);
  ncCheck(nc_inq_dimid(ncid, "station", &dimid_nsta), ncid);
  ncCheck(nc_inq_dimid(ncid, "namelen", &dimid_namelen), ncid);
  ncCheck(nc_inq_varid(ncid, "time", &varid_time), ncid);
  ncCheck(nc_inq_varid(ncid, "x", &varid_x), ncid);
  ncCheck(nc_inq_varid(ncid, "y", &varid_y), ncid);
  ncCheck(nc_inq_varid(ncid, "station_name", &varid_staname), ncid);

  size_t nsnap;
  ncCheck(nc_inq_dimlen(ncid, dimid_time, &nsnap), ncid);
  std::vector<double> t(nsnap);
  ncCheck(nc_get_var(ncid, varid_time, t.data()), ncid);
  std::vector<Date> date(nsnap);
  for (size_t i = 0; i < nsnap; ++i) {
    date[i] = this->m_coldstart + t[i];
  }
  t.clear();

  size_t nsta;
  ncCheck(nc_inq_dimlen(ncid, dimid_nsta, &nsta), ncid);
  std::vector<double> x(nsta);
  std::vector<double> y(nsta);
  ncCheck(nc_get_var(ncid, varid_x, x.data()), ncid);
  ncCheck(nc_get_var(ncid, varid_y, y.data()), ncid);

  size_t stanamelen;
  ncCheck(nc_inq_dimlen(ncid, dimid_namelen, &stanamelen), ncid);
  std::vector<char> stnname(nsta * stanamelen);
  ncCheck(nc_get_var(ncid, varid_staname, stnname.data()), ncid);
  std::vector<std::string> n;
  for (size_t i = 0; i < nsta; ++i) {
    std::string a(stnname.data() + stanamelen * i,
                  stnname.data() + (i + 1) * stanamelen);
    n.push_back(a);
  }

  //...Get the varid's
  int varid1;
  int varid2;
  int nvar = this->getAdcircVariableId(ncid, varid1, varid2);
  if (nvar == 0) {
    hmdf_throw_exception("No valid variables found in ADCIRC netCDF file");
    return 1;
  }

  for (size_t i = 0; i < nsta; ++i) {
    this->m_stations.emplace_back(i, x[i], y[i], nvar);
    this->m_stations.back().allocate(nsnap);
    this->m_stations.back().setName(n[i]);
  }

  for (size_t i = 0; i < nsta; ++i) {
    size_t istart[2] = {0, i};
    size_t icount[2] = {nsnap, 1};
    if (nvar == 1) {
      std::vector<double> d1(nsnap);
      ncCheck(nc_get_varm(ncid, varid1, istart, icount, nullptr, nullptr,
                          d1.data()),
              ncid);
      for (size_t j = 0; j < nsnap; j++) {
        this->m_stations[i] << Timepoint(date[j], d1[j]);
      }
    } else if (nvar == 2) {
      std::vector<double> d1(nsnap);
      std::vector<double> d2(nsnap);
      ncCheck(nc_get_varm(ncid, varid1, istart, icount, nullptr, nullptr,
                          d1.data()),
              ncid);
      ncCheck(nc_get_varm(ncid, varid2, istart, icount, nullptr, nullptr,
                          d2.data()),
              ncid);
      for (size_t j = 0; j < nsnap; j++) {
        this->m_stations[i] << Timepoint(date[j], d1[j], d2[j]);
      }
    }
  }

  ncCheck(nc_close(ncid));
  return 0;
}

void HmdfData::splitString(std::string &data,
                           std::vector<std::string> &fresult) {
  boost::trim_if(data, boost::is_any_of(" ,"));
  boost::algorithm::split(fresult, data, boost::is_any_of(" ,"),
                          boost::token_compress_on);
}

std::string HmdfData::sanitizeString(const std::string &a) {
  std::string b = a;
  boost::algorithm::trim(b);
  boost::algorithm::replace_all(b, "\t", " ");
  b.erase(std::remove(b.begin(), b.end(), '\r'), b.end());
  return b;
}

bool HmdfData::splitStringHmdfFormat(const std::string &data, int &year,
                                     int &month, int &day, int &hour,
                                     int &minute, int &second, double &value) {
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

HmdfData::FileType HmdfData::checkNetcdfType(const std::string &filename) {
  int ncid;
  HmdfData::ncCheck(nc_open(filename.c_str(), NC_NOWRITE, &ncid));
  int vid;
  double slam0;
  int ierr_genericNc = nc_inq_varid(ncid, "time_station_0001", &vid);
  int ierr_adcirc = nc_get_att_double(ncid, NC_GLOBAL, "slam0", &slam0);
  FileType retType;
  if (ierr_genericNc == NC_NOERR) {
    retType = HmdfData::FileType::NETCDF;
  } else if (ierr_adcirc == NC_NOERR) {
    retType = HmdfData::FileType::AdcircNetCDF;
  } else {
    retType = HmdfData::FileType::None;
  }
  HmdfData::ncCheck(nc_close(ncid));
  return retType;
}

int HmdfData::readImeds() {
  std::ifstream fid(this->m_filename.c_str());
  if (fid.bad()) {
    return 1;
  }

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
    templine = HmdfData::sanitizeString(templine);

    std::vector<std::string> templist;
    HmdfData::splitString(templine, templist);

    Station s(id, stod(templist[2]), stod(templist[1]), 1, 4326);
    s.setName(templist[0]);

    while (!fid.eof()) {
      std::getline(fid, templine);
      int year;
      int month;
      int day;
      int hour;
      int minute;
      int second;
      double value;
      bool status = HmdfData::splitStringHmdfFormat(
          templine, year, month, day, hour, minute, second, value);
      if (status) {
        Date d(year, month, day, hour, minute, second);
        if (value <= -9999) {
          value = Timepoint::nullValue();
        }
        s << Timepoint(d, value);
      } else {
        break;
      }
    }
    this->m_stations.push_back(s);
  }
  return 0;
}

void HmdfData::getNefisDatasets(const char *series,
                                std::vector<NefisSeriesMetadata> &metadata,
                                const size_t numStations) {
  const std::string basename = this->getFileBasename(this->m_filename);
  BText datfile = strdup(std::string(basename + ".def").c_str());
  BText deffile = strdup(std::string(basename + ".dat").c_str());

  BInt4 fid;
  HmdfData::nefCheck(Crenef(&fid, deffile, datfile, 'M', 'r'));

  BInt4 grpdim = MAX_NEFIS_DIM;
  BInt4 celdim = MAX_NEFIS_CEL_DIM;
  BInt4 nsteps = 0;
  std::vector<BInt4> grpdms(MAX_NEFIS_DIM);
  std::vector<BInt4> grpord(MAX_NEFIS_DIM);
  std::vector<BInt4> elmdimensions(MAX_NEFIS_DIM);
  std::vector<BChar> celname(MAX_NEFIS_NAME + 1);
  std::vector<BChar> type(MAX_NEFIS_TYPE + 1);
  std::vector<BChar> quantity(MAX_NEFIS_NAME + 1);
  std::vector<BChar> units(MAX_NEFIS_NAME + 1);
  std::vector<BChar> description(MAX_NEFIS_DESC + 1);
  BChar elmnames[MAX_NEFIS_CEL_DIM][MAX_NEFIS_NAME + 1];
  BChar *bseries;
  bseries = strdup(series);

  int ierr = Inqmxi(&fid, bseries, &nsteps);
  if (ierr != 0) {
    Clsnef(&fid);
    return;
  }

  HmdfData::nefCheck(Inqgrp(&fid, bseries, celname.data(), &grpdim,
                            grpdms.data(), grpord.data()),
                     fid);
  HmdfData::nefCheck(Inqcel(&fid, celname.data(), &celdim, elmnames), fid);

  for (int i = 0; i < celdim; ++i) {
    NefisSeriesMetadata se;
    se.setName(HmdfData::sanitizeString(elmnames[i]));
    BInt4 ndim = MAX_NEFIS_DIM;
    BInt4 nbyte = 0;
    HmdfData::nefCheck(
        Inqelm(&fid, elmnames[i], type.data(), &nbyte, quantity.data(),
               units.data(), description.data(), &ndim, elmdimensions.data()),
        fid);
    se.setDescription(HmdfData::sanitizeString(description.data()));
    se.setType(HmdfData::sanitizeString(type.data()));
    se.setUnits(HmdfData::sanitizeString(units.data()));
    se.setQuantity(HmdfData::sanitizeString(quantity.data()));
    se.setFromSeries(series);
    se.dim()->reserve(ndim);
    for (int j = 0; j < ndim; ++j) {
      se.dim()->push_back(elmdimensions[j]);
    }

    if (se.dim()->at(0) == numStations) {
      metadata.push_back(se);
    }
  }

  HmdfData::nefCheck(Clsnef(&fid));
  return;
}

int HmdfData::readNefisHeader() {
  std::string layerModel;
  this->m_nefisMetadata.clear();
  size_t nsta = this->getNefisStations();
  this->getNefisTimes(this->m_nefisTimes);
  this->m_nefisLayers = this->getNefisLayers(layerModel);
  this->getNefisDatasets("his-series", this->m_nefisMetadata, nsta);
  this->getNefisDatasets("his-wave-series", this->m_nefisMetadata, nsta);
  return 0;
}

size_t HmdfData::getNefisVarIndex(const std::string &var) {
  for (size_t i = 0; i < this->m_nefisMetadata.size(); ++i) {
    if (this->m_nefisMetadata[i].name() == var) {
      return i;
    }
  }
  return std::numeric_limits<size_t>::max();
}

int HmdfData::readNefisValue(const std::string &var, size_t layer) {
  size_t idx = this->getNefisVarIndex(var);
  if (idx == std::numeric_limits<size_t>::max()) {
    return 1;
  }

  if (layer != 0) {
    if (this->m_nefisMetadata[idx].dim()->size() != 3) {
      return 1;
    }
    if (layer - 1 > this->m_nefisMetadata[idx].dim()->at(2)) {
      return 1;
    }
  } else {
    layer = 1;
  }

  BText src = strdup(this->m_nefisMetadata[idx].fromSeries().c_str());
  BText series = strdup(this->m_nefisMetadata[idx].name().c_str());

  const std::string basename = this->getFileBasename(this->m_filename);
  BText datfile = strdup(std::string(basename + ".def").c_str());
  BText deffile = strdup(std::string(basename + ".dat").c_str());

  BInt4 fid;
  HmdfData::nefCheck(Crenef(&fid, deffile, datfile, 'M', 'r'));

  BInt4 uorder[3] = {1, 2, 3};
  BInt4 uindex[MAX_NEFIS_DIM][3] = {
      {1, static_cast<BInt4>(this->m_nefisTimes.size()), 1},
      {1, static_cast<BInt4>(this->m_stations.size()), 1},
      {static_cast<BInt4>(layer), static_cast<BInt4>(layer), 1},
      {1, 1, 1},
      {1, 1, 1}};

  for (auto &s : this->m_stations) {
    s.allocate(this->m_nefisTimes.size());
  }

  if (this->m_nefisMetadata[idx].type() == "REAL") {
    std::vector<BRea4> realBuf(this->m_nefisTimes.size() *
                               this->m_stations.size());
    BInt4 buffsize =
        sizeof(BRea4) * this->m_nefisTimes.size() * this->m_stations.size();
    HmdfData::nefCheck(Getelt(&fid, src, series, (BInt4 *)uindex, uorder,
                              &buffsize, realBuf.data()),
                       fid);
    for (size_t i = 0; i < this->m_nefisTimes.size(); ++i) {
      for (size_t j = 0; j < this->m_stations.size(); ++j) {
        this->m_stations[j] << Timepoint(
            this->m_nefisTimes[i], realBuf[i * this->m_stations.size() + j]);
      }
    }
  } else if (this->m_nefisMetadata[idx].type() == "INTEGER") {
  } else {
    HmdfData::nefCheck(Clsnef(&fid));
    return 1;
  }

  HmdfData::nefCheck(Clsnef(&fid));
  return 0;
}

size_t HmdfData::getNefisStations() {
  const std::string basename = this->getFileBasename(this->m_filename);
  BText datfile = strdup(std::string(basename + ".def").c_str());
  BText deffile = strdup(std::string(basename + ".dat").c_str());

  BInt4 fid;
  HmdfData::nefCheck(Crenef(&fid, deffile, datfile, 'M', 'r'));

  BText hisconst = strdup("his-const");
  BText xystat = strdup("XYSTAT");
  BText namst = strdup("NAMST");
  BInt4 uorder[2] = {1, 2};
  BInt4 uindex[MAX_NEFIS_DIM][3] = {
      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

  std::vector<BChar> elmqty(MAX_NEFIS_NAME + 1);
  std::vector<BChar> elmunt(MAX_NEFIS_NAME + 1);
  std::vector<BChar> elmdes(MAX_NEFIS_DESC + 1);
  std::vector<BInt4> elmdms(MAX_NEFIS_DIM);
  BInt4 elmndim = MAX_NEFIS_DIM;
  BInt4 elmnbyte;
  const int nefisNameLen = 20;

  HmdfData::nefCheck(
      Inqelm(&fid, xystat, elmqty.data(), &elmnbyte, elmqty.data(),
             elmunt.data(), elmdes.data(), &elmndim, elmdms.data()),
      fid);
  size_t nsta = elmdms[1];

  if (nsta < 1) {
    HmdfData::nefCheck(Clsnef(&fid));
    return 0;
  }

  std::vector<BRea4> rbuf(nsta * 2);
  int rBufSize = nsta * 2 * sizeof(BRea4);

  HmdfData::nefCheck(Getelt(&fid, hisconst, xystat, (BInt4 *)uindex, uorder,
                            &rBufSize, (BData)rbuf.data()),
                     fid);

  std::vector<BChar> charBuffer(nsta * (nefisNameLen + 1));
  int charBufSize = nsta * (nefisNameLen + 1);
  BInt4 charOrder[1] = {1};
  HmdfData::nefCheck(Getelt(&fid, hisconst, namst, (BInt4 *)uindex, charOrder,
                            &charBufSize, (BData)charBuffer.data()));
  HmdfData::nefCheck(Clsnef(&fid));

  this->m_stations.reserve(nsta);
  for (size_t i = 0; i < nsta; ++i) {
    double x = rbuf[i * 2];
    double y = rbuf[i * 2 + 1];
    std::string name(&charBuffer.data()[i * nefisNameLen], nefisNameLen);
    Station s(i, x, y, 1);
    s.setName(name);
    this->m_stations.push_back(s);
  }

  return nsta;
}

size_t HmdfData::getNefisLayers(std::string &layerModel) {
  const std::string basename = this->getFileBasename(this->m_filename);
  BText datfile = strdup(std::string(basename + ".def").c_str());
  BText deffile = strdup(std::string(basename + ".dat").c_str());

  BInt4 fid;
  HmdfData::nefCheck(Crenef(&fid, deffile, datfile, 'M', 'r'));

  BInt4 uindex[MAX_NEFIS_DIM][3] = {
      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
  BInt4 uorder[2] = {1, 2};
  BInt4 intDataBuffer[1] = {0};
  BInt4 intBuffSize = sizeof(BInt4);
  BInt4 charBuffSize = 16 * sizeof(BChar);
  BChar charDataBuffer[16];
  BText hisconst = strdup("his-const");
  BText kmax = strdup("KMAX");
  BText laymodel = strdup("LAYER_MODEL");

  HmdfData::nefCheck(Getelt(&fid, hisconst, kmax, (BInt4 *)uindex,
                            (BInt4 *)uorder, &intBuffSize, &intDataBuffer),
                     fid);
  HmdfData::nefCheck(Getelt(&fid, hisconst, laymodel, (BInt4 *)uindex,
                            (BInt4 *)uorder, &charBuffSize, &charDataBuffer),
                     fid);
  HmdfData::nefCheck(Clsnef(&fid));

  layerModel = std::string(charDataBuffer, charDataBuffer + charBuffSize - 1);

  return static_cast<size_t>(intDataBuffer[0]);
}

size_t HmdfData::getNefisTimes(std::vector<Date> &time) {
  const std::string basename = this->getFileBasename(this->m_filename);
  BText datfile = strdup(std::string(basename + ".def").c_str());
  BText deffile = strdup(std::string(basename + ".dat").c_str());

  BText hisconst = strdup("his-const");
  BText hisinfoseries = strdup("his-info-series");
  BText itdate = strdup("ITDATE");
  BText ithisc = strdup("ITHISC");
  BText dtc = strdup("DT");
  BText tunitc = strdup("TUNIT");
  BInt4 uorder[2] = {1, 2};
  BInt4 uindex[MAX_NEFIS_DIM][3] = {
      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

  BInt4 fid;
  HmdfData::nefCheck(Crenef(&fid, deffile, datfile, 'M', 'r'));

  BInt4 ibuflen = 2 * sizeof(BInt4);
  BInt4 ibuf[2] = {0, 0};
  BInt4 rbuflen = 2 * sizeof(BRea4);
  BRea4 rbuf[2] = {0.0, 0.0};
  HmdfData::nefCheck(
      Getelt(&fid, hisconst, itdate, (BInt4 *)uindex, uorder, &ibuflen, &ibuf),
      fid);

  int year = ibuf[0] / 10000;
  int month = (ibuf[0] - (year * 10000)) / 100;
  int day = ibuf[0] - (month * 100) - (year * 10000);
  Date initDate(year, month, day, 0, 0, 0);

  HmdfData::nefCheck(
      Getelt(&fid, hisconst, dtc, (BInt4 *)uindex, uorder, &rbuflen, &rbuf));
  double dt = rbuf[0];

  HmdfData::nefCheck(
      Getelt(&fid, hisconst, tunitc, (BInt4 *)uindex, uorder, &rbuflen, &rbuf));
  double tunit = rbuf[0];

  BInt4 nstep;
  HmdfData::nefCheck(Inqmxi(&fid, hisinfoseries, &nstep), fid);

  time.reserve(nstep);
  std::vector<BInt4> timeBuf(nstep);
  BInt4 timebuflen = sizeof(BInt4) * nstep;
  uindex[0][1] = nstep;
  HmdfData::nefCheck(Getelt(&fid, hisinfoseries, ithisc, (BInt4 *)uindex,
                            uorder, &timebuflen, timeBuf.data()),
                     fid);
  HmdfData::nefCheck(Clsnef(&fid));
  for (int i = 0; i < nstep; ++i) {
    time.push_back(initDate + (timeBuf[i] * dt * tunit));
  }
  return static_cast<size_t>(nstep);
}

Date HmdfData::string2date(const std::string &str) {
  int year = stoi(str.substr(0, 4));
  int month = stoi(str.substr(5, 2));
  int day = stoi(str.substr(8, 2));
  int hour = stoi(str.substr(10, 2));
  int minute = stoi(str.substr(12, 2));
  int second = stoi(str.substr(14, 2));
  return Date(year, month, day, hour, minute, second);
}

Station HmdfData::readGenericNetcdfStation(const int ncid, const int idx,
                                           const std::vector<double> &xcoor,
                                           const std::vector<double> &ycoor,
                                           const std::string &stationNames,
                                           const size_t namelen,
                                           const int epsg) {
  std::string n = stationNames.substr(idx * namelen, namelen);
  n.erase(n.find_last_not_of(" \n\r\t") + 1);
  Station s(idx, xcoor[idx], ycoor[idx], 1);
  s.setName(n);
  s.setEpsg(epsg);

  std::string timeVarName =
      boost::str(boost::format("time_station_%04d") % (idx + 1));
  std::string dataVarName =
      boost::str(boost::format("data_station_%04d") % (idx + 1));
  std::string dimName =
      boost::str(boost::format("stationLength_%04d") % (idx + 1));

  int dimid_stationLength;
  int varid_time;
  int varid_data;
  size_t len;
  HmdfData::ncCheck(nc_inq_dimid(ncid, dimName.c_str(), &dimid_stationLength),
                    ncid);
  HmdfData::ncCheck(nc_inq_dimlen(ncid, dimid_stationLength, &len), ncid);
  HmdfData::ncCheck(nc_inq_varid(ncid, timeVarName.c_str(), &varid_time), ncid);
  HmdfData::ncCheck(nc_inq_varid(ncid, dataVarName.c_str(), &varid_data), ncid);

  std::string refDateStr =
      HmdfData::checkTextAttLenAndReturn(ncid, varid_time, "referenceDate");
  Date refDate = refDateStr == "none" ? Date(1970, 1, 1, 0, 0, 0)
                                      : HmdfData::string2date(refDateStr);

  s.setDatum(HmdfData::checkTextAttLenAndReturn(ncid, varid_data, "datum"));
  s.setUnits(HmdfData::checkTextAttLenAndReturn(ncid, varid_data, "units"));
  s.setTimezone(
      HmdfData::checkTextAttLenAndReturn(ncid, varid_time, "timezone"));

  std::vector<long long> date(len);
  std::vector<double> data(len);
  HmdfData::ncCheck(nc_get_var_longlong(ncid, varid_time, date.data()), ncid);
  HmdfData::ncCheck(nc_get_var_double(ncid, varid_data, data.data()), ncid);

  s.allocate(len);
  for (size_t i = 0; i < len; ++i) {
    s << Timepoint(refDate + date[i], data[i]);
  }
  return s;
}

int HmdfData::readGenericNetcdf() {
  int ncid;
  HmdfData::ncCheck(nc_open(this->m_filename.c_str(), NC_NOWRITE, &ncid));

  int dimid_nstations;
  int dimid_stationNameLen;
  HmdfData::ncCheck(nc_inq_dimid(ncid, "numStations", &dimid_nstations), ncid);
  HmdfData::ncCheck(nc_inq_dimid(ncid, "stationNameLen", &dimid_stationNameLen),
                    ncid);

  size_t nsta;
  size_t staNameLen;
  HmdfData::ncCheck(nc_inq_dimlen(ncid, dimid_nstations, &nsta), ncid);
  HmdfData::ncCheck(nc_inq_dimlen(ncid, dimid_stationNameLen, &staNameLen),
                    ncid);

  int varid_xcoor;
  int varid_ycoor;
  int varid_stationName;
  int epsg;
  HmdfData::ncCheck(nc_inq_varid(ncid, "stationXCoordinate", &varid_xcoor),
                    ncid);
  HmdfData::ncCheck(nc_inq_varid(ncid, "stationYCoordinate", &varid_ycoor),
                    ncid);
  HmdfData::ncCheck(nc_inq_varid(ncid, "stationName", &varid_stationName),
                    ncid);
  HmdfData::ncCheck(
      nc_get_att_int(ncid, varid_xcoor, "HorizontalProjectionEPSG", &epsg),
      ncid);
  this->setEpsg(epsg);

  std::vector<double> stationXcoor(nsta);
  std::vector<double> stationYcoor(nsta);
  std::string stationName(nsta * (staNameLen + 1), ' ');

  HmdfData::ncCheck(nc_get_var_double(ncid, varid_xcoor, stationXcoor.data()),
                    ncid);
  HmdfData::ncCheck(nc_get_var_double(ncid, varid_ycoor, stationYcoor.data()),
                    ncid);
  HmdfData::ncCheck(nc_get_var_text(ncid, varid_stationName, &stationName[0]),
                    ncid);

  this->m_stations.reserve(nsta);
  for (size_t i = 0; i < nsta; ++i) {
    this->m_stations.push_back(HmdfData::readGenericNetcdfStation(
        ncid, i, stationXcoor, stationYcoor, stationName, staNameLen, epsg));
  }

  return 0;
}

std::string HmdfData::checkTextAttLenAndReturn(const int ncid, const int varid,
                                               const std::string &att) {
  size_t len;
  HmdfData::ncCheck(nc_inq_attlen(ncid, varid, att.c_str(), &len));
  if (len > 0) {
    std::string attValue(' ', len);
    HmdfData::ncCheck(nc_get_att_text(ncid, varid, att.c_str(), &attValue[0]),
                      ncid);
    attValue.erase(attValue.find_last_not_of(" \n\r\t") + 1);
    return attValue;
  } else {
    return "none";
  }
}

void HmdfData::bounds(Date &begin, Date &end, double &min, double &max) {
  end = Date::minDate();
  begin = Date::maxDate();
  min = std::numeric_limits<double>::max();
  max = -std::numeric_limits<double>::max();
  for (auto &s : this->m_stations) {
    begin = std::min(begin, s[0]->date());
    end = std::max(end, s[s.size() - 1]->date());
    double tempMin;
    double tempMax;
    s.minmax(tempMin, tempMax);
    min = std::min(min, tempMin);
    max = std::max(max, tempMax);
  }
  return;
}

int HmdfData::readDflowFmHeader() { return 1; }

int HmdfData::readDflowFmValue(const std::string &var, size_t layer) {
  return 1;
}
