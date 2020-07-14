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

/* Hmdf Interface File */
#if defined(SWIGPYTHON)
%module PyHmdf
#endif

%insert("python") %{
    import signal
    signal.signal(signal.SIGINT, signal.SIG_DFL)
%}

%{
    #define SWIG_FILE_WITH_INIT
    #include "hmdf_global.h"
    #include "hmdf.h"
    #include "station.h"
    #include "timepoint.h"
    #include "cdate.h"
%}

%include <std_string.i>
%include <exception.i>
%include <std_vector.i>
%include <windows.i>

%exception { 
  try { 
    $action 
  } catch (const std::exception& e) { 
    SWIG_exception(SWIG_RuntimeError, e.what()); 
  } catch (const std::string& e) { 
    SWIG_exception(SWIG_RuntimeError, e.c_str()); 
  } 
} 

namespace std {
    %template(IntVector) vector<int>;
    %template(SizetVector) vector<size_t>;
    %template(DoubleVector) vector<double>;
    %template(DoubleDoubleVector) vector<vector<double>>;
    %template(SizetSizetVector) vector<vector<size_t>>;
}

%include "hmdf_global.h"
%include "hmdf.h"
%include "station.h"
%include "timepoint.h"
%include "cdate.h"
