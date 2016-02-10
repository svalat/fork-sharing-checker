######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.1.0-dev                    #
#            DATE     : 02/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
include_directories(../../extern-deps/from-htopml/)

######################################################
add_library(fork-sharing-checker SHARED Reader.cpp
                                        Checker.cpp
                                        ProcPagemapReader.cpp 
                                        LinuxProcMapReader.cpp 
                                        $<TARGET_OBJECTS:htopml-json>)

######################################################
add_subdirectory(tests)

######################################################
INSTALL(TARGETS fork-sharing-checker
	ARCHIVE DESTINATION lib${LIBSUFFIX}
	LIBRARY DESTINATION lib${LIBSUFFIX})
INSTALL(FILES Checker.h LinuxProcMapReader.hpp ProcPagemapReader.hpp Reader.hpp
	DESTINATION include/fork-sharing-checker)