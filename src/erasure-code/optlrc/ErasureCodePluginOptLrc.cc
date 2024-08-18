/*
***   Implementation of Optimal-LRC plugin for Ceph   ***

Written by: Oleg Kolosov, Tel Aviv University
Date: 27/06/2018

The code for the implementation of USENIX ATC 2018 paper 
## On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes ###
https://www.usenix.org/conference/atc18/presentation/kolosov

Authors: 
Oleg Kolosov, School of Electrical Engineering, Tel Aviv University; 
Gala Yadgar, Computer Science Department, Technion and School of Electrical Engineering, Tel Aviv University; 
Matan Liram, Computer Science Department, Technion; 
Itzhak Tamo, School of Electrical Engineering, Tel Aviv University; 
Alexander Barg, Department of ECE/ISR, University of Maryland
*/

#include "ceph_ver.h"
#include "common/debug.h"
#include "ErasureCodeOptLrc.h"
#include "ErasureCodePluginOptLrc.h"
// re-include our assert
#include "include/ceph_assert.h"

#define dout_subsys ceph_subsys_osd
#undef dout_prefix
#define dout_prefix _prefix(_dout)

int ErasureCodePluginOptLrc::factory(const std::string &directory,
		      ceph::ErasureCodeProfile &profile,
		      ceph::ErasureCodeInterfaceRef *erasure_code,
		      std::ostream *ss) {
    ErasureCodeOptLrc *interface;
    interface = new ErasureCodeOptLrc();
    int r = interface->init(profile, ss);
    if (r) {
      delete interface;
      return r;
    }
    *erasure_code = ErasureCodeInterfaceRef(interface);
    return 0;
};

#ifndef BUILDING_FOR_EMBEDDED

const char *__erasure_code_version() { return CEPH_GIT_NICE_VER; }

int __erasure_code_init(char *plugin_name, char *directory)
{
  auto& instance = ceph::ErasureCodePluginRegistry::instance();
  auto plugin = std::make_unique<ErasureCodePluginOptLrc>();
  int r = instance.add(plugin_name, plugin.get());
  if (r == 0) {
    plugin.release();
  }
  return r;
}

#endif

