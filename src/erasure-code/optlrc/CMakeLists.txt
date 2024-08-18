#***   Implementation of Optimal-LRC plugin for Ceph   ***
#
#Written by: Oleg Kolosov, Tel Aviv University
#Date: 27/06/2018
#
#The code for the implementation of USENIX ATC 2018 paper 
### On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes ###
#https://www.usenix.org/conference/atc18/presentation/kolosov
#
#Authors: 
#Oleg Kolosov, School of Electrical Engineering, Tel Aviv University; 
#Gala Yadgar, Computer Science Department, Technion and School of Electrical Engineering, Tel Aviv University; 
#Matan Liram, Computer Science Department, Technion; 
#Itzhak Tamo, School of Electrical Engineering, Tel Aviv University; 
#Alexander Barg, Department of ECE/ISR, University of Maryland

#set(optlrc_objs
set(optlrc_srcs
  ErasureCodeOptLrc.cc
  ErasureCodePluginOptLrc.cc
  $<TARGET_OBJECTS:erasure_code_objs>
  $<TARGET_OBJECTS:crush_objs>
  ${CMAKE_SOURCE_DIR}/src/common/str_map.cc
)

#add_library(optlrc_objs OBJECT ${optlrc_srcs})

add_library(ec_optlrc SHARED ${optlrc_srcs}
  $<TARGET_OBJECTS:jerasure_objs>
  $<TARGET_OBJECTS:gf-complete_objs>)
#  $<TARGET_OBJECTS:jerasure_utils>)
#  $<TARGET_OBJECTS:optlrc_objs>)

target_link_libraries(ec_optlrc json_spirit)
set_target_properties(ec_optlrc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS ec_optlrc DESTINATION ${erasure_plugin_dir})
