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
#include "timepoint.h"

#include <cassert>
#include <cmath>
#include <utility>

Timepoint::Timepoint() : m_date(0), m_v{0} {}

Timepoint::Timepoint(const Date &d, const double v) : m_date(d), m_v{v} {};

Timepoint::Timepoint(const Date &d, const double v1, const double v2)
    : m_date(d), m_v{v1, v2} {};

Timepoint::Timepoint(const Date &d, const double v1, const double v2,
                     const double v3)
    : m_date(d), m_v{v1, v2, v3} {};

Timepoint::Timepoint(const Date &d, std::vector<double> v)
    : m_date(d), m_v(std::move(v)) {}

Date Timepoint::date() const { return this->m_date; }

void Timepoint::setDate(const Date &date) { this->m_date = date; }

double Timepoint::value(size_t index) {
  assert(index < m_v.size());
  return this->m_v[index];
}

double Timepoint::value() { return this->m_v[0]; }

void Timepoint::set(const Date &d, const double v) {
  this->m_date = d;
  this->m_v[0] = v;
}

void Timepoint::set(const Date &d, const double v1, const double v2) {
  assert(this->m_v.size() == 2);
  this->m_date = d;
  this->m_v[0] = v1;
  this->m_v[1] = v2;
}

void Timepoint::set(const Date &d, const double v1, const double v2,
                    const double v3) {
  assert(this->m_v.size() == 3);
  this->m_date = d;
  this->m_v[0] = v1;
  this->m_v[1] = v2;
  this->m_v[2] = v3;
}

void Timepoint::set(const Date &d, const std::vector<double> &v) {
  assert(this->m_v.size() == v.size());
  this->m_date = d;
  this->m_v = v;
}

void Timepoint::setValue(const double v) { this->m_v[0] = v; }

void Timepoint::setValue(const size_t index, const double v) {
  assert(index < this->m_v.size());
  this->m_v[index] = v;
}

void Timepoint::setValue(const double v1, const double v2) {
  assert(this->m_v.size() == 2);
  this->m_v[0] = v1;
  this->m_v[1] = v2;
}
void Timepoint::setValue(const double v1, const double v2, const double v3) {
  assert(this->m_v.size() == 3);
  this->m_v[0] = v1;
  this->m_v[1] = v2;
  this->m_v[2] = v3;
}
void Timepoint::setValue(const std::vector<double> &v) {
  assert(v.size() == this->m_v.size());
  this->m_v = v;
}

bool Timepoint::operator<(const Timepoint &p) const {
  return this->date() < p.date();
}

bool Timepoint::operator>(const Timepoint &p) const {
  return this->date() > p.date();
}

bool Timepoint::operator==(const Timepoint &p) const {
  return this->date() == p.date() && this->m_v == p.m_v;
}

bool Timepoint::operator!=(const Timepoint &p) const { return !(*(this) == p); }

double Timepoint::operator()(const size_t index) const {
  assert(index < this->m_v.size());
  return this->m_v[index];
}

double Timepoint::operator[](const size_t index) const {
  return this->operator()(index);
}

bool Timepoint::dateEqual(const Timepoint &p1, const Timepoint &p2) {
  return p1.date() == p2.date();
}

size_t Timepoint::dimension() const { return this->m_v.size(); }

void Timepoint::redimension(size_t n) {
  assert(n > 0);
  this->m_v.resize(n);
}

Timepoint::iterator Timepoint::begin() noexcept { return this->m_v.begin(); }

Timepoint::const_iterator Timepoint::cbegin() const noexcept {
  return this->m_v.cbegin();
}

Timepoint::iterator Timepoint::end() noexcept { return this->m_v.end(); }

Timepoint::const_iterator Timepoint::cend() const noexcept {
  return this->m_v.cend();
}

std::ostream &operator<<(std::ostream &os, const Timepoint &t) {
  os << t.date().toString() << " ";
  for (double i : t.m_v) {
    os << i << " ";
  }
  return os;
}

double Timepoint::magnitude() {
  assert(this->m_v.size() > 1);
  double s = 0.0;
  for (auto &e : m_v) {
    s += std::pow(e, 2.0);
  }
  return std::pow(s, 1.0 / static_cast<double>(m_v.size()));
}

double Timepoint::direction() {
  assert(this->m_v.size() == 2);
  double a = std::atan2(m_v[1], m_v[0]);
  a = a * 180.0 / M_PI;
  if (a >= 360.0) {
    return a - 360.0;
  }
  if (a < 0.0) {
    return a + 360.0;
  } else {
    return a;
  }
}
