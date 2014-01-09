XRootD Rados CMS Plugin
=======================================

This project contains a plug-in for the **XRootD** server to locate objects in a
RADOS pool.

It is published under the GPLv3 license: http://www.gnu.org/licenses/gpl-3.0.html


Setup
======

This plug-in is loaded by the XRootD Server. In order to accomplish this, it is
necessary to indicate the server where the plug-in is by adding the following
line to the server's configuration file:

  ofs.cmslib /path/to/libRadosCms.so

It is necessary to tell which Ceph cluster it should interface with. This is done
by adding the following file to the same configuration file indicated above:

  radoscms.config /path/to/your/ceph.conf

The Rados CMNS plug-in maps pools with object names' prefixes. Thus, it is possible
to configure which prefixes point to which pools by setting the *radoscms.pools*
variable in the configuration file similar to the following example:

  radososs.pools /test/:mytestpool /:data

If you need the Rados instance to be created with a specific user name, you can do
it in the following way (myusername is an existing user name):

  radososs.user myusername
