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

#ifndef CEPH_ERASURE_CODE_PLUGIN_LRC_H
#define CEPH_ERASURE_CODE_PLUGIN_LRC_H

#include "erasure-code/ErasureCodePlugin.h"

class ErasureCodePluginOptLrc : public ceph::ErasureCodePlugin {
public:
  int factory(const std::string &directory,
		      ceph::ErasureCodeProfile &profile,
		      ceph::ErasureCodeInterfaceRef *erasure_code,
		      std::ostream *ss) override;
};

#endif