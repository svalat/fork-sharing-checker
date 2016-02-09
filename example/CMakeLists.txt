######################################################
#            PROJECT  : fork-sharing-checking        #
#            VERSION  : 0.1.0-dev                    #
#            DATE     : 02/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_executable(example main.cpp)
target_link_libraries(example fork-sharing-checker)