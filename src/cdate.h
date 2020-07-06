#ifndef CDATE_H
#define CDATE_H
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

class CDate {
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

  HMDF_EXPORT CDate();
  HMDF_EXPORT CDate(const std::chrono::system_clock::time_point &t);
  HMDF_EXPORT CDate(const std::vector<int> &v);
  HMDF_EXPORT CDate(const CDate &d);
  HMDF_EXPORT CDate(int year, int month = 1, int day = 1, int hour = 0,
                    int minute = 0, int second = 0, int millisecond = 0);

#ifndef SWIG
  //...operator overloads
  bool HMDF_EXPORT operator<(const CDate &d) const;
  bool HMDF_EXPORT operator>(const CDate &d) const;
  bool HMDF_EXPORT operator==(const CDate &d) const;
  bool HMDF_EXPORT operator!=(const CDate &d) const;

  template <class T, typename std::enable_if<std::is_integral<T>::value>::type
                         * = nullptr>
  CDate &operator+=(const T &rhs) {
    this->m_datetime += CDate::seconds(rhs);
    return *this;
  }

  template <class T, typename std::enable_if<
                         std::is_floating_point<T>::value>::type * = nullptr>
  CDate &operator+=(const T &rhs) {
    this->m_datetime +=
        CDate::milliseconds(static_cast<long>(std::floor(rhs * 1000.0)));
    return *this;
  }

  template <
      class T,
      typename std::enable_if<
          !std::is_integral<T>::value && !std::is_floating_point<T>::value &&
          !std::is_same<T, CDate::years>::value &&
          !std::is_same<T, CDate::months>::value>::type * = nullptr>
  CDate &operator+=(const T &rhs) {
    this->m_datetime += rhs;
    return *this;
  }

  CDate HMDF_EXPORT &operator+=(const CDate::years &rhs);
  CDate HMDF_EXPORT &operator+=(const CDate::months &rhs);

  template <class T, typename std::enable_if<std::is_integral<T>::value>::type
                         * = nullptr>
  CDate &operator-=(const T &rhs) {
    this->m_datetime -= CDate::seconds(rhs);
    return *this;
  }

  template <class T, typename std::enable_if<
                         std::is_floating_point<T>::value>::type * = nullptr>
  CDate &operator-=(const T &rhs) {
    this->m_datetime -=
        CDate::milliseconds(static_cast<long>(std::floor(rhs * 1000.0)));
    return *this;
  }

  template <
      class T,
      typename std::enable_if<
          !std::is_integral<T>::value && !std::is_floating_point<T>::value &&
          !std::is_same<T, CDate::years>::value &&
          !std::is_same<T, CDate::months>::value>::type * = nullptr>
  CDate &operator-=(const T &rhs) {
    this->m_datetime -= rhs;
    return *this;
  }

  CDate HMDF_EXPORT &operator-=(const CDate::years &rhs);
  CDate HMDF_EXPORT &operator-=(const CDate::months &rhs);

  friend std::ostream HMDF_EXPORT &operator<<(std::ostream &os,
                                              const CDate &dt) {
    os << dt.toString();
    return os;
  }

  // end operator overloads
#endif

  void HMDF_EXPORT addSeconds(const long &value);
  void HMDF_EXPORT addMinutes(const long &value);
  void HMDF_EXPORT addHours(const long &value);
  void HMDF_EXPORT addDays(const long &value);
  void HMDF_EXPORT addWeeks(const long &value);
  void HMDF_EXPORT addMonths(const long &value);
  void HMDF_EXPORT addYears(const long &value);

  static CDate maxDate() { return CDate(3000, 1, 1, 0, 0, 0); }
  static CDate minDate() { return CDate(1900, 1, 1, 0, 0, 0); }

  std::vector<int> HMDF_EXPORT get() const;

  void HMDF_EXPORT set(const std::vector<int> &v);
  void HMDF_EXPORT set(const std::chrono::system_clock::time_point &t);
  void HMDF_EXPORT set(const CDate &v);
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
  void HMDF_EXPORT setMinute(int month);

  int HMDF_EXPORT second() const;
  void HMDF_EXPORT setSecond(int second);

  int HMDF_EXPORT millisecond() const;
  void HMDF_EXPORT setMillisecond(int milliseconds);

  void HMDF_EXPORT fromString(const std::string &datestr);

  std::string HMDF_EXPORT toString() const;

  std::chrono::system_clock::time_point HMDF_EXPORT time_point() const;

  static CDate HMDF_EXPORT now();

 private:
  std::chrono::system_clock::time_point m_datetime;
};

template <typename T>
CDate operator+(CDate lhs, const T &rhs) {
  lhs += rhs;
  return lhs;
}

template <typename T>
CDate operator-(CDate lhs, const T &rhs) {
  lhs -= rhs;
  return lhs;
}

#ifndef SWIG
template CDate HMDF_EXPORT operator+(CDate, const short &);
template CDate HMDF_EXPORT operator+(CDate, const int &);
template CDate HMDF_EXPORT operator+(CDate, const long &);
template CDate HMDF_EXPORT operator+(CDate, const unsigned short &);
template CDate HMDF_EXPORT operator+(CDate, const unsigned int &);
template CDate HMDF_EXPORT operator+(CDate, const unsigned long &);
template CDate HMDF_EXPORT operator+(CDate, const float &);
template CDate HMDF_EXPORT operator+(CDate, const double &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::milliseconds &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::seconds &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::minutes &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::hours &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::days &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::months &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::weeks &);
template CDate HMDF_EXPORT operator+(CDate, const CDate::years &);
template CDate HMDF_EXPORT operator-(CDate, const short &);
template CDate HMDF_EXPORT operator-(CDate, const int &);
template CDate HMDF_EXPORT operator-(CDate, const long &);
template CDate HMDF_EXPORT operator-(CDate, const unsigned short &);
template CDate HMDF_EXPORT operator-(CDate, const unsigned int &);
template CDate HMDF_EXPORT operator-(CDate, const unsigned long &);
template CDate HMDF_EXPORT operator-(CDate, const float &);
template CDate HMDF_EXPORT operator-(CDate, const double &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::milliseconds &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::seconds &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::minutes &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::hours &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::days &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::months &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::weeks &);
template CDate HMDF_EXPORT operator-(CDate, const CDate::years &);
#endif

#endif  // CDATE_H
