######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.2.0-dev                    #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_definitions(-DVIEW_PATH="${CMAKE_INSTALL_PREFIX}/share/fork-sharing-checker/view/")
add_executable(fork-sharing-checker-cmd main.cpp)
target_link_libraries(fork-sharing-checker-cmd fork-sharing-checker)

######################################################
install(TARGETS fork-sharing-checker-cmd RUNTIME DESTINATION bin RENAME fork-sharing-checker)