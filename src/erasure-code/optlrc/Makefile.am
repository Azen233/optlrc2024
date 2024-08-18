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

# lrc plugin
optlrc_sources = \
	erasure-code/jerasure/jerasure/src/cauchy.c \
	erasure-code/jerasure/jerasure/src/galois.c \
	erasure-code/jerasure/jerasure/src/jerasure.c \
	erasure-code/jerasure/jerasure/src/liberation.c \
	erasure-code/jerasure/jerasure/src/reed_sol.c \
	erasure-code/jerasure/gf-complete/src/gf_wgen.c \
	erasure-code/jerasure/gf-complete/src/gf_method.c \
	erasure-code/jerasure/gf-complete/src/gf_w16.c \
	erasure-code/jerasure/gf-complete/src/gf.c \
	erasure-code/jerasure/gf-complete/src/gf_w32.c \
	erasure-code/jerasure/gf-complete/src/gf_w64.c \
	erasure-code/jerasure/gf-complete/src/gf_w128.c \
	erasure-code/jerasure/gf-complete/src/gf_general.c \
	erasure-code/jerasure/gf-complete/src/gf_w4.c \
	erasure-code/jerasure/gf-complete/src/gf_rand.c \
	erasure-code/jerasure/gf-complete/src/gf_w8.c \
	erasure-code/ErasureCode.cc \
	erasure-code/optlrc/ErasureCodePluginOptLrc.cc \
	erasure-code/optlrc/ErasureCodeOptLrc.cc

noinst_HEADERS += \
	erasure-code/optlrc/ErasureCodeOptLrc.h \
	erasure-code/jerasure/jerasure/include/cauchy.h \
	erasure-code/jerasure/jerasure/include/galois.h \
	erasure-code/jerasure/jerasure/include/jerasure.h \
	erasure-code/jerasure/jerasure/include/liberation.h \
	erasure-code/jerasure/jerasure/include/reed_sol.h \
	erasure-code/jerasure/gf-complete/include/gf_int.h \
	erasure-code/jerasure/gf-complete/include/gf_complete.h \
	erasure-code/jerasure/gf-complete/include/gf_rand.h \
	erasure-code/jerasure/gf-complete/include/gf_method.h \
	erasure-code/jerasure/gf-complete/include/gf_general.h

erasure-code/optlrc/ErasureCodePluginOptLrc.cc: ./ceph_ver.h

libec_optlrc_la_SOURCES = ${optlrc_sources} common/str_map.cc
libec_optlrc_la_CFLAGS = ${AM_CFLAGS} \
	-I$(srcdir)/erasure-code/jerasure/jerasure/include \
	-I$(srcdir)/erasure-code/jerasure/gf-complete/include \
	-I$(srcdir)/erasure-code/jerasure \
	-I$(srcdir)/erasure-code/optlrc
libec_optlrc_la_CXXFLAGS= ${AM_CXXFLAGS} \
	-I$(srcdir)/erasure-code/jerasure/jerasure/include \
	-I$(srcdir)/erasure-code/jerasure/gf-complete/include \
	-I$(srcdir)/erasure-code/jerasure \
	-I$(srcdir)/erasure-code/optlrc
libec_optlrc_la_LIBADD = $(LIBCRUSH) $(PTHREAD_LIBS) $(LIBJSON_SPIRIT)
libec_optlrc_la_LDFLAGS = ${AM_LDFLAGS} -module -avoid-version -shared
if LINUX
libec_optlrc_la_LDFLAGS += -export-symbols-regex '.*__erasure_code_.*'
endif

erasure_codelib_LTLIBRARIES += libec_optlrc.la
