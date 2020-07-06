#ifndef LOGGING_H
#define LOGGING_H
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

#include <iostream>
#include <string>

#include "hmdf_global.h"

/**
 * @class Logging
 * @author Zachary Cobell
 * @copyright Copyright 2015-2020 Zachary Cobell. All Rights Reserved. This
 * project is released under the terms of the GNU General Public License v3
 * @brief The Logging class is used to throw errors and log messages to standard
 * output
 */
class Logging {
 public:
  Logging() = default;

  static void throwError(const std::string &s);
  static void throwError(const std::string &s, const char *file, int line);

  static void logError(const std::string &s,
                       const std::string &heading = std::string());
  static void warning(const std::string &s,
                      const std::string &heading = std::string());
  static void log(const std::string &s,
                  const std::string &heading = std::string());

 private:
  static void printMessage(const std::string &header,
                           const std::string &message);
  static void printErrorMessage(const std::string &header,
                                const std::string &message);
};

/**
 * @def hmdf_throw_exception
 * @brief Throws an exception to the user with the file and line number sources
 * from which the exception was thrown
 * @param arg string describing the error that is being thrown
 */
#define hmdf_throw_exception(arg) Logging::throwError(arg, __FILE__, __LINE__)

#endif  // ADCMOD_LOGGING_H
