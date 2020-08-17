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
#include "hmdf_global.h"
#include "nefisseriesmetadata.h"
#include "station.h"
#include "types.h"

class Hmdf {
 public:
  HMDF_EXPORT Hmdf(HmdfString filename = HmdfString(),
                   const Date &coldstart = Date(),
                   HmdfString stationFile = HmdfString());

  int HMDF_EXPORT read();
  static int HMDF_EXPORT write(const HmdfString &filename);

  int HMDF_EXPORT readNefisValue(const HmdfString &var, size_t layer = 0);

  size_t HMDF_EXPORT nStations() const;

  HmdfVector<HmdfString> HMDF_EXPORT headerData() const;
  void HMDF_EXPORT setHeaderData(const HmdfVector<HmdfString> &headerData);

  bool HMDF_EXPORT success() const;

  bool HMDF_EXPORT null() const;

  int HMDF_EXPORT dimension() const;

  Station HMDF_EXPORT *station(size_t index);

  int HMDF_EXPORT setStation(size_t index, Station &s);

  int HMDF_EXPORT addStation(Station &s);

  int HMDF_EXPORT reproject(int epsg);

  void HMDF_EXPORT setEpsg(int epsg);

  int HMDF_EXPORT resize(size_t n);

  void HMDF_EXPORT operator<<(const Station &s);

  void HMDF_EXPORT operator<<(const HmdfVector<Station> &s);

  void HMDF_EXPORT sanitize();

#ifndef SWIG
  typedef typename HmdfVector<Station>::iterator iterator;
  typedef typename HmdfVector<Station>::const_iterator const_iterator;

  iterator HMDF_EXPORT begin() noexcept;
  const_iterator HMDF_EXPORT cbegin() const noexcept;
  iterator HMDF_EXPORT end() noexcept;
  const_iterator HMDF_EXPORT cend() const noexcept;
#endif

  HmdfString HMDF_EXPORT getFilename() const;
  void HMDF_EXPORT setFilename(const HmdfString &filename);

  Date HMDF_EXPORT getColdstart() const;
  void HMDF_EXPORT setColdstart(const Date &coldstart);

  HmdfString HMDF_EXPORT getStationFile() const;
  void HMDF_EXPORT setStationFile(const HmdfString &stationFile);

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

  static FileType getFiletype(const HmdfString &filename);
  static HmdfString getFileExtension(const HmdfString &filename);
  HmdfString getFileBasename(const HmdfString &filename);
  static void splitString(HmdfString &data, HmdfVector<HmdfString> &fresult);
  static HmdfString sanitizeString(const HmdfString &a);
  static bool splitStringHmdfFormat(const HmdfString &data, int &year,
                                    int &month, int &day, int &hour,
                                    int &minute, int &second, double &value);
  static FileType checkNetcdfType(const HmdfString &filename);
  static HmdfString checkTextAttLenAndReturn(const int ncid, const int varid,
                                             const std::string &att);
  static Station readGenericNetcdfStation(const int ncid, const int idx,
                                          const std::vector<double> &xcoor,
                                          const std::vector<double> &ycoor,
                                          const std::string &stationNames,
                                          const size_t namelen, const int epsg);
  static Date string2date(const std::string &str);

  int readAdcircAscii();
  int readAdcircNetCDF();
  int readNefisHeader();
  int readDflowFmHeader();
  int readDflowFmValue(const HmdfString &var, size_t layer = 0);
  int readImeds();
  int readGenericNetcdf();
  static size_t readAdcircStationFile(const HmdfString &filename,
                                      HmdfVector<double> &x,
                                      HmdfVector<double> &y);
  static void ncCheck(const int retcode, const int fid = -1);
  static void nefCheck(const int retcode, int fid = -1);
  static int getAdcircVariableId(const int ncid, int &varid1, int &varid2);

  void getNefisDatasets(const char *series,
                        HmdfVector<NefisSeriesMetadata> &metadata,
                        const size_t numStations);
  size_t getNefisLayers(HmdfString &layerModel);
  size_t getNefisTimes(HmdfVector<Date> &time);
  size_t getNefisStations();
  size_t getNefisVarIndex(const HmdfString &var);

  HmdfString m_filename;
  Date m_coldstart;
  HmdfString m_stationFile;
  HmdfVector<HmdfString> m_headerData;
  HmdfVector<NefisSeriesMetadata> m_nefisMetadata;
  HmdfVector<Date> m_nefisTimes;
  size_t m_nefisLayers;
  bool m_success, m_null;
  int m_dimension;
  int m_epsg;

  HmdfVector<Station> m_stations;
};

#endif  // HMDF_H
