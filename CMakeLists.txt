######################################################
#            PROJECT  : fork-sharing-checking        #
#            VERSION  : 0.1.0                        #
#            DATE     : 02/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Usage of OBJECT keyword in add_library require 2.8.8 (or something like that, need to check)
cmake_minimum_required(VERSION 2.8.8)
project(fork-sharing-checker)

######################################################
#global setup
enable_testing()

######################################################
#Add internal search path for scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

######################################################
#Delegate some cmake scripts
include(${CMAKE_SOURCE_DIR}/cmake/macros.cmake)

######################################################
#Some global setup
setup_internal_gmock_and_gtest()
if (ENABLE_GCC_COVERAGE)
	fsc_enable_gcc_coverage()
endif(ENABLE_GCC_COVERAGE)

######################################################
#build subdirs
add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(extern-deps)