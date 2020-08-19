#ifndef HMDF_TIMEPOINT_H
#define HMDF_TIMEPOINT_H
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
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "date.h"
#include "hmdf_global.h"

namespace Hmdf {

class Timepoint {
 public:
  /// Default constructor
  HMDF_EXPORT Timepoint();

  /// Baseline constructors (1d, 2d, 3d)
  HMDF_EXPORT Timepoint(const Hmdf::Date &d, const double v);
  HMDF_EXPORT Timepoint(const Hmdf::Date &d, const double v1, const double v2);
  HMDF_EXPORT Timepoint(const Hmdf::Date &d, const double v1, const double v2,
                        const double v3);

  /// Generic vector constructor
  HMDF_EXPORT Timepoint(const Hmdf::Date &d, std::vector<double> v);

  /// Templated constructor (c++ only) using variadic template
  template <typename... Double>
  HMDF_EXPORT Timepoint(const Hmdf::Date &d, const double v1, const double v2,
                        const double v3, const Double &... args)
      : m_date(d), m_v{v1, v2, v3, args...} {}

  static constexpr double nullValue() {
    return std::numeric_limits<double>::max();
  }

  static Timepoint null() { return Timepoint(Hmdf::Date(), nullValue()); }

  Hmdf::Date HMDF_EXPORT date() const;
  void HMDF_EXPORT setDate(const Hmdf::Date &date);

  double HMDF_EXPORT value(size_t index);
  double HMDF_EXPORT value();

  double HMDF_EXPORT magnitude();
  double HMDF_EXPORT direction();

  void HMDF_EXPORT set(const Hmdf::Date &d, const double v);
  void HMDF_EXPORT set(const Hmdf::Date &d, const double v1, const double v2);
  void HMDF_EXPORT set(const Hmdf::Date &d, const double v1, const double v2,
                       const double v3);
  void HMDF_EXPORT set(const Hmdf::Date &d, const std::vector<double> &v);

  void HMDF_EXPORT setValue(const double v);
  void HMDF_EXPORT setValue(const size_t index, const double v);
  void HMDF_EXPORT setValue(const double v1, const double v2);
  void HMDF_EXPORT setValue(const double v1, const double v2, const double v3);
  void HMDF_EXPORT setValue(const std::vector<double> &v);

  bool HMDF_EXPORT operator<(const Hmdf::Timepoint &p) const;
  bool HMDF_EXPORT operator>(const Hmdf::Timepoint &p) const;
  bool HMDF_EXPORT operator==(const Hmdf::Timepoint &p) const;
  bool HMDF_EXPORT operator!=(const Hmdf::Timepoint &p) const;

  double HMDF_EXPORT operator()(const size_t index) const;
  double HMDF_EXPORT operator[](const size_t index) const;

  static bool HMDF_EXPORT dateEqual(const Hmdf::Timepoint &p1,
                                    const Hmdf::Timepoint &p2);

  size_t HMDF_EXPORT dimension() const;
  void HMDF_EXPORT redimension(size_t n);

  void HMDF_EXPORT shift(const long time, const double value);
  void HMDF_EXPORT shift(const long time, const std::vector<double> &value);

#ifndef SWIG
  typedef typename std::vector<double>::iterator iterator;
  typedef typename std::vector<double>::const_iterator const_iterator;

  iterator HMDF_EXPORT begin() noexcept;
  const_iterator HMDF_EXPORT cbegin() const noexcept;
  iterator HMDF_EXPORT end() noexcept;
  const_iterator HMDF_EXPORT cend() const noexcept;
#endif

  friend std::ostream &operator<<(std::ostream &os, const Hmdf::Timepoint &t);

 private:
  Hmdf::Date m_date;
  std::vector<double> m_v;
};

}  // namespace Hmdf

#endif  // TIMEPOINT_H
