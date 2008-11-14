cmake_minimum_required (VERSION 2.4)
project ( PTFS ) 
add_subdirectory( src )

FIND_LIBRARY(fuse_LIBRARY fuse
	/usr/lib
	/usr/local/lib)

IF(fuse_LIBRARY)
	MESSAGE(STATUS "Check for FUSE -- found") 
	INSTALL(PROGRAMS src/ptfs DESTINATION /usr/local/bin)
ELSE(fuse_LIBRARY)
	MESSAGE(STATUS "Check for FUSE -- not found") 
	MESSAGE(SEND_ERROR "Can't find FUSE")
ENDIF(fuse_LIBRARY)
