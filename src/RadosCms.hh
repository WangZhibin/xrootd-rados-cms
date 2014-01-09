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


#ifndef __XROOTD_PLUGIN_RADOSCMS_HH__
#define __XROOTD_PLUGIN_RADOSCMS_HH__

/*----------------------------------------------------------------------------*/
#include <string>
/*----------------------------------------------------------------------------*/
#include "XrdCms/XrdCmsClient.hh"
#include "XrdOuc/XrdOucErrInfo.hh"
#include "XrdOuc/XrdOucTList.hh"
#include "XrdSys/XrdSysPthread.hh"
#include "XrdSec/XrdSecEntity.hh"
/*----------------------------------------------------------------------------*/


//------------------------------------------------------------------------------
//! Plugin class for the XrdCmsClient to deal with LFC mappings
//------------------------------------------------------------------------------
class RadosCms: public XrdCmsClient
{
  public:

    //--------------------------------------------------------------------------
    //! Constructor
    //--------------------------------------------------------------------------
    RadosCms(XrdSysLogger* logger);


    //--------------------------------------------------------------------------
    //! Destructor
    //--------------------------------------------------------------------------
    virtual ~RadosCms();
 

    //--------------------------------------------------------------------------
    //! Configure() is called to configure the client. If the client is obtained
    //!            via a plug-in then Parms are the  parameters specified after
    //!            cmslib path. It is zero if no parameters exist.
    //! @return:   If successful, true must be returned; otherwise, false
    //!
    //--------------------------------------------------------------------------
    virtual int Configure( const char* cfn, char* Parms, XrdOucEnv* EnvInfo );


    //--------------------------------------------------------------------------
    //! Locate() is called to retrieve file location information. It is only used
    //!        on a manager node. This can be the list of servers that have a
    //!        file or the single server that the client should be sent to. The
    //!        "flags" indicate what is required and how to process the request.
    //!
    //!        SFS_O_LOCATE  - return the list of servers that have the file.
    //!                        Otherwise, redirect to the best server for the file.
    //!        SFS_O_NOWAIT  - w/ SFS_O_LOCATE return readily available info.
    //!                        Otherwise, select online files only.
    //!        SFS_O_CREAT   - file will be created.
    //!        SFS_O_NOWAIT  - select server if file is online.
    //!        SFS_O_REPLICA - a replica of the file will be made.
    //!        SFS_O_STAT    - only stat() information wanted.
    //!        SFS_O_TRUNC   - file will be truncated.
    //!
    //!        For any the the above, additional flags are passed:
    //!        SFS_O_META    - data will not change (inode operation only)
    //!        SFS_O_RESET   - reset cached info and recaculate the location(s).
    //!        SFS_O_WRONLY  - file will be only written    (o/w RDWR   or RDONLY).
    //!        SFS_O_RDWR    - file may be read and written (o/w WRONLY or RDONLY).
    //!
    //! @return:  As explained above
    //!
    //--------------------------------------------------------------------------
    virtual int Locate( XrdOucErrInfo& Resp,
                        const char*    path,
                        int            flags,
                        XrdOucEnv*     Info = 0 );


    //--------------------------------------------------------------------------
    //! Space() is called to obtain the overall space usage of a cluster. It is
    //!         only called on manager nodes.
    //!
    //! @return: Space information as defined by the response to kYR_statfs. For a
    //!         typical implementation see XrdCmsNode::do_StatFS().
    //!
    //--------------------------------------------------------------------------
    virtual int Space( XrdOucErrInfo& Resp,
                       const char*    path,
                       XrdOucEnv*     Info = 0 );

  private:

    std::string mRoot;          ///< the root directory we are interested in
    std::string mRedirHost;     ///< host to where we redirect 
    unsigned int mRedirPort;    ///< port to where we redirect, by default 1094

};

#endif // __XROOTD_PLUGIN_RADOSCMS_HH__  
