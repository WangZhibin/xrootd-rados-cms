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
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
/*----------------------------------------------------------------------------*/
#include "RadosCms.hh"
/*----------------------------------------------------------------------------*/
#include "XrdOss/XrdOss.hh"
#include "XrdSfs/XrdSfsInterface.hh"
#include "XrdSys/XrdSysError.hh"
#include "XrdSys/XrdSysLogger.hh"
#include "XrdSys/XrdSysTimer.hh"
#include "XrdOuc/XrdOucTrace.hh"
#include "XrdOuc/XrdOucEnv.hh"
#include "XrdOuc/XrdOucString.hh"
#include "XrdOuc/XrdOucTokenizer.hh"
/*----------------------------------------------------------------------------*/

// Singleton variable
static XrdCmsClient* instance = NULL;

using namespace XrdCms;

namespace XrdCms
{
  XrdSysError RadosCmsError (0, "RadosCms_");
  XrdOucTrace Trace (&RadosCmsError);
};

//------------------------------------------------------------------------------
// CMS Client Instantiator
//------------------------------------------------------------------------------
extern "C"
{

  XrdCmsClient*
  XrdCmsGetClient (XrdSysLogger* logger,
                   int opMode,
                   int myPort,
                   XrdOss* theSS)
  {
    if (instance)
    {
      return static_cast<XrdCmsClient*> (instance);
    }

    instance = new RadosCms(logger);
    return static_cast<XrdCmsClient*> (instance);
  }
}

//------------------------------------------------------------------------------
// OsdMap Thread Start Function
//------------------------------------------------------------------------------

void*
RadosCms::StaticOsdMapThread (void* arg)
{
  return reinterpret_cast<RadosCms*> (arg)->RefreshOsdMap();
}


//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------

RadosCms::RadosCms (XrdSysLogger* logger) :
XrdCmsClient (XrdCmsClient::amRemote),
mRedirPort (1094)
{
  RadosCmsError.logger(logger);
  mRoot.clear();
  mRedirHost.clear();
  mOsdDumpFile = "/var/tmp/xrootd-rados-cms.osd.dump";
  mRadosUser = "atlassfst";

  XrdSysThread::Run(&mOsdMapThread,
                    RadosCms::StaticOsdMapThread,
                    static_cast<void *> (this),
                    XRDSYSTHREAD_HOLD,
                    "OsdMap Thread");
}


//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------

RadosCms::~RadosCms ()
{
  if (mOsdMapThread)
  {
    XrdSysThread::Cancel(mOsdMapThread);
    XrdSysThread::Join(mOsdMapThread, NULL);
    mOsdMapThread = 0;
  }
}


//------------------------------------------------------------------------------
// Configure
//------------------------------------------------------------------------------

int
RadosCms::Configure (const char* cfn, char* params, XrdOucEnv* EnvInfo)
{
  RadosCmsError.Emsg("Configure", "Init RadosCms plugin with params:", params);
  
  std::string path;
  std::string user;
  return !LoadConfig(cfn, path, user);
}

//------------------------------------------------------------------------------
// LoadCOnfig
//------------------------------------------------------------------------------

int
RadosCms::LoadConfig (const char *pluginConf,
            std::string &configPath,
            std::string &userName)
{
  return 0;
}


//------------------------------------------------------------------------------
// Locate
//------------------------------------------------------------------------------

int
RadosCms::Locate (XrdOucErrInfo& Resp,
                  const char* path,
                  int flags,
                  XrdOucEnv* Info)
{
  XrdSecEntity* sec_entity = NULL;

  if (Info)
  {
    sec_entity = const_cast<XrdSecEntity*> (Info->secEnv());
  }

  if (!sec_entity)
  {
    sec_entity = new XrdSecEntity("");
    sec_entity->tident = new char[16];
    sec_entity->tident = strncpy(sec_entity->tident, "unknown", 7);
    sec_entity->tident[7] = '\0';
  }

  std::string retString;

  Resp.setErrCode(mRedirPort);
  retString = mRedirHost;
  retString += "?radoscms=1";

  Resp.setErrData(retString.c_str());
  return SFS_REDIRECT;
}


//------------------------------------------------------------------------------
// Space
//------------------------------------------------------------------------------

int
RadosCms::Space (XrdOucErrInfo& Resp,
                 const char* path,
                 XrdOucEnv* Info)
{
  //............................................................................
  // Will return the space in the attached RADOS pool
  //............................................................................
  return 0;
}

//------------------------------------------------------------------------------
// Refresh OsdMap
//------------------------------------------------------------------------------

void*
RadosCms::RefreshOsdMap ()
{
  XrdSysTimer sleeper;
  while (1)
  {
    XrdSysThread::SetCancelOff();
    DumpOsdMap();
    XrdSysThread::SetCancelOn();
    sleeper.Wait(10000);
  }
  return 0;
}


//------------------------------------------------------------------------------
// DumpOsdMap
//------------------------------------------------------------------------------

bool
RadosCms::DumpOsdMap ()
{
  RadosCmsError.Emsg("DumpOsdMap","refreshing the OSD map");

  std::string lOsDump = "ceph --id ";
  lOsDump += mRadosUser;
  lOsDump += " osd dump | grep \"osd\\.\" > ";
  lOsDump += mOsdDumpFile;

  int rc = system(lOsDump.c_str());
  if (WEXITSTATUS(rc))
  {
    fprintf(stderr, "error: <%s> failed with retc=%d",
            lOsDump.c_str(),
            WEXITSTATUS(rc));
    return false;
  }

  // load the current osd configuration into a string
  std::string lOsdConfig = "";
  {
    std::ifstream load(mOsdDumpFile.c_str());
    std::stringstream buffer;

    buffer << load.rdbuf();
    lOsdConfig = buffer.str();
  }

  // parse the configuration into the OsdMap
  XrdOucTokenizer tokenizer((char*) lOsdConfig.c_str());

  while (tokenizer.GetLine())
  {
    XrdOucString osd = tokenizer.GetToken();
    osd.replace("osd.", "");
    errno = 0;
    uint64_t id = strtoull(osd.c_str(), 0, 10);
    if (!errno)
    {
      mOsdMap_tmp[id].mId = id;
      mOsdMap_tmp[id].mActive = tokenizer.GetToken();
      mOsdMap_tmp[id].mState = tokenizer.GetToken();
      XrdOucString tag = tokenizer.GetToken();
      if (tag == "weight")
      {
        XrdOucString weight = tokenizer.GetToken();
        mOsdMap_tmp[id].mWeight = atof(weight.length() ? weight.c_str() : "");
      }
      const char* val = 0;
      while (val = tokenizer.GetToken())
      {
        XrdOucString ip = val;
        int pos = 0;
        if ((pos = ip.find(":")) != STR_NPOS)
        {
          ip.erase(pos);
          mOsdMap_tmp[id].mIp = ip.c_str();
          break;
        }
      }
    }
    fprintf(stderr, "     # OSD[ %04d ] active=%-10s state=%-10s weight=%.02f ip=[%s]\n",
            id,
            mOsdMap_tmp[id].mActive.c_str(),
            mOsdMap_tmp[id].mState.c_str(),
            mOsdMap_tmp[id].mWeight,
            mOsdMap_tmp[id].mIp.c_str());

    // assign the new OSD map
    XrdSysMutexHelper lLock(mOsdMapMutex);
    mOsdMap = mOsdMap_tmp;
  }
  return true;
}

