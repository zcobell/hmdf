#ifndef HMDFDATE_H
#define HMDFDATE_H
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
  HMDF_EXPORT Date(const Date &d);
  HMDF_EXPORT Date(int year, int month = 1, int day = 1, int hour = 0,
                   int minute = 0, int second = 0, int millisecond = 0);

#ifndef SWIG
  //...operator overloads
  bool HMDF_EXPORT operator<(const Date &d) const;
  bool HMDF_EXPORT operator>(const Date &d) const;
  bool HMDF_EXPORT operator==(const Date &d) const;
  bool HMDF_EXPORT operator!=(const Date &d) const;

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

  template <class T,
            typename std::enable_if<!std::is_integral<T>::value &&
                                    !std::is_floating_point<T>::value &&
                                    !std::is_same<T, Date::years>::value &&
                                    !std::is_same<T, Date::months>::value>::type
                * = nullptr>
  Date &operator+=(const T &rhs) {
    this->m_datetime += rhs;
    return *this;
  }

  Date HMDF_EXPORT &operator+=(const Date::years &rhs);
  Date HMDF_EXPORT &operator+=(const Date::months &rhs);

  template <class T, typename std::enable_if<std::is_integral<T>::value>::type
                         * = nullptr>
  Date &operator-=(const T &rhs) {
    this->m_datetime -= Date::seconds(rhs);
    return *this;
  }

  template <class T, typename std::enable_if<
                         std::is_floating_point<T>::value>::type * = nullptr>
  Date &operator-=(const T &rhs) {
    this->m_datetime -=
        Date::milliseconds(static_cast<long>(std::floor(rhs * 1000.0)));
    return *this;
  }

  template <class T,
            typename std::enable_if<!std::is_integral<T>::value &&
                                    !std::is_floating_point<T>::value &&
                                    !std::is_same<T, Date::years>::value &&
                                    !std::is_same<T, Date::months>::value>::type
                * = nullptr>
  Date &operator-=(const T &rhs) {
    this->m_datetime -= rhs;
    return *this;
  }

  Date HMDF_EXPORT &operator-=(const Date::years &rhs);
  Date HMDF_EXPORT &operator-=(const Date::months &rhs);

  friend std::ostream HMDF_EXPORT &operator<<(std::ostream &os, const Date &dt);

  // end operator overloads
#endif

  void HMDF_EXPORT addSeconds(const long &value);
  void HMDF_EXPORT addMinutes(const long &value);
  void HMDF_EXPORT addHours(const long &value);
  void HMDF_EXPORT addDays(const long &value);
  void HMDF_EXPORT addWeeks(const long &value);
  void HMDF_EXPORT addMonths(const long &value);
  void HMDF_EXPORT addYears(const long &value);

  static Date maxDate() { return Date(3000, 1, 1, 0, 0, 0); }
  static Date minDate() { return Date(1900, 1, 1, 0, 0, 0); }

  std::vector<int> HMDF_EXPORT get() const;

  void HMDF_EXPORT set(const std::vector<int> &v);
  void HMDF_EXPORT set(const std::chrono::system_clock::time_point &t);
  void HMDF_EXPORT set(const Date &v);
  void HMDF_EXPORT set(int year, int month = 1, int day = 1, int hour = 0,
                       int minute = 0, int second = 0, int millisecond = 0);

  void HMDF_EXPORT fromSeconds(long seconds);

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

  static Date HMDF_EXPORT now();

 private:
  std::chrono::system_clock::time_point m_datetime;
};

template <typename T>
Date operator+(Date lhs, const T &rhs) {
  lhs += rhs;
  return lhs;
}

template <typename T>
Date operator-(Date lhs, const T &rhs) {
  lhs -= rhs;
  return lhs;
}

#ifndef SWIG
template Date HMDF_EXPORT operator+(Date, const short &);
template Date HMDF_EXPORT operator+(Date, const int &);
template Date HMDF_EXPORT operator+(Date, const long &);
template Date HMDF_EXPORT operator+(Date, const unsigned short &);
template Date HMDF_EXPORT operator+(Date, const unsigned int &);
template Date HMDF_EXPORT operator+(Date, const unsigned long &);
template Date HMDF_EXPORT operator+(Date, const float &);
template Date HMDF_EXPORT operator+(Date, const double &);
template Date HMDF_EXPORT operator+(Date, const Date::milliseconds &);
template Date HMDF_EXPORT operator+(Date, const Date::seconds &);
template Date HMDF_EXPORT operator+(Date, const Date::minutes &);
template Date HMDF_EXPORT operator+(Date, const Date::hours &);
template Date HMDF_EXPORT operator+(Date, const Date::days &);
template Date HMDF_EXPORT operator+(Date, const Date::months &);
template Date HMDF_EXPORT operator+(Date, const Date::weeks &);
template Date HMDF_EXPORT operator+(Date, const Date::years &);
template Date HMDF_EXPORT operator-(Date, const short &);
template Date HMDF_EXPORT operator-(Date, const int &);
template Date HMDF_EXPORT operator-(Date, const long &);
template Date HMDF_EXPORT operator-(Date, const unsigned short &);
template Date HMDF_EXPORT operator-(Date, const unsigned int &);
template Date HMDF_EXPORT operator-(Date, const unsigned long &);
template Date HMDF_EXPORT operator-(Date, const float &);
template Date HMDF_EXPORT operator-(Date, const double &);
template Date HMDF_EXPORT operator-(Date, const Date::milliseconds &);
template Date HMDF_EXPORT operator-(Date, const Date::seconds &);
template Date HMDF_EXPORT operator-(Date, const Date::minutes &);
template Date HMDF_EXPORT operator-(Date, const Date::hours &);
template Date HMDF_EXPORT operator-(Date, const Date::days &);
template Date HMDF_EXPORT operator-(Date, const Date::months &);
template Date HMDF_EXPORT operator-(Date, const Date::weeks &);
template Date HMDF_EXPORT operator-(Date, const Date::years &);
#endif

#endif  // HMDFDATE_H
