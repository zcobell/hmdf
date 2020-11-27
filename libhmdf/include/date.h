#ifndef HMDF_DATE_H
#define HMDF_DATE_H
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
#include <chrono>
#include <cmath>
#include <iostream>
#include <ratio>
#include <string>
#include <type_traits>
#include <vector>

#include "hmdf_global.h"
namespace Hmdf {

class Date {
 public:
  using milliseconds = std::chrono::milliseconds;
  using seconds = std::chrono::seconds;
  using minutes = std::chrono::minutes;
  using hours = std::chrono::hours;
  using days = std::chrono::duration<
      int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
  using weeks =
      std::chrono::duration<int,
                            std::ratio_multiply<std::ratio<7>, days::period>>;
  using years = std::chrono::duration<
      int, std::ratio_multiply<std::ratio<146097, 400>, days::period>>;
  using months =
      std::chrono::duration<int,
                            std::ratio_divide<years::period, std::ratio<12>>>;

  HMDF_EXPORT Date();
  HMDF_EXPORT Date(const std::chrono::system_clock::time_point &t);
  HMDF_EXPORT Date(const std::vector<int> &v);
  HMDF_EXPORT Date(const Hmdf::Date &d);
  HMDF_EXPORT Date(int year, int month = 1, int day = 1, int hour = 0,
                   int minute = 0, int second = 0, int millisecond = 0);

#ifndef SWIG
  //...operator overloads
  bool HMDF_EXPORT operator<(const Hmdf::Date &d) const;
  bool HMDF_EXPORT operator>(const Hmdf::Date &d) const;
  bool HMDF_EXPORT operator==(const Hmdf::Date &d) const;
  bool HMDF_EXPORT operator!=(const Hmdf::Date &d) const;

  template <class T, typename std::enable_if<std::is_integral<T>::value>::type
                         * = nullptr>
  Date &operator+=(const T &rhs) {
    this->m_datetime += Date::seconds(rhs);
    return *this;
  }

  template <class T, typename std::enable_if<
                         std::is_floating_point<T>::value>::type * = nullptr>
  Date &operator+=(const T &rhs) {
    this->m_datetime +=
        Date::milliseconds(static_cast<long>(std::floor(rhs * 1000.0)));
    return *this;
  }

  template <
      class T,
      typename std::enable_if<
          !std::is_integral<T>::value && !std::is_floating_point<T>::value &&
          !std::is_same<T, Hmdf::Date::years>::value &&
          !std::is_same<T, Hmdf::Date::months>::value>::type * = nullptr>
  Date &operator+=(const T &rhs) {
    this->m_datetime += rhs;
    return *this;
  }

  Date HMDF_EXPORT &operator+=(const Hmdf::Date::years &rhs);
  Date HMDF_EXPORT &operator+=(const Hmdf::Date::months &rhs);

  template <class T, typename std::enable_if<std::is_integral<T>::value>::type
                         * = nullptr>
  Date &operator-=(const T &rhs) {
    this->m_datetime -= Hmdf::Date::seconds(rhs);
    return *this;
  }

  template <class T, typename std::enable_if<
                         std::is_floating_point<T>::value>::type * = nullptr>
  Date &operator-=(const T &rhs) {
    this->m_datetime -=
        Hmdf::Date::milliseconds(static_cast<long>(std::floor(rhs * 1000.0)));
    return *this;
  }

  template <
      class T,
      typename std::enable_if<
          !std::is_integral<T>::value && !std::is_floating_point<T>::value &&
          !std::is_same<T, Hmdf::Date::years>::value &&
          !std::is_same<T, Hmdf::Date::months>::value>::type * = nullptr>
  Date &operator-=(const T &rhs) {
    this->m_datetime -= rhs;
    return *this;
  }

  Date HMDF_EXPORT &operator-=(const Hmdf::Date::years &rhs);
  Date HMDF_EXPORT &operator-=(const Hmdf::Date::months &rhs);

  friend std::ostream HMDF_EXPORT &operator<<(std::ostream &os,
                                              const Hmdf::Date &dt);

  // end operator overloads
#endif

  void HMDF_EXPORT addSeconds(const long &value);
  void HMDF_EXPORT addMinutes(const long &value);
  void HMDF_EXPORT addHours(const long &value);
  void HMDF_EXPORT addDays(const long &value);
  void HMDF_EXPORT addWeeks(const long &value);
  void HMDF_EXPORT addMonths(const long &value);
  void HMDF_EXPORT addYears(const long &value);

  static Date maxDate() { return Hmdf::Date(3000, 1, 1, 0, 0, 0); }
  static Date minDate() { return Hmdf::Date(1900, 1, 1, 0, 0, 0); }

  std::vector<int> HMDF_EXPORT get() const;

  void HMDF_EXPORT set(const std::vector<int> &v);
  void HMDF_EXPORT set(const std::chrono::system_clock::time_point &t);
  void HMDF_EXPORT set(const Hmdf::Date &v);
  void HMDF_EXPORT set(int year, int month = 1, int day = 1, int hour = 0,
                       int minute = 0, int second = 0, int millisecond = 0);

  void HMDF_EXPORT fromSeconds(long long seconds);

  void HMDF_EXPORT fromMSeconds(long long mseconds);

  long HMDF_EXPORT toSeconds() const;

  long long HMDF_EXPORT toMSeconds() const;

  int HMDF_EXPORT year() const;
  void HMDF_EXPORT setYear(int year);

  int HMDF_EXPORT month() const;
  void HMDF_EXPORT setMonth(int month);

  int HMDF_EXPORT day() const;
  void HMDF_EXPORT setDay(int day);

  int HMDF_EXPORT hour() const;
  void HMDF_EXPORT setHour(int hour);

  int HMDF_EXPORT minute() const;
  void HMDF_EXPORT setMinute(int minute);

  int HMDF_EXPORT second() const;
  void HMDF_EXPORT setSecond(int second);

  int HMDF_EXPORT millisecond() const;
  void HMDF_EXPORT setMillisecond(int milliseconds);

  void HMDF_EXPORT fromString(const std::string &datestr,
                              const std::string &format = "%Y-%m-%d %H:%M:%OS");

  std::string HMDF_EXPORT
  toString(const std::string &format = "%Y-%m-%d %H:%M:%OS") const;

  std::chrono::system_clock::time_point HMDF_EXPORT time_point() const;

  static Hmdf::Date HMDF_EXPORT now();

 private:
  std::chrono::system_clock::time_point m_datetime;
};
}  // namespace Hmdf

#ifndef SWIG

template <typename DateType>
using is_valid_hmdfdate = typename std::enable_if<
    std::is_arithmetic<DateType>::value ||
    std::is_same<Hmdf::Date::milliseconds, DateType>::value ||
    std::is_same<Hmdf::Date::seconds, DateType>::value ||
    std::is_same<Hmdf::Date::minutes, DateType>::value ||
    std::is_same<Hmdf::Date::hours, DateType>::value ||
    std::is_same<Hmdf::Date::days, DateType>::value ||
    std::is_same<Hmdf::Date::weeks, DateType>::value ||
    std::is_same<Hmdf::Date::months, DateType>::value ||
    std::is_same<Hmdf::Date::years, DateType>::value>::type;

template <typename T, typename = is_valid_hmdfdate<T>>
Hmdf::Date operator+(Hmdf::Date lhs, const T &rhs) {
  lhs += rhs;
  return lhs;
}

template <typename T, typename = is_valid_hmdfdate<T>>
Hmdf::Date operator-(Hmdf::Date lhs, const T &rhs) {
  lhs -= rhs;
  return lhs;
}

template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const short &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const int &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const long &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const unsigned short &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const unsigned int &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const unsigned long &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const float &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const double &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::milliseconds &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::seconds &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::minutes &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::hours &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date, const Hmdf::Date::days &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::months &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::weeks &);
template Hmdf::Date HMDF_EXPORT operator+(Hmdf::Date,
                                          const Hmdf::Date::years &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const short &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const int &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const long &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const unsigned short &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const unsigned int &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const unsigned long &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const float &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const double &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::milliseconds &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::seconds &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::minutes &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::hours &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date, const Hmdf::Date::days &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::months &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::weeks &);
template Hmdf::Date HMDF_EXPORT operator-(Hmdf::Date,
                                          const Hmdf::Date::years &);
#endif

#endif  // HMDFDATE_H
