#ifndef HMDF_HMDFDATA_H
#define HMDF_HMDFDATA_H
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
#include "nefisseriesmetadata.h"
#include "station.h"

#ifdef SWIG
#define NODISCARD 
#else
#define NODISCARD [[nodiscard]]
#endif

namespace Hmdf {

class HmdfData {
 public:
  HMDF_EXPORT explicit HmdfData(const std::string &filename = std::string(),
                       const Hmdf::Date &coldstart = Hmdf::Date(),
                       const std::string &stationFile = std::string());

  int HMDF_EXPORT read();
  static int HMDF_EXPORT write(const std::string &filename);

  int HMDF_EXPORT readNefisValue(const std::string &var, size_t layer = 0);

  NODISCARD size_t HMDF_EXPORT nStations() const;

  NODISCARD std::vector<std::string> HMDF_EXPORT headerData() const;
  void HMDF_EXPORT setHeaderData(const std::vector<std::string> &headerData);

  NODISCARD bool HMDF_EXPORT success() const;

  NODISCARD bool HMDF_EXPORT null() const;

  NODISCARD int HMDF_EXPORT dimension() const;

  Hmdf::Station HMDF_EXPORT *station(size_t index);

  int HMDF_EXPORT setStation(size_t index, Hmdf::Station &s);

  int HMDF_EXPORT addStation(const Hmdf::Station &s);
  int HMDF_EXPORT moveStation(Hmdf::Station s);

  void HMDF_EXPORT deleteStation(size_t index);

  int HMDF_EXPORT reproject(int epsg);

  void HMDF_EXPORT setEpsg(int epsg);

  int HMDF_EXPORT resize(size_t n);

  void HMDF_EXPORT operator<<(const Hmdf::Station &s);

  void HMDF_EXPORT operator<<(const std::vector<Hmdf::Station> &s);

  void HMDF_EXPORT sanitize();

#ifndef SWIG
  typedef typename std::vector<Hmdf::Station>::iterator iterator;
  typedef typename std::vector<Hmdf::Station>::const_iterator const_iterator;

  iterator HMDF_EXPORT begin() noexcept;
  NODISCARD const_iterator HMDF_EXPORT cbegin() const noexcept;
  iterator HMDF_EXPORT end() noexcept;
  NODISCARD const_iterator HMDF_EXPORT cend() const noexcept;
#endif

  NODISCARD std::string HMDF_EXPORT getFilename() const;
  void HMDF_EXPORT setFilename(const std::string &filename);

  NODISCARD Hmdf::Date HMDF_EXPORT getColdstart() const;
  void HMDF_EXPORT setColdstart(const Hmdf::Date &coldstart);

  NODISCARD std::string HMDF_EXPORT getStationFile() const;
  void HMDF_EXPORT setStationFile(const std::string &stationFile);

  void HMDF_EXPORT bounds(Hmdf::Date &begin, Hmdf::Date &end, double &min,
                          double &max);

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
  std::string getFileBasename(const std::string &filename);
  static void splitString(std::string &data, std::vector<std::string> &fresult);
  static std::string sanitizeString(const std::string &a);
  static bool splitStringHmdfFormat(const std::string &data, int &year,
                                    int &month, int &day, int &hour,
                                    int &minute, int &second, double &value);
  static FileType checkNetcdfType(const std::string &filename);
  static std::string checkTextAttLenAndReturn(int ncid, int varid,
                                              const std::string &att);
  static Station readGenericNetcdfStation(int ncid, int idx,
                                          const std::vector<double> &xcoor,
                                          const std::vector<double> &ycoor,
                                          const std::string &stationNames,
                                          size_t namelen, int epsg);
  static Date string2date(const std::string &str);

  int readAdcircAscii();
  int readAdcircNetCDF();
  int readNefisHeader();
  int readDflowFmHeader();
  int readDflowFmValue(const std::string &var, size_t layer = 0);
  int readImeds();
  int readGenericNetcdf();
  static size_t readAdcircStationFile(const std::string &filename,
                                      std::vector<double> &x,
                                      std::vector<double> &y);
  static void ncCheck(const int retcode, const int fid = -1);
  static void nefCheck(const int retcode, int fid = -1);
  static int getAdcircVariableId(const int ncid, int &varid1, int &varid2);

  void getNefisDatasets(const char *series,
                        std::vector<Hmdf::NefisSeriesMetadata> &metadata,
                        const size_t numStations);
  size_t getNefisLayers(std::string &layerModel);
  size_t getNefisTimes(std::vector<Hmdf::Date> &time);
  size_t getNefisStations();
  size_t getNefisVarIndex(const std::string &var);

  std::string m_filename;
  Hmdf::Date m_coldstart;
  std::string m_stationFile;
  std::vector<std::string> m_headerData;
  std::vector<Hmdf::NefisSeriesMetadata> m_nefisMetadata;
  std::vector<Hmdf::Date> m_nefisTimes;
  size_t m_nefisLayers;
  bool m_success, m_null;
  int m_dimension;
  int m_epsg;

  std::vector<Hmdf::Station> m_stations;
};
}  // namespace Hmdf

#endif  // HMDF_H
