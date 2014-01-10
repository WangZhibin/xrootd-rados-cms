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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
#include "XrdOuc/XrdOucStream.hh"
#include "XrdOuc/XrdOucString.hh"
#include "XrdOuc/XrdOucTokenizer.hh"
/*----------------------------------------------------------------------------*/

#define RADOS_CMS_CONFIG_PREFIX "radoscms"
#define RADOS_CONFIG (RADOS_CMS_CONFIG_PREFIX ".config")
#define RADOS_CONFIG_POOLS_KW (RADOS_CMS_CONFIG_PREFIX ".pools")
#define RADOS_CONFIG_USER (RADOS_CMS_CONFIG_PREFIX ".user")
#define DEFAULT_POOL_PREFIX "/"
#define LOG_PREFIX "------ RadosCms ---- "

// Singleton variable
static XrdCmsClient* instance = NULL;

using namespace XrdCms;

namespace XrdCms
{
  XrdSysError RadosCmsError (0, "RadosCms_");
  XrdOucTrace Trace (&RadosCmsError);
};

#include "RadosLocator.hh"
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
// Constructor
//------------------------------------------------------------------------------

RadosCms::RadosCms (XrdSysLogger* logger) :
XrdCmsClient (XrdCmsClient::amRemote),
mRedirPort (1094)
{
  RadosCmsError.logger(logger);
  mRoot.clear();
  mRedirHost.clear();
  mRadosUser = "admin";
  mCephCluster = 0;
  mCephConf = "/etc/ceph/ceph.conf";
}


//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------

RadosCms::~RadosCms ()
{
  
  if (mCephCluster)
    rados_shutdown(mCephCluster);
}


//------------------------------------------------------------------------------
// Configure
//------------------------------------------------------------------------------

int
RadosCms::Configure (const char* cfn, char* params, XrdOucEnv* EnvInfo)
{
  RadosCmsError.Emsg("Configure", "Init RadosCms plugin with params:", params);

  int retc = !LoadConfig(cfn, mCephConf, mCephUser);

  if (!mPoolMap.size())
  {
    RadosCmsError.Emsg("Configure",
                       "your pool map is empty! Define 'radoscms.pools=...'");
    return 0;
  }

  retc = rados_create(&mCephCluster, mCephUser.c_str());

  if (retc != 0)
  {
    RadosCmsError.Emsg("Problem when creating the cluster", strerror(-retc));
    return 0;
    ;
  }

  retc = rados_conf_read_file(mCephCluster, mCephConf.c_str());

  if (retc != 0)
  {
    RadosCmsError.Emsg("Problem when reading Ceph's config file",
                       mCephConf.c_str(), ":", strerror(-retc));
    return 0;
  }

  retc = rados_connect(mCephCluster);

  if (retc == 0 && mPoolMap.count(DEFAULT_POOL_PREFIX) == 0)
  {
    RadosCmsPool defaultPool = {GetDefaultPoolName()};
    mPoolMap[DEFAULT_POOL_PREFIX] = defaultPool;
    mPoolPrefixSet.insert(DEFAULT_POOL_PREFIX);

    RadosCmsError.Emsg("Got default pool name since none was configured",
                       DEFAULT_POOL_PREFIX, "=", defaultPool.name.c_str());
  }

  InitIoctxInPools();

  // do a test location
  rados_ioctx_t ioctx;
  std::string root = "/";

  if (!GetIoctxFromPath(root,
                        &ioctx))
  {
    std::vector<RadosLocator::location_t> root_locations;
    RadosLocator::Locate(ioctx, root.c_str(), root_locations);
    std::string out;

    for (size_t i = 0; i < root_locations.size(); i++)
    {
      out = "path=/ ";
      out += root_locations[i].Dump();
      RadosCmsError.Say(LOG_PREFIX, out.c_str());
    }
  }

  return 1;

}

//------------------------------------------------------------------------------
// LoadConfig
//------------------------------------------------------------------------------

int
RadosCms::LoadConfig (const char *pluginConf,
                      std::string &configPath,
                      std::string &userName)
{
  XrdOucStream Config;
  int cfgFD;
  char *var;

  if ((cfgFD = open(pluginConf, O_RDONLY, 0)) < 0)
    return cfgFD;

  Config.Attach(cfgFD);
  while ((var = Config.GetMyFirstWord()))
  {
    if (configPath == "" && strcmp(var, RADOS_CONFIG) == 0)
    {
      configPath = Config.GetWord();
    }
    else if (userName == "" && strcmp(var, RADOS_CONFIG_USER) == 0)
    {
      userName = Config.GetWord();
    }
    else if (strcmp(var, RADOS_CONFIG_POOLS_KW) == 0)
    {
      const char *pool;
      while (pool = Config.GetWord())
        AddPoolFromConfStr(pool);
    }
  }

  Config.Close();
  return 0;
}

//------------------------------------------------------------------------------
// GetDefaultPoolName
//------------------------------------------------------------------------------

std::string
RadosCms::GetDefaultPoolName () const
{
  const int poolListMaxSize = 1024;
  char poolList[poolListMaxSize];
  rados_pool_list(mCephCluster, poolList, poolListMaxSize);

  return poolList;
}

void
RadosCms::InitIoctxInPools ()
{
  std::map<std::string, RadosCmsPool>::iterator it = mPoolMap.begin();

  while (it != mPoolMap.end())
  {
    const std::string &key = (*it).first;
    RadosCmsPool &pool = (*it).second;
    int res = rados_ioctx_create(mCephCluster, pool.name.c_str(), &pool.ioctx);

    it++;

    if (res != 0)
    {
      RadosCmsError.Emsg("Problem creating pool from name",
                         pool.name.c_str(), strerror(-res));
      mPoolMap.erase(key);
    }
  }
}
//------------------------------------------------------------------------------
// AddPoolFromConfStr
//------------------------------------------------------------------------------

void
RadosCms::AddPoolFromConfStr (const char *confStr)
{
  int delimeterIndex;
  XrdOucString str(confStr);

  delimeterIndex = str.find(':');
  if (delimeterIndex == STR_NPOS || delimeterIndex == 0 ||
      delimeterIndex == str.length() - 1)
  {
    RadosCmsError.Emsg("Error splitting the pool conf str", confStr);
    return;
  }

  RadosCmsPool pool = {""};
  XrdOucString poolPrefix(str, 0, delimeterIndex - 1);
  XrdOucString poolName(str, delimeterIndex + 1);

  pool.name = poolName.c_str();

  RadosCmsError.Say(LOG_PREFIX "Found pool with name ", poolName.c_str(),
                    " for prefix ", poolPrefix.c_str());

  mPoolMap[poolPrefix.c_str()] = pool;
  mPoolPrefixSet.insert(poolPrefix.c_str());
}

//------------------------------------------------------------------------------
// GetPoolFromPath
//------------------------------------------------------------------------------

const RadosCmsPool*
RadosCms::GetPoolFromPath (const std::string &path)
{
std:
  set<std::string>::reverse_iterator it;
  for (it = mPoolPrefixSet.rbegin(); it != mPoolPrefixSet.rend(); it++)
  {
    if (path.compare(0, (*it).length(), *it) == 0)
      return &mPoolMap[*it];
  }

  return 0;
}

//------------------------------------------------------------------------------
// GetIoctxFromPath
//------------------------------------------------------------------------------

int
RadosCms::GetIoctxFromPath (const std::string &objectName,
                            rados_ioctx_t *ioctx)
{
  const RadosCmsPool *pool = GetPoolFromPath(objectName);

  if (!pool)
  {
    RadosCmsError.Emsg("No pool was found for object name", objectName.c_str());
    return -ENODEV;
  }

  *ioctx = pool->ioctx;

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

