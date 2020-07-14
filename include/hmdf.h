#ifndef HMDF_H
#define HMDF_H
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
#include <string>
#include <vector>

#include "hmdf_global.h"
#include "station.h"

class Hmdf {
 public:
  HMDF_EXPORT Hmdf(const std::string &filename = std::string(),
                   const CDate &coldstart = CDate(),
                   const std::string &stationFile = std::string());

  int HMDF_EXPORT read();
  int HMDF_EXPORT write(const std::string &filename);

  size_t HMDF_EXPORT nStations() const;

  std::vector<std::string> HMDF_EXPORT headerData() const;
  void HMDF_EXPORT setHeaderData(const std::vector<std::string> &headerData);

  bool HMDF_EXPORT success() const;

  bool HMDF_EXPORT null() const;

  int HMDF_EXPORT dimension() const;

  Station HMDF_EXPORT *station(size_t index);

  int HMDF_EXPORT setStation(size_t index, Station &s);

  int HMDF_EXPORT addStation(Station &s);

  int HMDF_EXPORT reproject(int epsg);

  int HMDF_EXPORT resize(size_t n);

  void HMDF_EXPORT operator<<(const Station &s);

  void HMDF_EXPORT operator<<(const std::vector<Station> &s);

  void HMDF_EXPORT sanitize();

#ifndef SWIG
  typedef typename std::vector<Station>::iterator iterator;
  typedef typename std::vector<Station>::const_iterator const_iterator;

  iterator HMDF_EXPORT begin() noexcept;
  const_iterator HMDF_EXPORT cbegin() const noexcept;
  iterator HMDF_EXPORT end() noexcept;
  const_iterator HMDF_EXPORT cend() const noexcept;
#endif

  std::string getFilename() const;
  void setFilename(const std::string &filename);

  CDate getColdstart() const;
  void setColdstart(const CDate &coldstart);

  std::string getStationFile() const;
  void setStationFile(const std::string &stationFile);

 private:
  enum FileType {
    None,
    AdcircAscii,
    AdcircNetCDF,
    Delft3D,
    DFlowFM,
    IMEDS,
    NETCDF
  };

  static FileType getFiletype(const std::string &filename);
  static std::string getFileExtension(const std::string &filename);
  static void splitString(std::string &data, std::vector<std::string> &fresult);
  static std::string sanitizeString(const std::string &a);
  static bool splitStringHmdfFormat(const std::string &data, int &year,
                                    int &month, int &day, int &hour,
                                    int &minute, int &second, double &value);

  int readAdcircAscii(const std::string &filename,
                      const std::string &stationFile, const CDate &coldstart);
  int readAdcircNetCDF(const std::string &filename, const CDate &coldstart);
  int readDelft3D(const std::string &filename);
  int readDFlowFM(const std::string &filename);
  int readImeds(const std::string &filename);
  int readgenericNetCDF(const std::string &filename);
  size_t readAdcircStationFile(const std::string &filename,
                               std::vector<double> &x, std::vector<double> &y);
  void ncCheck(const int retcode);
  int getAdcircVariableId(const int ncid, int &varid1, int &varid2);

  std::string m_filename;
  CDate m_coldstart;
  std::string m_stationFile;
  std::vector<std::string> m_headerData;
  bool m_success, m_null;
  int m_dimension;
  int m_epsg;

  std::vector<Station> m_stations;
};

#endif  // HMDF_H
