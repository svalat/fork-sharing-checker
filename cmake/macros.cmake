######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.2.0-dev                    #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
set(ENABLE_VALGRIND     "no" CACHE BOOL "Enable automatic usage of valgrind memcheck on each unit test execution.")
set(ENABLE_JUNIT_OUTPUT "no" CACHE BOOL "Generate unit test output in junit format to be used by Jenkins or similar.")


######################################################
#Setup paths to gtest/gmock headers and library
MACRO(setup_internal_gmock_and_gtest)
	set(GMOCK_SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern-deps/gmock-1.7.0)
	set(GMOCK_INCLUDE_DIR ${GMOCK_SOURCE_DIR}/include)
	set(GMOCK_INCLUDE_DIRS ${GMOCK_SOURCE_DIR}/include)
	set(GMOCK_BOTH_LIBRARIES gmock gmock_main)
	set(GTEST_BOTH_LIBRARIES gtest)
	set(GTEST_INCLUDE_DIR ${GMOCK_SOURCE_DIR}/gtest/include/)
	set(GTEST_INCLUDE_DIRS ${GMOCK_SOURCE_DIR}/gtest/include/)
ENDMACRO(setup_internal_gmock_and_gtest)

######################################################
#For integration of tests in jenkins, but only in self-test mode
macro (fsc_add_test test_name)
	#steup wrapper
	if (ENABLE_VALGRIND)
			set(tmp_test_wrapper valgrind --xml=yes --xml-file=${CMAKE_CURRENT_BINARY_DIR}/${test_name}.memcheck.xml --tool=memcheck --leak-check=full --show-reachable=yes)
	else(ENABLE_VALGRIND)
			set(tmp_test_wrapper "")
	endif(ENABLE_VALGRIND)

	#setup output options
	if (ENABLE_JUNIT_OUTPUT)
		set(tmp_test_run_option --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${test_name}.gtest.xml)
	endif (ENABLE_JUNIT_OUTPUT)

	#gen text command and register
	add_test(${test_name} ${tmp_test_wrapper} ${CMAKE_CURRENT_BINARY_DIR}/${test_name} ${tmp_test_run_option})
endmacro (fsc_add_test)


######################################################
MACRO(fsc_enable_gcc_coverage)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
ENDMACRO(fsc_enable_gcc_coverage)
