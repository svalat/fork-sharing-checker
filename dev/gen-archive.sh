#!/bin/bash
######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.1.0                        #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#extract version
version=0.1.0
prefix=fork-sharing-checker-${version}

######################################################
echo "Generate ${prefix}.tar.gz..."
git archive --format=tar --prefix=${prefix}/ HEAD | gzip > ${prefix}.tar.gz
echo "Finished"
