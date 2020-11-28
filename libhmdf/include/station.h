#ifndef HMDF_STATION_H
#define HMDF_STATION_H
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
#include "timepoint.h"

#ifdef SWIG
#define NODISCARD
#else
#define NODISCARD [[nodiscard]]
#endif

namespace Hmdf {

class Station {
 public:
  HMDF_EXPORT explicit Station(unsigned char dimension = 1);
  HMDF_EXPORT Station(size_t id, double x, double y,
                      unsigned char dimension = 1, unsigned int epsg = 4326);

  NODISCARD double HMDF_EXPORT x() const;
  NODISCARD double HMDF_EXPORT y() const;
  NODISCARD double HMDF_EXPORT x_original() const;
  NODISCARD double HMDF_EXPORT y_original() const;
  NODISCARD double HMDF_EXPORT latitude() const;
  NODISCARD double HMDF_EXPORT longitude() const;

  void HMDF_EXPORT setX(double x);
  void HMDF_EXPORT setY(double y);
  void HMDF_EXPORT setLatitude(double latitude);
  void HMDF_EXPORT setLongitude(double longitude);
  void HMDF_EXPORT setLocation(double x, double y);

  Hmdf::Timepoint HMDF_EXPORT operator()(size_t index) const;
  Hmdf::Timepoint HMDF_EXPORT *operator[](size_t index);
  NODISCARD Hmdf::Timepoint HMDF_EXPORT cat(size_t index) const;
  Hmdf::Timepoint HMDF_EXPORT *at(size_t index);

  void HMDF_EXPORT push_back(const Hmdf::Timepoint &p);
  void HMDF_EXPORT operator<<(const Hmdf::Timepoint &p);
  void HMDF_EXPORT operator<<(const std::vector<Hmdf::Timepoint> &p);

  void HMDF_EXPORT deleteAt(size_t index);

  void HMDF_EXPORT clear();

  NODISCARD int HMDF_EXPORT epsg() const;
  NODISCARD int HMDF_EXPORT epsg_original() const;
  void HMDF_EXPORT setEpsg(int epsg);

  int HMDF_EXPORT reproject(int epsg);

  void HMDF_EXPORT allocate(size_t n);

  NODISCARD std::string HMDF_EXPORT name() const;
  void HMDF_EXPORT setName(const std::string &name);

  NODISCARD size_t HMDF_EXPORT index() const;
  void HMDF_EXPORT setIndex(const size_t &id);

  NODISCARD std::string HMDF_EXPORT datum() const;
  void HMDF_EXPORT setDatum(const std::string &datum);

  NODISCARD std::string HMDF_EXPORT units() const;
  void HMDF_EXPORT setUnits(const std::string &units);

  NODISCARD std::string HMDF_EXPORT timezone() const;
  void HMDF_EXPORT setTimezone(const std::string &timezone);

  NODISCARD std::string HMDF_EXPORT id() const;
  void HMDF_EXPORT setId(const std::string &id);

  NODISCARD size_t HMDF_EXPORT dimension() const;

  NODISCARD size_t HMDF_EXPORT size() const;

  NODISCARD double HMDF_EXPORT meanDt() const;

  void HMDF_EXPORT show() const;

  void HMDF_EXPORT sanitize();

  void HMDF_EXPORT shift(const long time, const double value);

  NODISCARD size_t HMDF_EXPORT nNotNull(size_t index = 0) const;
  NODISCARD double HMDF_EXPORT sum(size_t index = 0) const;
  NODISCARD double HMDF_EXPORT mean(size_t index = 0) const;
  NODISCARD double HMDF_EXPORT median(size_t index = 0) const;
  NODISCARD double HMDF_EXPORT max(size_t index = 0) const;
  NODISCARD double HMDF_EXPORT min(size_t index = 0) const;
  NODISCARD double HMDF_EXPORT range(size_t index = 0) const;
  void HMDF_EXPORT minmax(double &min, double &max, size_t index = 0) const;

#ifndef SWIG
  typedef typename std::vector<Hmdf::Timepoint>::iterator iterator;
  typedef typename std::vector<Hmdf::Timepoint>::const_iterator const_iterator;

  iterator HMDF_EXPORT begin() noexcept;
  NODISCARD const_iterator HMDF_EXPORT begin() const noexcept;
  NODISCARD const_iterator HMDF_EXPORT cbegin() const noexcept;
  iterator HMDF_EXPORT end() noexcept;
  NODISCARD const_iterator HMDF_EXPORT end() const noexcept;
  NODISCARD const_iterator HMDF_EXPORT cend() const noexcept;

  Hmdf::Timepoint HMDF_EXPORT front() noexcept;
  Hmdf::Timepoint HMDF_EXPORT back() noexcept;
#endif

  friend std::ostream HMDF_EXPORT &operator<<(std::ostream &os,
                                              const Hmdf::Station *s);

 private:
  std::vector<Hmdf::Timepoint> m_data;
  std::string m_name;
  std::string m_datum;
  std::string m_units;
  std::string m_timezone;
  std::string m_id;
  size_t m_index;
  double m_x;
  double m_y;
  unsigned int m_epsg;
  double m_x_original;
  double m_y_original;
  unsigned int m_epsg_original;
  unsigned char m_dimension;
};

}  // namespace Hmdf

#endif  // STATION_H
