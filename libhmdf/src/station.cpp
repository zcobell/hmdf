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
#include "station.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>

#include "boost/format.hpp"
#include "projection.h"

using namespace Hmdf;

Station::Station(const unsigned char dimension)
    : m_name("noname"),
      m_datum("none"),
      m_units("none"),
      m_timezone("none"),
      m_index(0),
      m_x(0.0),
      m_y(0.0),
      m_epsg(4326),
      m_x_original(m_x),
      m_y_original(m_y),
      m_epsg_original(m_epsg),
      m_dimension(dimension) {}

Station::Station(const size_t id, const double x, const double y,
                 const unsigned char dimension, const unsigned int epsg)
    : m_name("noname"),
      m_datum("none"),
      m_units("none"),
      m_timezone("none"),
      m_index(id),
      m_x(x),
      m_y(y),
      m_epsg(epsg),
      m_x_original(m_x),
      m_y_original(m_y),
      m_epsg_original(m_epsg),
      m_dimension(dimension) {}

double Station::x() const { return this->m_x; }
double Station::y() const { return this->m_y; }
double Station::x_original() const { return this->m_x_original; }
double Station::y_original() const { return this->m_y_original; }
double Station::latitude() const { return this->m_y; }
double Station::longitude() const { return this->m_x; }

void Station::setX(double x) { this->m_x = x; }
void Station::setY(double y) { this->m_y = y; }
void Station::setLatitude(double latitude) { this->m_y = latitude; }
void Station::setLongitude(double longitude) { this->m_x = longitude; }
void Station::setLocation(double x, double y) {
  this->m_x = x;
  this->m_y = y;
}

Timepoint Station::operator()(const size_t index) const {
  assert(index < m_data.size());
  return m_data[index];
}

Timepoint *Station::operator[](const size_t index) {
  assert(index < m_data.size());
  return &m_data[index];
}

Timepoint Station::cat(const size_t index) const {
  assert(index < m_data.size());
  return m_data[index];
}

Timepoint *Station::at(const size_t index) {
  assert(index < m_data.size());
  return &m_data[index];
}

void Station::push_back(const Timepoint &p) { this->m_data.push_back(p); }
void Station::operator<<(const Timepoint &p) { this->push_back(p); }

std::string Station::id() const { return m_id; }

void Station::setId(const std::string &id) { m_id = id; }

std::string Station::timezone() const { return m_timezone; }

void Station::setTimezone(const std::string &timezone) {
  m_timezone = timezone;
}

std::string Station::units() const { return m_units; }

void Station::setUnits(const std::string &units) { m_units = units; }
void Station::operator<<(const std::vector<Timepoint> &p) {
  this->m_data.insert(this->m_data.end(), p.begin(), p.end());
}

void Station::deleteAt(const size_t index) {
  this->m_data.erase(this->m_data.begin() + index);
}

void Station::clear() { this->m_data.clear(); }

int Station::epsg() const { return this->m_epsg; }

int Station::epsg_original() const { return this->m_epsg_original; }

void Station::setEpsg(int epsg) {
  this->m_epsg = epsg;
  this->m_epsg_original = epsg;
}

int Station::reproject(int epsg) {
  this->m_epsg = epsg;
  return Projection::transform(this->m_epsg_original, this->m_epsg,
                               this->m_x_original, this->m_y_original,
                               this->m_x, this->m_y);
}

void Station::allocate(size_t n) { this->m_data.reserve(n); }

std::string Station::name() const { return this->m_name; }
void Station::setName(const std::string &name) { this->m_name = name; }

size_t Station::index() const { return this->m_index; }
void Station::setIndex(const size_t &id) { this->m_index = id; }

std::string Station::datum() const { return this->m_datum; }
void Station::setDatum(const std::string &datum) { this->m_datum = datum; }

size_t Station::dimension() const {
  if (!this->m_data.empty()) {
    return this->m_data[0].dimension();
  }
  return 0;
}

size_t Station::size() const { return this->m_data.size(); }

double Station::meanDt() const {
  double dt = 0;
  for (size_t i = 1; i < this->m_data.size(); ++i) {
    dt += this->m_data[i].date().toSeconds() -
          this->m_data[i - 1].date().toSeconds();
  }
  return dt / static_cast<double>(this->m_data.size() - 1);
}

std::ostream &Hmdf::operator<<(std::ostream &os, const Hmdf::Station *s) {
  if (s->name() == "noname") {
    os << "Station: " << s->id() << std::endl;
  } else {
    os << "Station: " << s->name() << std::endl;
  }
  os << "             ID: " << s->id() << std::endl;
  if (s->epsg() != s->epsg_original()) {
    os << "   X Coordinate: " << boost::str(boost::format("%f") % s->x())
       << " (" << boost::str(boost::format("%f") % s->x_original()) << ")"
       << std::endl;
    os << "   Y Coordinate: " << boost::str(boost::format("%f") % s->y())
       << " (" << boost::str(boost::format("%f") % s->y_original()) << ")"
       << std::endl;
    os << "     Projection: " << s->epsg() << " (" << s->epsg_original() << ")"
       << std::endl;
  } else {
    os << "   X Coordinate: " << boost::str(boost::format("%f") % s->x())
       << std::endl;
    os << "   Y Coordinate: " << boost::str(boost::format("%f") % s->y())
       << std::endl;
    os << "     Projection: " << s->epsg() << std::endl;
  }
  os << "          Datum: " << s->datum() << std::endl;
  os << "          Units: " << s->units() << std::endl;
  os << "       Timezone: " << s->timezone() << std::endl;
  os << "      Dimension: " << s->dimension() << std::endl;
  os << "         Length: " << s->size() << std::endl;
  os << "  Mean Timestep: " << s->meanDt() << std::endl;

  std::string mx("empty");
  std::string mn("empty");
  std::string me("empty");
  std::string nl("empty");
  std::string dateStart("empty");
  std::string dateEnd("empty");

  if (s->m_data.size() > 0) {
    dateStart = s->m_data.front().date().toString();
    dateEnd = s->m_data.back().date().toString();
    for (size_t i = 0; i < s->dimension(); ++i) {
      if (i > 0) {
        mx = boost::str(boost::format("%s, %f") % mx % s->max(i));
        mn = boost::str(boost::format("%s, %f") % mn % s->min(i));
        me = boost::str(boost::format("%s, %f") % me % s->mean(i));
        nl = boost::str(boost::format("%s, %d") % nl %
                        (s->size() - s->nNotNull(i)));
      } else {
        mx = boost::str(boost::format("%f") % s->max(0));
        mn = boost::str(boost::format("%f") % s->min(0));
        me = boost::str(boost::format("%f") % s->mean(0));
        nl = boost::str(boost::format("%d") % (s->size() - s->nNotNull(0)));
      }
    }
  }
  os << "            Min: " << mn << std::endl;
  os << "            Max: " << mx << std::endl;
  os << "           Mean: " << me << std::endl;
  os << "          nNull: " << nl << std::endl;
  os << "          Begin: " << dateStart << std::endl;
  os << "            End: " << dateEnd << std::endl;
  return os;
}

void Station::show() const { std::cout << this; }

void Station::sanitize() {
  std::stable_sort(this->m_data.begin(), this->m_data.end());
  this->m_data.erase(std::unique(this->m_data.begin(), this->m_data.end(),
                                 Timepoint::dateEqual),
                     this->m_data.end());
}

double Station::sum(const size_t index) const {
  double sum = 0.0;
  for (auto &s : this->m_data) {
    if (s[index] != Timepoint::nullValue()) {
      sum += s[index];
    }
  }
  return sum;
}

size_t Station::nNotNull(const size_t index) const {
  size_t n = 0;
  for (auto &s : this->m_data) {
    if (s[index] != Timepoint::nullValue()) {
      n += 1;
    }
  }
  return n;
}

double Station::mean(const size_t index) const {
  return this->sum(index) / static_cast<double>(this->nNotNull());
}

double Station::median(const size_t index) const {
  std::vector<double> v;
  v.reserve(this->nNotNull(index));
  for (auto &s : this->m_data) {
    if (s[index] != Timepoint::nullValue()) {
      v.push_back(s[index]);
    }
  }
  std::sort(v.begin(), v.end());
  return v.size() % 2 == 0 ? (v[v.size() / 2] + v[v.size() / 2 + 1]) / 2
                           : v[v.size() / 2];
}

double Station::min(const size_t index) const {
  double min = std::numeric_limits<double>::max();
  for (auto &s : this->m_data) {
    if (s[index] != Timepoint::nullValue()) {
      min = std::min(min, s[index]);
    }
  }
  return min;
}

double Station::max(const size_t index) const {
  double max = -std::numeric_limits<double>::max();
  for (auto &s : this->m_data) {
    if (s[index] != Timepoint::nullValue()) {
      max = std::max(max, s[index]);
    }
  }
  return max;
}

double Station::range(size_t index) const {
  return this->max(index) - this->min(index);
}

void Station::minmax(double &min, double &max, const size_t index) const {
  min = std::numeric_limits<double>::max();
  max = -std::numeric_limits<double>::max();
  for (auto &s : this->m_data) {
    if (s[index] != Timepoint::nullValue()) {
      min = std::min(min, s[index]);
      max = std::max(max, s[index]);
    }
  }
  return;
}

Station::iterator Station::begin() noexcept { return this->m_data.begin(); }

Station::const_iterator Station::begin() const noexcept {
  return this->m_data.begin();
}

Station::const_iterator Station::cbegin() const noexcept {
  return this->m_data.cbegin();
}

Station::iterator Station::end() noexcept { return this->m_data.end(); }

Station::const_iterator Station::end() const noexcept {
  return this->m_data.end();
}

Station::const_iterator Station::cend() const noexcept {
  return this->m_data.cend();
}

Timepoint Station::front() noexcept { return this->m_data.front(); }

Timepoint Station::back() noexcept { return this->m_data.back(); }

void Station::shift(const long time, const double value) {
  for (auto &t : this->m_data) {
    t.shift(time, value);
  }
}
