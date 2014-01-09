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
<<<<<<< HEAD
project( xrootd-rados-cms )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/cmake )

set(CMAKE_INSTALL_PREFIX /usr)

=======
project( RadosOss )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/cmake )

>>>>>>> 1c333d6... Initial Import
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
<<<<<<< HEAD
# 'gitdist' target
#-------------------------------------------------------------------------------
add_custom_target(gitdist
    COMMAND git archive --prefix=${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}/ HEAD
        | gzip > ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}.tar.gz
=======
# 'dist' target
#-------------------------------------------------------------------------------
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${RELEASE}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.deps/;/.bzr/;~$;'.'o$;/CMakeFiles/;Makefile;/_CPack_Packages/;.tar.gz$;.tar.bz2;CMakeCache.txt;build;install_manifest.txt;/lib/;/.git/;${CPACK_SOURCE_IGNORE_FILES};")

#-------------------------------------------------------------------------------
# 'gitdist' target
#-------------------------------------------------------------------------------
add_custom_target(gitdist
    COMMAND git archive --prefix=${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}/ HEAD
        | gzip > ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${RELEASE}.tar.gz
>>>>>>> 1c333d6... Initial Import
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#-------------------------------------------------------------------------------
# 'srpm' target
#-------------------------------------------------------------------------------
<<<<<<< HEAD
ADD_CUSTOM_TARGET(sourcerpm COMMAND rpmbuild -ts ${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}.tar.gz)
ADD_DEPENDENCIES(sourcerpm gitdist)
=======
ADD_CUSTOM_TARGET(srpm COMMAND rpmbuild -ts ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${RELEASE}.tar.gz)
ADD_DEPENDENCIES(srpm dist)
>>>>>>> 1c333d6... Initial Import

#-------------------------------------------------------------------------------
# 'rpm' target
#-------------------------------------------------------------------------------
<<<<<<< HEAD
ADD_CUSTOM_TARGET(binaryrpm COMMAND rpmbuild -tb ${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE}.tar.gz)
ADD_DEPENDENCIES(binaryrpm gitdist)
=======
ADD_CUSTOM_TARGET(rpm COMMAND rpmbuild -tb ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${RELEASE}.tar.gz)
ADD_DEPENDENCIES(rpm dist)
>>>>>>> 1c333d6... Initial Import
