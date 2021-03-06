#!/usr/bin/env python3

import PyHmdf


#...Read an ADCIRC fort.61 with a duplicate record
m1 = PyHmdf.HmdfData("../testing/test_files/fort_dup.61",PyHmdf.Date(2019,2,5,0,0,0),"../testing/test_files/stations.csv")
ierr = m1.read()

#...Read a netCDF ADCIRC fort.72.nc 
m2 = PyHmdf.HmdfData("../testing/test_files/fort.72.nc",PyHmdf.Date(2019,2,5,0,0,0))
ierr = m2.read()

#...Find and remove the duplicate records in the data
m1.sanitize()

#...Reproject the file to UTM-Zone 15
m1.reproject(26915)

#...Show a description of WSE station 1 
m1.station(1).show()
print(" ")

#...Show a description of wind station 2
m2.station(2).show()
print(m2.station(2)(1).date().toString("%Y-%m"))

d = PyHmdf.Date()
d.fromString("2019-01-02","%Y-%m-%d")
print(d.toString())

exit(0)
