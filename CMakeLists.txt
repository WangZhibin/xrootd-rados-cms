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

#-------------------------------------------------------------------------------
# Project description
#-------------------------------------------------------------------------------
set( VERSION_MAJOR "1" )
set( VERSION_MINOR "0" )
set( VERSION_PATCH "0")

cmake_minimum_required( VERSION 2.6 )
project( xrootd-rados-cms )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/cmake )

set(CMAKE_INSTALL_PREFIX /usr)

if( MacOSX )
  set( LIB_INSTALL_DIR lib )
else( MacOSX )
  set( LIB_INSTALL_DIR lib64 )
endif( MacOSX )

add_subdirectory( src )



#-------------------------------------------------------------------------------
# Packaging
#-------------------------------------------------------------------------------

if (RELEASE)
else (RELEASE)
set(RELEASE "head")
endif (RELEASE)

include(CPack)

#-------------------------------------------------------------------------------
# 'uninstall' target
#-------------------------------------------------------------------------------

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake") 


#-------------------------------------------------------------------------------
# 'gitdist' target
#-------------------------------------------------------------------------------
add_custom_target(gitdist
    COMMAND git archive --prefix=${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}/ HEAD
        | gzip > ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}.tar.gz
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#-------------------------------------------------------------------------------
# 'srpm' target
#-------------------------------------------------------------------------------
ADD_CUSTOM_TARGET(srpm COMMAND rpmbuild -ts ${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}.tar.gz)
ADD_DEPENDENCIES(srpm gitdist)

#-------------------------------------------------------------------------------
# 'rpm' target
#-------------------------------------------------------------------------------
ADD_CUSTOM_TARGET(rpm COMMAND rpmbuild -tb ${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}.tar.gz)
ADD_DEPENDENCIES(rpm gitdist)
