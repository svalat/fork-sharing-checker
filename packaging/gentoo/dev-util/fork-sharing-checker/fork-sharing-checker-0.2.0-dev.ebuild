######################################################
#            PROJECT  : fork-sharing-checking        #
#            VERSION  : 0.2.0-dev                    #
#            DATE     : 03/2016                      #
#            AUTHOR   : Valat Sébastien - CERN       #
#            LICENSE  : CeCILL-C                     #
######################################################

EAPI=4

inherit cmake-utils

RESTRICT="primaryuri"
DESCRIPTION="A short tool to check page sharing after forking to track COW (Copy On Write) usage."
HOMEPAGE="https://github.com/svalat/fork-sharing-checking"
SRC_URI="https://github.com/downloads/svalat/fork-sharing-checking/fork-sharing-checking-0.1.0.tar.bz2"

LICENSE="CeCILL-C"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE=""

DEPEND=""
RDEPEND=""

src_configure() {
	#local mycmakeargs=(-DXXX=YYY)
	cmake-utils_src_configure
}