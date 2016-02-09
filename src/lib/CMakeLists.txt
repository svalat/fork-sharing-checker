######################################################
#            PROJECT  : fork-sharing-checking        #
#            VERSION  : 0.1.0                        #
#            DATE     : 02/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
include_directories(../../extern-deps/from-htopml/)

######################################################
add_library(fork-sharing-checker SHARED LinuxProcMapReader.cpp ProcPagemapReader.cpp ForkSharingChecker.cpp $<TARGET_OBJECTS:htopml-json>)

######################################################
add_subdirectory(tests)

######################################################
INSTALL(TARGETS fork-sharing-checker
	ARCHIVE DESTINATION lib${LIBSUFFIX}
	LIBRARY DESTINATION lib${LIBSUFFIX})