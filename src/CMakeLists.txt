#************************************************************************
# * Rados CMS Plugin for XRootD                                          *
# * Copyright © 2014 CERN/Switzerland                                    *
# *                                                                      *
# * Author: Andreas-Joachim Peters <andreas.joachim.peters@cern.ch>      *
# *                                                                      *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation, either version 3 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# * This program is distributed in the hope that it will be useful,      *
# * GNU General Public License for more details.                         *
# *                                                                      *
# * You should have received a copy of the GNU General Public License    *
# * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
# ************************************************************************/

find_package( XRootD REQUIRED )
find_package( LibRados REQUIRED )

include_directories( ./  
		     ${RADOS_INCLUDE_DIR} 
		     ${XROOTD_INCLUDE_DIR} 
		     ${XROOTD_PRIVATE_INCLUDE_DIR} 
           ceph/src/)

link_directories( ${RADOS_LIB_DIR} )

add_library( RadosCms MODULE
	          RadosCms.cc         RadosCms.hh
	         )


target_link_libraries( RadosCms ${RADOS_LIB} )

if (Linux)
  set_target_properties ( RadosLib PROPERTIES
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    SOVERSION ${VERSION_MAJOR}
    CLEAN_DIRECT_OUTPUT 1
  )
endif(Linux)

install( TARGETS RadosCms
         LIBRARY DESTINATION ${LIB_INSTALL_DIR}
         ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

