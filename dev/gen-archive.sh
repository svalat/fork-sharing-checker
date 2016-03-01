#!/bin/bash
######################################################
#            PROJECT  : fork-sharing-checker         #
#            VERSION  : 0.2.0-dev                    #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#extract version
version=0.2.0-dev
prefix=fork-sharing-checker-${version}

######################################################
echo "Generate ${prefix}.tar.gz..."
git archive --format=tar --prefix=${prefix}/ HEAD | gzip > ${prefix}.tar.gz
echo "Finished"
