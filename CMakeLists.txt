
###########################################################################
#                     CMake Build File for HMDF
#
#    Written By: Zach Cobell
#
###########################################################################
#
# The CMake build system enable HMDF to be deployed and built
# in a cross platform environment. 
#
###########################################################################
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckLibraryExists) 
INCLUDE (CheckFunctionExists)
INCLUDE (GNUInstallDirs)
INCLUDE (CMakePackageConfigHelpers)

#...Set the default build type
IF(DEFINED CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
        build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
        Release RelWithDebInfo MinSizeRel.")
ELSEIF(COVERAGE)
        SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build,
            options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
            RelWithDebInfo MinSizeRel.")
ELSE()
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build,
        options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
        RelWithDebInfo MinSizeRel.")
ENDIF()

###########################################################################
#  GENERAL OPTIONS
###########################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT(HMDF)
###########################################################################


###########################################################################
# Enable running tests
###########################################################################
IF(UNIX OR CYGWIN)
    ENABLE_TESTING()
ENDIF(UNIX OR CYGWIN)
###########################################################################


###########################################################################
# Enable Coverage
###########################################################################
#OPTION(COVERAGE "Export Code Coverage report from tests" OFF)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
IF(COVERAGE)
    IF(CMAKE_COMPILER_IS_GNUCXX) 
        INCLUDE(CodeCoverage)
        setup_target_for_coverage(hmdf_coverage tests coverage)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -pthread -g -O0 -fprofile-arcs -ftest-coverage")
    ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(COVERAGE)
###########################################################################


###########################################################################
# C++ 11/14 Check
###########################################################################
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
IF(COMPILER_SUPPORTS_CXX11)
    SET(CMAKE_CXX_STANDARD 11)
    SET(CMAKE_CXX_STANDARD_REQUIRED ON)
ELSE(COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
ENDIF(COMPILER_SUPPORTS_CXX11)
###########################################################################


###########################################################################
#  Compiler flags 
###########################################################################
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
MARK_AS_ADVANCED( CLEAR CMAKE_CXX_FLAGS_RELEASE )
MARK_AS_ADVANCED( CLEAR CMAKE_CXX_FLAGS_DEBUG )
MARK_AS_ADVANCED( CLEAR CMAKE_C_FLAGS_RELEASE )
MARK_AS_ADVANCED( CLEAR CMAKE_C_FLAGS_DEBUG )
MARK_AS_ADVANCED( CLEAR CMAKE_CXX_COMPILER )
MARK_AS_ADVANCED( CLEAR CMAKE_C_COMPILER )
###########################################################################

###########################################################################
#  LIBRARY VERSION
###########################################################################
set(HMDF_VERSION_MAJOR 0)
set(HMDF_VERSION_MINOR 1)
set(HMDF_VERSION_PATCH 0)
set(HMDF_VERSION_STRING ${HMDF_VERSION_MAJOR}.${HMDF_VERSION_MINOR}.${HMDF_VERSION_PATCH})
###########################################################################

###########################################################################
# CODE VERSION (GIT)
###########################################################################
EXECUTE_PROCESS( COMMAND git describe --always --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_VERSION
                    RESULT_VARIABLE GIT_RETURN_VALUE
                    OUTPUT_STRIP_TRAILING_WHITESPACE )
IF( NOT "${GIT_RETURN_VALUE}" STREQUAL "0" )
    SET(GIT_VERSION "${HMDF_VERSION_STRING}.cv")
ENDIF()
MESSAGE(STATUS "HMDF Version: ${GIT_VERSION}")
###########################################################################

###########################################################################
# Boost
###########################################################################
FIND_PACKAGE( Boost 1.60.0 )
IF( NOT Boost_FOUND OR PACKAGE_VERSION_UNSUITABLE )
    MESSAGE( STATUS "Using included Boost Version: 1.73.0")
    SET( Boost_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/boost_1_73_0 )
ENDIF()
###########################################################################

###########################################################################
# TESTING 
###########################################################################
OPTION(BUILD_TESTS "Build test cases" OFF)
###########################################################################

###########################################################################
#  SET THE LOCATION OF TEMPORARY STATIC LIBS
###########################################################################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles)
###########################################################################

###########################################################################
#  SWIG/PYTHON/PERL
#  Note: This will attempt to default to Python3
#  Give the flag -DPYTHON2=ON to force Python2 usage
###########################################################################
IF(${PYTHON2})
    MESSAGE(STATUS "CMake will attempt to use the Python2 interface only")
    FIND_PACKAGE(PythonInterp 2.7 REQUIRED )
    FIND_PACKAGE(PythonLibs 2.7 REQUIRED )
    SET(PYTHONTYPE 2)
ELSE()
    FIND_PACKAGE(PythonInterp 3.0)
    FIND_PACKAGE(PythonLibs 3.0)
    IF(NOT PythonLibs_FOUND AND NOT PythonInterp_FOUND)
        FIND_PACKAGE(PythonInterp 2.7)
        FIND_PACKAGE(PythonLibs 2.7)
        SET(PYTHONTYPE 2)
    ELSE()
        SET(PYTHONTYPE 3)
    ENDIF()
ENDIF()

FIND_PACKAGE(SWIG 3.0)
IF(SWIG_FOUND AND PythonLibs_FOUND AND PythonInterp_FOUND)
    INCLUDE(${SWIG_USE_FILE})
    IF(PythonLibs_FOUND)
        OPTION(PYTHON_TESTING "Test the Python Libraries with CTest" OFF)
        EXECUTE_PROCESS( COMMAND ${PYTHON_EXECUTABLE} -m site --user-site 
                         OUTPUT_VARIABLE temp_PYTHON_SITE_DIRECTORY OUTPUT_STRIP_TRAILING_WHITESPACE)
        SET( PYTHON_INSTALL_DIRECTORY ${temp_PYTHON_SITE_DIRECTORY} CACHE STRING "Location where Python libraries will be installed" )
        MARK_AS_ADVANCED( CLEAR PYTHON_EXECUTABLE )
    ELSE(PythonLibs_FOUND)
        SET( PYTHON_INSTALL_DIRECTORY "PYTHON_INSTALL_DIRECTORY-NOTFOUND" CACHE STRING "Location where Python libraries will be installed" )
    ENDIF(PythonLibs_FOUND)
    
    #FIND_PACKAGE(PerlLibs)
    #IF(PerlLibs_FOUND)
    #    OPTION(TEST_PERL "Test the perl build for SWIG. Note: Does not work yet" OFF) 
    #ENDIF(PerlLibs_FOUND)
ELSE(SWIG_FOUND AND PythonLibs_FOUND AND PythonInterp_FOUND)
    MESSAGE( WARNING "SWIG/Python libraries could not be found. PyHmdf disabled.")
ENDIF(SWIG_FOUND AND PythonLibs_FOUND AND PythonInterp_FOUND)
###########################################################################
    
    
###########################################################################
#  NETCDF
###########################################################################
IF(WIN32)
    SET(NETCDF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/netcdf/include" ) 
ELSE(WIN32)
	IF(NOT "${NETCDFHOME}" STREQUAL "")
        SET(NETCDF_DIR "${NETCDFHOME}" CACHE STRING "Location of NETCDF Library")
	ELSEIF(NOT $ENV{NETCDFHOME} STREQUAL "")
        SET(NETCDF_DIR $ENV{NETCDFHOME} CACHE STRING "Location of NETCDF Library")
	ENDIF(NOT "${NETCDFHOME}" STREQUAL "")

	FIND_PACKAGE(NetCDF)

	SET(NETCDF_AdditionalLibs "" CACHE STRING "Additional libraries that may be required for netCDF")

	IF(NOT NETCDF_FOUND)
		MESSAGE(SEND_ERROR "Specify the netCDF path on the following screen")
	ELSE(NOT NETCDF_FOUND)
		
		SET(netcdf_c_code
	"
		#include <netcdf.h>
		int main(){
			int ncid,varid;
			int ierr = nc_def_var_deflate(ncid,varid,1,2,2);
			return 0;
		}
	"
		)
		FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/netcdf_c_check.c" "${netcdf_c_code}")
		TRY_COMPILE(NC_DEFLATE_FOUND "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/netcdf_c_check.c" CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIRS}" LINK_LIBRARIES "${NETCDF_LIBRARIES}" LINK_LIBRARIES "${NETCDF_AdditionalLibs}" OUTPUT_VARIABLE LOG1)
			
		IF(NC_DEFLATE_FOUND)
			SET(NETCDF_LINKER_FLAG "${NETCDF_LIBRARIES}")
		ELSE(NC_DEFLATE_FOUND)
			MESSAGE(SEND_ERROR "The netCDF library is not functional.")
		ENDIF(NC_DEFLATE_FOUND)
	ENDIF(NOT NETCDF_FOUND)
ENDIF(WIN32)
###########################################################################

###########################################################################
#  LIBRARIES    
###########################################################################
###########################################################################

###########################################################################
#  Proj Library
###########################################################################
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj EXCLUDE_FROM_ALL )
###########################################################################

###########################################################################
#  HMDF Library
###########################################################################
SET( HMDF_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hmdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/station.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timepoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdate.cpp )

ADD_LIBRARY( hmdf SHARED ${HMDF_SOURCES} )

SET(HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/hmdf.h )

TARGET_INCLUDE_DIRECTORIES( hmdf PRIVATE
                                ${CMAKE_CURRENT_SOURCE_DIR}/src
                                ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj/src
                                ${Boost_INCLUDE_DIRS} 
                                ${NETCDF_INCLUDE_DIR} ) 

ADD_DEPENDENCIES( hmdf ezproj )

TARGET_COMPILE_DEFINITIONS( hmdf PRIVATE GIT_VERSION="${GIT_VERSION}")
TARGET_COMPILE_DEFINITIONS( hmdf PRIVATE HMDF_LIBRARY )

SET_TARGET_PROPERTIES( hmdf PROPERTIES VERSION ${HMDF_VERSION_STRING} SOVERSION ${HMDF_VERSION_MAJOR} )
SET_TARGET_PROPERTIES( hmdf PROPERTIES PUBLIC_HEADER "${HEADER_LIST}" ) 

IF(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local" OR ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local/" OR
   ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/" OR ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/")
    SET(HEADER_DEST "${CMAKE_INSTALL_INCLUDEDIR}/hmdf")
ELSE()
    SET(HEADER_DEST ${CMAKE_INSTALL_INCLUDEDIR})
ENDIF()

WRITE_BASIC_PACKAGE_VERSION_FILE( hmdfConfigVersion.cmake VERSION ${HMDF_VERSION_STRING} COMPATIBILITY SameMajorVersion )
INSTALL( TARGETS hmdf RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT HMDF_RUNTIME
                      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT HMDF_RUNTIME
                      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT HMDF_DEVELOPMENT
                      PUBLIC_HEADER DESTINATION ${HEADER_DEST}    COMPONENT HMDF_DEVELOPMENT )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/hmdfConfigVersion.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake )

SET_TARGET_PROPERTIES(hmdf PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
SET_TARGET_PROPERTIES(hmdf PROPERTIES CMAKE_CXX_INLINES_HIDDEN YES)

TARGET_LINK_LIBRARIES( hmdf ezproj )

IF(WIN32)
    LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/netcdf/libs_vc64 )
    TARGET_LINK_LIBRARIES( hmdf netcdf hdf5 hdf5_hl )
ELSE(WIN32)
    TARGET_LINK_LIBRARIES( hmdf ${NETCDF_LIBRARIES} )
ENDIF(WIN32)

IF(APPLE)
    SET(CMAKE_MACOSX_RPATH 0)
    SET_TARGET_PROPERTIES(hmdf PROPERTIES INSTALL_NAME_DIR "hmdf")
    SET_TARGET_PROPERTIES(hmdf PROPERTIES MACOSX_RPATH "hmdf")
ENDIF(APPLE)

###########################################################################

###########################################################################
#  PyHmdf Library
###########################################################################
IF(SWIG_FOUND AND PythonLibs_FOUND AND PythonInterp_FOUND)

    IF(PYTHONTYPE EQUAL 2)
        SET(PYTHONFLAG "")
    ELSE()
        SET(PYTHONFLAG "-py3")
    ENDIF()
    
    IF(WIN32)

        #...Builds a dummy cmake project
        #   Which will add some windows specific include
        #   files not required by Unix/Cygwin [mingw64]
        #   Further testing required to see if this is 
        #   required by MSVC compilers
        FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_wrapper.cmake "
            FILE(READ \${SRC} DATA)
            FILE(WRITE \${DST} \"#include <math.h>\")
            FILE(APPEND \${DST} \"\\n\\n\")
            FILE(APPEND \${DST} \"\${DATA}\")
        ")

        ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx
            COMMAND ${SWIG_EXECUTABLE} -outdir ${CMAKE_CURRENT_BINARY_DIR} -c++ -python ${PYTHONFLAG}
            -I${CMAKE_CURRENT_SOURCE_DIR}/src 
                    -I${PYTHON_INCLUDE_PATH}
                    -o ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx.1
                    ${CMAKE_CURRENT_SOURCE_DIR}/swig/hmdf.i
                    COMMAND ${CMAKE_COMMAND} -DSRC=\"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx.1\" 
                                     -DDST=\"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx\"
                                     -P ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_wrapper.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating SWIG interface to Python..."
        )

    ELSE(WIN32)
        ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx
            COMMAND ${SWIG_EXECUTABLE} -outdir ${CMAKE_CURRENT_BINARY_DIR} -c++ -python ${PYTHONFLAG}
                    -I${CMAKE_CURRENT_SOURCE_DIR}/src 
                    -I${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj/src 
                    -I${PYTHON_INCLUDE_PATH}  
                    -o ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/hmdf.i
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating SWIG interface to Python..."
        )
    ENDIF(WIN32)

    ADD_LIBRARY( PyHmdf SHARED ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/python_hmdf_wrap.cxx )
    TARGET_INCLUDE_DIRECTORIES( PyHmdf PRIVATE ${PYTHON_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/src ) 
    TARGET_LINK_LIBRARIES( PyHmdf hmdf ${PYTHON_LIBRARY} )
    SET_TARGET_PROPERTIES( PyHmdf PROPERTIES PREFIX "_")
    SET_TARGET_PROPERTIES( PyHmdf PROPERTIES INSTALL_NAME_DIR "PyHmdf")
    SET_PROPERTY( DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES PyHmdf.py CMakeFiles/python_hmdf_wrap.cxx )

    ADD_DEPENDENCIES( PyHmdf hmdf )

    IF(WIN32)
        SET_TARGET_PROPERTIES( PyHmdf PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
                                      INSTALL(TARGETS PyHmdf LIBRARY DESTINATION bin RUNTIME DESTINATION ${PYTHON_INSTALL_DIRECTORY})
    ELSEIF(CYGWIN)
        SET_TARGET_PROPERTIES( PyHmdf PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        INSTALL(TARGETS PyHmdf LIBRARY DESTINATION lib RUNTIME DESTINATION ${PYTHON_INSTALL_DIRECTORY})
    ELSE(WIN32)
        SET_TARGET_PROPERTIES( PyHmdf PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        INSTALL(TARGETS PyHmdf LIBRARY DESTINATION ${PYTHON_INSTALL_DIRECTORY})
    ENDIF(WIN32)
    
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/PyHmdf.py DESTINATION ${PYTHON_INSTALL_DIRECTORY})
    
    IF(WIN32)
        SET_TARGET_PROPERTIES( PyHmdf PROPERTIES SUFFIX ".pyd")
        IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64 bits
            TARGET_COMPILE_DEFINITIONS(PyHmdf PRIVATE "MS_WIN64")
        ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
            # 32 bits
            TARGET_COMPILE_DEFINITIONS(PyHmdf PRIVATE "MS_WIN32")
        ENDIF()
    ELSEIF(APPLE)
        SET_TARGET_PROPERTIES( PyHmdf PROPERTIES SUFFIX ".so")
    ENDIF(WIN32)
ENDIF(SWIG_FOUND AND PythonLibs_FOUND AND PythonInterp_FOUND)
###########################################################################

###########################################################################
# Test Cases
###########################################################################
IF(UNIX OR CYGWIN)
    IF(BUILD_TESTS)
        #...C++ Testing
        FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cxx_testcases)

        SET(TEST_LIST "" ) 

        FOREACH(TESTFILE ${TEST_LIST} ) 
            GET_FILENAME_COMPONENT( TESTNAME ${TESTFILE} NAME_WE )
            ADD_EXECUTABLE( ${TESTNAME} ${CMAKE_SOURCE_DIR}/testing/cxx_tests/${TESTFILE} )
            ADD_DEPENDENCIES( ${TESTNAME} adcircmodules )
            TARGET_INCLUDE_DIRECTORIES( ${TESTNAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src 
                                                            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ezproj/src ) 
            TARGET_LINK_LIBRARIES( ${TESTNAME} adcircmodules )
            SET_TARGET_PROPERTIES( ${TESTNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cxx_testcases )
    
            ADD_TEST( NAME TEST_${TESTNAME} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cxx_testcases/${TESTNAME}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/testing )
            IF(CYGWIN)
                SET_TESTS_PROPERTIES( TEST_${TESTNAME} PROPERTIES ENVIRONMENT "PATH=$ENV{PATH}:${CMAKE_CURRENT_BINARY_DIR}")
            ELSE(CYGWIN)
                SET_TESTS_PROPERTIES( TEST_${TESTNAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${CMAKE_CURRENT_BINARY_DIR}")
            ENDIF(CYGWIN)
        ENDFOREACH()

        #...Python Test
        IF(PYTHON_TESTING)
            SET(PYTHON_TESTNAME TEST_python3_PyHmdf)
            ADD_TEST( NAME ${PYTHON_TESTNAME} COMMAND ${PYTHON_EXECUTABLE} 
                      ${CMAKE_SOURCE_DIR}/testing/python/adcirc_read.py
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
            SET_PROPERTY( TEST ${PYTHON_TESTNAME} PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${CMAKE_CURRENT_BINARY_DIR}")
            SET_PROPERTY( TEST ${PYTHON_TESTNAME} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
        ENDIF(PYTHON_TESTING)
    ENDIF(BUILD_TESTS)
ENDIF(UNIX OR CYGWIN)
###########################################################################
