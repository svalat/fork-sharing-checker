######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.1.0-dev                    #
#            DATE     : 02/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
configure_file(fork-sharing-checker.pc.in fork-sharing-checker.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fork-sharing-checker.pc DESTINATION share/pkgconfig)
