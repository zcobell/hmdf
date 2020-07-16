# ##############################################################################
# CMake Build File for HMDF
#
# Written By: Zach Cobell
#
# ##############################################################################
#
# The CMake build system enable HMDF to be deployed and built in a cross
# platform environment.
#
# ##############################################################################

# ##############################################################################
# GENERAL OPTIONS
# ##############################################################################
cmake_minimum_required(VERSION 3.0.2)
project(
  HMDF
  LANGUAGES C CXX
  VERSION 0.0.1)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckFunctionExists)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_11)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE
         "Enable -ftime-trace to generate time tracing .json files on clang"
         OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(project_options INTERFACE -ftime-trace)
  endif()
endif()

add_library(project_warnings INTERFACE)
include(cmake/Cache.cmake)
include(cmake/CompilerWarnings.cmake)
option(HMDF_PROJECT_WARNINGS "Enable compile time warnings as errors" OFF)
if(HMDF_PROJECT_WARNINGS)
  set_project_warnings(project_warnings)
endif()
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)
include(cmake/StaticAnalyzers.cmake)
# ##############################################################################

# ##############################################################################
# Enable running tests
# ##############################################################################
if(UNIX OR CYGWIN)
  enable_testing()
endif(UNIX OR CYGWIN)
# ##############################################################################

# ##############################################################################
# Enable Coverage
# ##############################################################################
# OPTION(COVERAGE "Export Code Coverage report from tests" OFF)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
if(COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    setup_target_for_coverage(hmdf_coverage tests coverage)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -pedantic -pthread -g -O0 -fprofile-arcs -ftest-coverage"
    )
  endif(CMAKE_COMPILER_IS_GNUCXX)
endif(COVERAGE)
# ##############################################################################

# ##############################################################################
# C++ 11/14 Check
# ##############################################################################
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
else(COMPILER_SUPPORTS_CXX11)
  message(
    FATAL_ERROR
      "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler."
  )
endif(COMPILER_SUPPORTS_CXX11)
# ##############################################################################

# ##############################################################################
# Compiler flags
# ##############################################################################
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
mark_as_advanced(CLEAR CMAKE_CXX_FLAGS_RELEASE)
mark_as_advanced(CLEAR CMAKE_CXX_FLAGS_DEBUG)
mark_as_advanced(CLEAR CMAKE_C_FLAGS_RELEASE)
mark_as_advanced(CLEAR CMAKE_C_FLAGS_DEBUG)
mark_as_advanced(CLEAR CMAKE_CXX_COMPILER)
mark_as_advanced(CLEAR CMAKE_C_COMPILER)
# ##############################################################################

# ##############################################################################
# LIBRARY VERSION
# ##############################################################################
set(HMDF_VERSION_MAJOR 0)
set(HMDF_VERSION_MINOR 1)
set(HMDF_VERSION_PATCH 0)
set(HMDF_VERSION_STRING
    ${HMDF_VERSION_MAJOR}.${HMDF_VERSION_MINOR}.${HMDF_VERSION_PATCH})
# ##############################################################################

# ##############################################################################
# CODE VERSION (GIT)
# ##############################################################################
execute_process(
  COMMAND git describe --always --tags
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  RESULT_VARIABLE GIT_RETURN_VALUE
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT "${GIT_RETURN_VALUE}" STREQUAL "0")
  set(GIT_VERSION "${HMDF_VERSION_STRING}.cv")
endif()
message(STATUS "HMDF Version: ${GIT_VERSION}")
# ##############################################################################

# ##############################################################################
# Boost
# ##############################################################################
find_package(Boost 1.60.0)
if(NOT Boost_FOUND OR PACKAGE_VERSION_UNSUITABLE)
  message(STATUS "Using included Boost Version: 1.73.0")
  set(Boost_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/boost_1_73_0)
endif()
# ##############################################################################

# ##############################################################################
# TESTING
# ##############################################################################
option(HMDF_BUILD_TESTS "Build test cases" OFF)
if(HMDF_BUILD_TESTS)
  enable_testing()
endif()
# ##############################################################################

# ##############################################################################
# SET THE LOCATION OF TEMPORARY STATIC LIBS
# ##############################################################################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles)
# ##############################################################################

# ##############################################################################
# SWIG/PYTHON/PERL Note: This will attempt to default to Python3 Give the flag
# -DPYTHON2=ON to force Python2 usage
# ##############################################################################
if(${PYTHON2})
  message(STATUS "CMake will attempt to use the Python2 interface only")
  find_package(PythonInterp 2.7 REQUIRED)
  find_package(PythonLibs 2.7 REQUIRED)
  set(PYTHONTYPE 2)
else()
  find_package(PythonInterp 3.0)
  find_package(PythonLibs 3.0)
  if(NOT PythonLibs_FOUND AND NOT PythonInterp_FOUND)
    find_package(PythonInterp 2.7)
    find_package(PythonLibs 2.7)
    set(PYTHONTYPE 2)
  else()
    set(PYTHONTYPE 3)
  endif()
endif()

find_package(SWIG 3.0)
if(SWIG_FOUND
   AND PythonLibs_FOUND
   AND PythonInterp_FOUND)
  include(${SWIG_USE_FILE})
  if(PythonLibs_FOUND)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -m site --user-site
      OUTPUT_VARIABLE temp_PYTHON_SITE_DIRECTORY
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(PYTHON_INSTALL_DIRECTORY
        ${temp_PYTHON_SITE_DIRECTORY}
        CACHE STRING "Location where Python libraries will be installed")
    mark_as_advanced(CLEAR PYTHON_EXECUTABLE)
  else(PythonLibs_FOUND)
    set(PYTHON_INSTALL_DIRECTORY
        "PYTHON_INSTALL_DIRECTORY-NOTFOUND"
        CACHE STRING "Location where Python libraries will be installed")
  endif(PythonLibs_FOUND)

  # FIND_PACKAGE(PerlLibs) IF(PerlLibs_FOUND) OPTION(TEST_PERL "Test the perl
  # build for SWIG. Note: Does not work yet" OFF) ENDIF(PerlLibs_FOUND)
else(
  SWIG_FOUND
  AND PythonLibs_FOUND
  AND PythonInterp_FOUND)
  message(WARNING "SWIG/Python libraries could not be found. PyHmdf disabled.")
endif(
  SWIG_FOUND
  AND PythonLibs_FOUND
  AND PythonInterp_FOUND)
# ##############################################################################

# ##############################################################################
# NETCDF
# ##############################################################################
if(WIN32)
  set(NETCDF_INCLUDE_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/netcdf/include")
else(WIN32)
  if(NOT "${NETCDFHOME}" STREQUAL "")
    set(NETCDF_DIR
        "${NETCDFHOME}"
        CACHE STRING "Location of NETCDF Library")
  elseif(NOT $ENV{NETCDFHOME} STREQUAL "")
    set(NETCDF_DIR
        $ENV{NETCDFHOME}
        CACHE STRING "Location of NETCDF Library")
  endif(NOT "${NETCDFHOME}" STREQUAL "")

  find_package(NetCDF)

  set(NETCDF_AdditionalLibs
      ""
      CACHE STRING "Additional libraries that may be required for netCDF")

  if(NOT NETCDF_FOUND)
    message(SEND_ERROR "Specify the netCDF path on the following screen")
  else(NOT NETCDF_FOUND)

    set(netcdf_c_code
        "
		#include <netcdf.h>
		int main(){
			int ncid,varid;
			int ierr = nc_def_var_deflate(ncid,varid,1,2,2);
			return 0;
		}
	")
    file(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/netcdf_c_check.c"
         "${netcdf_c_code}")
    try_compile(
      NC_DEFLATE_FOUND "${CMAKE_CURRENT_BINARY_DIR}"
      "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/netcdf_c_check.c"
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIRS}"
      LINK_LIBRARIES "${NETCDF_LIBRARIES}"
      LINK_LIBRARIES "${NETCDF_AdditionalLibs}"
      OUTPUT_VARIABLE LOG1)

    if(NC_DEFLATE_FOUND)
      set(NETCDF_LINKER_FLAG "${NETCDF_LIBRARIES}")
    else(NC_DEFLATE_FOUND)
      message(SEND_ERROR "The netCDF library is not functional.")
    endif(NC_DEFLATE_FOUND)
  endif(NOT NETCDF_FOUND)
endif(WIN32)
# ##############################################################################

# ##############################################################################
# LIBRARIES
# ##############################################################################
# ##############################################################################

# ##############################################################################
# External Library
# ##############################################################################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nefis EXCLUDE_FROM_ALL)
# ##############################################################################

# ##############################################################################
# HMDF Library
# ##############################################################################
set(HMDF_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hmdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/station.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timepoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/date.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nefisseriesmetadata.cpp)

add_library(hmdf SHARED ${HMDF_SOURCES})

set(HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/hmdf.h)

target_include_directories(
  hmdf
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nefis/include
          ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
          ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj/src
          ${Boost_INCLUDE_DIRS}
          ${NETCDF_INCLUDE_DIR})

add_dependencies(hmdf ezproj nefis)

target_compile_definitions(hmdf PRIVATE GIT_VERSION="${GIT_VERSION}")
target_compile_definitions(hmdf PRIVATE HMDF_LIBRARY)

set_target_properties(hmdf PROPERTIES VERSION ${HMDF_VERSION_STRING}
                                      SOVERSION ${HMDF_VERSION_MAJOR})
set_target_properties(hmdf PROPERTIES PUBLIC_HEADER "${HEADER_LIST}")

if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local"
   OR ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local/"
   OR ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/"
   OR ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/")
  set(HEADER_DEST "${CMAKE_INSTALL_INCLUDEDIR}/hmdf")
else()
  set(HEADER_DEST ${CMAKE_INSTALL_INCLUDEDIR})
endif()

write_basic_package_version_file(
  hmdfConfigVersion.cmake
  VERSION ${HMDF_VERSION_STRING}
  COMPATIBILITY SameMajorVersion)
install(
  TARGETS hmdf
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT HMDF_RUNTIME
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT HMDF_RUNTIME
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT HMDF_DEVELOPMENT
  PUBLIC_HEADER DESTINATION ${HEADER_DEST} COMPONENT HMDF_DEVELOPMENT)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hmdfConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)

set_target_properties(hmdf PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
set_target_properties(hmdf PROPERTIES CMAKE_CXX_INLINES_HIDDEN YES)

target_link_libraries(hmdf ezproj nefis project_warnings project_options)

if(WIN32)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/netcdf/libs_vc64)
  target_link_libraries(hmdf netcdf hdf5 hdf5_hl)
else(WIN32)
  target_link_libraries(hmdf ${NETCDF_LIBRARIES})
endif(WIN32)

if(APPLE)
  set(CMAKE_MACOSX_RPATH 0)
  set_target_properties(hmdf PROPERTIES INSTALL_NAME_DIR "hmdf")
  set_target_properties(hmdf PROPERTIES MACOSX_RPATH "hmdf")
endif(APPLE)

# ##############################################################################

# ##############################################################################
# PyHmdf Library
# ##############################################################################
if(SWIG_FOUND
   AND PythonLibs_FOUND
   AND PythonInterp_FOUND)

  if(PYTHONTYPE EQUAL 2)
    set(PYTHONFLAG "")
  else()
    set(PYTHONFLAG "-py3")
  endif()

  if(WIN32)

    # ...Builds a dummy cmake project Which will add some windows specific
    # include files not required by Unix/Cygwin [mingw64] Further testing
    # required to see if this is required by MSVC compilers
    file(
      WRITE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_wrapper.cmake
      "
            FILE(READ \${SRC} DATA)
            FILE(WRITE \${DST} \"#include <math.h>\")
            FILE(APPEND \${DST} \"\\n\\n\")
            FILE(APPEND \${DST} \"\${DATA}\")
        ")

    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx
      COMMAND
        ${SWIG_EXECUTABLE} -outdir ${CMAKE_CURRENT_BINARY_DIR} -c++ -python
        ${PYTHONFLAG} -I${CMAKE_CURRENT_SOURCE_DIR}/include
        -I${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
        -I${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj/src
        -I${PYTHON_INCLUDE_PATH} -o
        ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx.1
        ${CMAKE_CURRENT_SOURCE_DIR}/python/hmdf.i
      COMMAND
        ${CMAKE_COMMAND}
        -DSRC=\"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx.1\"
        -DDST=\"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx\" -P
        ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_wrapper.cmake
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating SWIG interface to Python...")

  else(WIN32)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx
      COMMAND
        ${SWIG_EXECUTABLE} -outdir ${CMAKE_CURRENT_BINARY_DIR} -c++ -python
        ${PYTHONFLAG} -I${CMAKE_CURRENT_SOURCE_DIR}/include
        -I${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
        -I${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj/src
        -I${PYTHON_INCLUDE_PATH} -o
        ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/python/hmdf.i
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating SWIG interface to Python...")
  endif(WIN32)

  add_library(PyHmdf SHARED
              ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx)
  target_include_directories(PyHmdf PRIVATE ${PYTHON_INCLUDE_PATH}
                                            ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(PyHmdf hmdf ${PYTHON_LIBRARY})
  set_target_properties(PyHmdf PROPERTIES PREFIX "_")
  set_target_properties(PyHmdf PROPERTIES INSTALL_NAME_DIR "PyHmdf")
  set_property(
    DIRECTORY
    APPEND
    PROPERTY ADDITIONAL_MAKE_CLEAN_FILES PyHmdf.py
             CMakeFiles/python_hmdf_wrap.cxx)

  add_dependencies(PyHmdf hmdf)

  if(WIN32)
    set_target_properties(
      PyHmdf PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    install(
      TARGETS PyHmdf
      LIBRARY DESTINATION bin
      RUNTIME DESTINATION ${PYTHON_INSTALL_DIRECTORY})
  elseif(CYGWIN)
    set_target_properties(PyHmdf PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                            ${CMAKE_CURRENT_BINARY_DIR})
    install(
      TARGETS PyHmdf
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION ${PYTHON_INSTALL_DIRECTORY})
  else(WIN32)
    set_target_properties(PyHmdf PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                            ${CMAKE_CURRENT_BINARY_DIR})
    install(TARGETS PyHmdf LIBRARY DESTINATION ${PYTHON_INSTALL_DIRECTORY})
  endif(WIN32)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PyHmdf.py
          DESTINATION ${PYTHON_INSTALL_DIRECTORY})

  if(WIN32)
    set_target_properties(PyHmdf PROPERTIES SUFFIX ".pyd")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      # 64 bits
      target_compile_definitions(PyHmdf PRIVATE "MS_WIN64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # 32 bits
      target_compile_definitions(PyHmdf PRIVATE "MS_WIN32")
    endif()
  elseif(APPLE)
    set_target_properties(PyHmdf PROPERTIES SUFFIX ".so")
  endif(WIN32)
endif(
  SWIG_FOUND
  AND PythonLibs_FOUND
  AND PythonInterp_FOUND)
# ##############################################################################

# ##############################################################################
# Test Cases
# ##############################################################################
if(UNIX OR CYGWIN)
  if(HMDF_BUILD_TESTS)
    # ...C++ Testing
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cxx)

    set(TEST_LIST "cxx_readAdcirc.cpp" "cxx_readAdcircNetcdf.cpp"
                  "cxx_readNefis.cpp")

    foreach(TESTFILE ${TEST_LIST})
      get_filename_component(TESTNAME ${TESTFILE} NAME_WE)
      add_executable(${TESTNAME} ${CMAKE_SOURCE_DIR}/testing/cxx/${TESTFILE})
      add_dependencies(${TESTNAME} hmdf)
      target_include_directories(${TESTNAME}
                                 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
      target_link_libraries(${TESTNAME} hmdf ${NETCDF_LIBS})
      set_target_properties(
        ${TESTNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                               ${CMAKE_CURRENT_BINARY_DIR}/cxx)

      add_test(
        NAME TEST_${TESTNAME}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cxx/${TESTNAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/testing)
      if(CYGWIN)
        set_tests_properties(
          TEST_${TESTNAME}
          PROPERTIES ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
      else(CYGWIN)
        set_tests_properties(
          TEST_${TESTNAME}
          PROPERTIES
            ENVIRONMENT
            "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${CMAKE_CURRENT_BINARY_DIR}")
      endif(CYGWIN)
    endforeach()

    # ...Python Test
    if(SWIG_FOUND)
      option(HMDF_PYTHON_TESTING "Run python3 tests" OFF)
      if(HMDF_PYTHON_TESTING)
        set(PYTHON_TESTNAME TEST_python3_PyHmdf)
        add_test(
          NAME ${PYTHON_TESTNAME}
          COMMAND ${PYTHON_EXECUTABLE}
                  ${CMAKE_SOURCE_DIR}/testing/python/adcirc_read.py
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_property(
          TEST ${PYTHON_TESTNAME}
          PROPERTY
            ENVIRONMENT
            "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${CMAKE_CURRENT_BINARY_DIR}")
        set_property(
          TEST ${PYTHON_TESTNAME} APPEND
          PROPERTY ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
      endif()
    endif(SWIG_FOUND)
  endif(HMDF_BUILD_TESTS)
endif(UNIX OR CYGWIN)
# ##############################################################################
