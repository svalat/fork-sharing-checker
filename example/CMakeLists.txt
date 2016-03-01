######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.2.0-dev                    #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_executable(example-simple example-simple.cpp)
target_link_libraries(example-simple fork-sharing-checker)

######################################################
add_executable(example-timeline example-timeline.cpp)
target_link_libraries(example-timeline fork-sharing-checker)