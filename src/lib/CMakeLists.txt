######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.1.0                        #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
include_directories(../../extern-deps/from-htopml/)

######################################################
find_package(Threads REQUIRED)

######################################################
set(STACK_SIZE "2" CACHE STRING "Set size of stack to capture.")
add_definitions(-DCOW_MINI_STACK_SIZE=${STACK_SIZE})

######################################################
add_library(fork-sharing-checker SHARED Reader.cpp
                                        Checker.cpp
                                        COWTracker.cpp
                                        ProcPagemapReader.cpp 
                                        LinuxProcMapReader.cpp 
                                        $<TARGET_OBJECTS:htopml-json>)
target_link_libraries(fork-sharing-checker ${CMAKE_THREAD_LIBS_INIT})
                                        
######################################################
add_subdirectory(tests)

######################################################
INSTALL(TARGETS fork-sharing-checker
	ARCHIVE DESTINATION lib${LIBSUFFIX}
	LIBRARY DESTINATION lib${LIBSUFFIX})
INSTALL(FILES Checker.h LinuxProcMapReader.hpp ProcPagemapReader.hpp Reader.hpp
	DESTINATION include/fork-sharing-checker)