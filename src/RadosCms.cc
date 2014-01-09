/************************************************************************
 * Rados CMS Plugin for XRootD                                          *
 * Copyright © 2014 CERN/Switzerland                                    *
 *                                                                      *
 * Author: Andreas-Joachim Peters <andreas.joachim.peters@cern.ch>      *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 ************************************************************************/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
#include <unistd.h>
/*----------------------------------------------------------------------------*/
#include "RadosCms.hh"
/*----------------------------------------------------------------------------*/
#include "XrdOss/XrdOss.hh"
#include "XrdSfs/XrdSfsInterface.hh"
#include "XrdSys/XrdSysError.hh"
#include "XrdSys/XrdSysLogger.hh"
#include "XrdOuc/XrdOucTrace.hh"
#include "XrdOuc/XrdOucEnv.hh"
/*----------------------------------------------------------------------------*/

// Singleton variable
static XrdCmsClient* instance = NULL;

using namespace XrdCms;

namespace XrdCms {
  XrdSysError  RadosCmsError( 0, "EosLfc_" );
  XrdOucTrace  Trace( &RadosCmsError );
};

//------------------------------------------------------------------------------
// CMS Client Instantiator
//------------------------------------------------------------------------------
extern "C"
{
  XrdCmsClient* XrdCmsGetClient( XrdSysLogger* logger,
                                 int           opMode,
                                 int           myPort,
                                 XrdOss*       theSS )
  {
    if ( instance ) {
      return static_cast<XrdCmsClient*>( instance );
    }

    instance = new RadosCms( logger );
    return static_cast<XrdCmsClient*>( instance );
  }
}


//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
RadosCms::RadosCms( XrdSysLogger* logger ):
  XrdCmsClient( XrdCmsClient::amRemote ),
  mRedirPort( 1094 )
{
  RadosCmsError.logger( logger );
  mRoot.clear();
  mRedirHost.clear();
}


//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
RadosCms::~RadosCms()
{
}


//------------------------------------------------------------------------------
// Configure
//------------------------------------------------------------------------------
int
RadosCms::Configure( const char* cfn, char* params, XrdOucEnv* EnvInfo )
{
  RadosCmsError.Emsg( "Configure", "Init RadosCms plugin with params:", params );
  
  return 1;
}


//------------------------------------------------------------------------------
// Locate
//------------------------------------------------------------------------------
int
RadosCms::Locate( XrdOucErrInfo& Resp,
                      const char*    path,
                      int            flags,
                      XrdOucEnv*     Info )
{
  XrdSecEntity* sec_entity = NULL;

  if ( Info ) {
    sec_entity = const_cast<XrdSecEntity*>( Info->secEnv() );
  }

  if ( !sec_entity ) {
    sec_entity = new XrdSecEntity( "" );
    sec_entity->tident = new char[16];
    sec_entity->tident = strncpy( sec_entity->tident, "unknown", 7 );
    sec_entity->tident[7]='\0';
  }
  
  std::string retString ;
 
  Resp.setErrCode( mRedirPort );
  retString = mRedirHost;
  retString += "?radoscms=1";
  
  Resp.setErrData( retString.c_str() );
  return SFS_REDIRECT;
}


//------------------------------------------------------------------------------
// Space
//------------------------------------------------------------------------------
int
RadosCms::Space( XrdOucErrInfo& Resp,
                     const char*    path,
                     XrdOucEnv*     Info )
{
  //............................................................................
  // Will return the space in the attached RADOS pool
  //............................................................................
  return 0;
}


