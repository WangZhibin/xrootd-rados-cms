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


#ifndef __XROOTD_PLUGIN_RADOSLOCATOR_HH__
#define __XROOTD_PLUGIN_RADOSLOCATOR_HH__

#include "librados/IoCtxImpl.h"
#include "XrdSys/XrdSysDNS.hh"

class RadosLocator {
public:

  struct Location {
    std::string Ip;
    std::string HostName;
    int64_t OsdID;

    std::string Dump() {
      char p[1024];
      snprintf(p, sizeof (p) - 1,
               "osd-id=%lld ip=%s host=%s",
               OsdID, Ip.c_str(),
               HostName.c_str());
      return std::string(p);
    }
  };
  typedef struct Location location_t;

  RadosLocator () { };

  virtual
  ~RadosLocator () { };

  static bool
  Locate (rados_ioctx_t io, const char *o,
          std::vector<RadosLocator::location_t> &locations
          ) {
    librados::IoCtxImpl *ctx = (librados::IoCtxImpl *)io;
    object_t oid(o);
    OSDMap* osdmap = ctx->objecter->osdmap;
    int64_t pool = ctx->poolid;
    if (!osdmap->have_pg_pool(pool)) {
      return false;
    }

    object_locator_t loc(pool);
    pg_t raw_pgid = osdmap->object_locator_to_pg(oid, loc);
    pg_t pgid = osdmap->raw_pg_to_pg(raw_pgid);

    vector<int> acting;
    osdmap->pg_to_acting_osds(pgid, acting);

    for (size_t i = 0; i < acting.size(); i++) {
      std::stringstream hostaddr;
      entity_addr_t addr = osdmap->get_addr(acting[i]);
      hostaddr << addr.ss_addr();
      std::string host = hostaddr.str().c_str();
      host.erase(host.find(":"));
      struct sockaddr inetaddr;

      XrdSysDNS::getHostAddr(host.c_str(),
                             inetaddr);

      RadosLocator::location_t new_location;
      new_location.Ip = host;
      new_location.Ip += ":1094";
      new_location.HostName = XrdSysDNS::getHostName(inetaddr);
      new_location.HostName += ":1094";
      new_location.OsdID = acting[i];
      locations.push_back(new_location);
    }
  }
};
#endif	/* __XROOTD_PLUGIN_RADOSLCOATOR_HH__ */

