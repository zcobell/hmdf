#ifndef STATION_H
#define STATION_H
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
#include <vector>

#include "hmdf_global.h"
#include "timepoint.h"

class Station {
 public:
  HMDF_EXPORT Station(size_t dimension = 1);
  HMDF_EXPORT Station(size_t id, double x, double y, size_t dimension = 1,
                      int epsg = 4326);

  double HMDF_EXPORT x() const;
  double HMDF_EXPORT y() const;
  double HMDF_EXPORT x_original() const;
  double HMDF_EXPORT y_original() const;
  double HMDF_EXPORT latitude() const;
  double HMDF_EXPORT longitude() const;

  void HMDF_EXPORT setX(double x);
  void HMDF_EXPORT setY(double y);
  void HMDF_EXPORT setLatitude(double latitude);
  void HMDF_EXPORT setLongitude(double longitude);
  void HMDF_EXPORT setLocation(double x, double y);

  Timepoint HMDF_EXPORT operator()(const size_t index) const;
  Timepoint HMDF_EXPORT *operator[](const size_t index);

  void HMDF_EXPORT push_back(const Timepoint &p);
  void HMDF_EXPORT operator<<(const Timepoint &p);
  void HMDF_EXPORT operator<<(const std::vector<Timepoint> &p);

  void HMDF_EXPORT clear();

  int HMDF_EXPORT epsg() const;
  int HMDF_EXPORT epsg_original() const;
  void HMDF_EXPORT setEpsg(int epsg);

  int HMDF_EXPORT reproject(int epsg);

  void HMDF_EXPORT allocate(size_t n);

  std::string HMDF_EXPORT name() const;
  void HMDF_EXPORT setName(const std::string &name);

  size_t HMDF_EXPORT id() const;
  void HMDF_EXPORT setId(const size_t &id);

  std::string HMDF_EXPORT datum() const;
  void HMDF_EXPORT setDatum(const std::string &datum);

  size_t HMDF_EXPORT dimension() const;

  size_t HMDF_EXPORT size() const;

  double HMDF_EXPORT meanDt() const;

  void HMDF_EXPORT show() const;

  void HMDF_EXPORT sanitize();

  size_t HMDF_EXPORT nNotNull(size_t index = 0) const;
  double HMDF_EXPORT sum(size_t index = 0) const;
  double HMDF_EXPORT mean(size_t index = 0) const;
  double HMDF_EXPORT median(size_t index = 0) const;
  double HMDF_EXPORT max(size_t index = 0) const;
  double HMDF_EXPORT min(size_t index = 0) const;
  double HMDF_EXPORT range(size_t index = 0) const;

#ifndef SWIG
  typedef typename std::vector<Timepoint>::iterator iterator;
  typedef typename std::vector<Timepoint>::const_iterator const_iterator;

  iterator HMDF_EXPORT begin() noexcept;
  const_iterator HMDF_EXPORT cbegin() const noexcept;
  iterator HMDF_EXPORT end() noexcept;
  const_iterator HMDF_EXPORT cend() const noexcept;
#endif

  friend std::ostream HMDF_EXPORT &operator<<(std::ostream &os,
                                              const Station *s);

 private:
  std::vector<Timepoint> m_data;
  std::string m_name;
  std::string m_datum;
  size_t m_id;
  double m_x;
  double m_y;
  double m_epsg;
  double m_x_original;
  double m_y_original;
  int m_epsg_original;
  size_t m_dimension;
};

#endif  // STATION_H
