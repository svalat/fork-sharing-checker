######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.2.0-dev                    #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Use gtest
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

######################################################
add_definitions(-DTEST_BIN_DIR="${CMAKE_CURRENT_BINARY_DIR}")

######################################################
set(TEST_NAMES TestProcPagemapReader)

######################################################
FOREACH(test_name ${TEST_NAMES})
	add_executable(${test_name} ${test_name}.cpp)
	target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} fork-sharing-checker)
	fsc_add_test(${test_name} ${test_name})
ENDFOREACH(test_name)
