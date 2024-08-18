***   Implementation of Optimal-LRC plugin for Ceph   ***

Written by: Oleg Kolosov, Tel Aviv University
Date: 27/06/2018

The code for the implementation of USENIX ATC 2018 paper 
### On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes ###
https://www.usenix.org/conference/atc18/presentation/kolosov

Authors: 
Oleg Kolosov, School of Electrical Engineering, Tel Aviv University; 
Gala Yadgar, Computer Science Department, Technion and School of Electrical Engineering, Tel Aviv University; 
Matan Liram, Computer Science Department, Technion; 
Itzhak Tamo, School of Electrical Engineering, Tel Aviv University; 
Alexander Barg, Department of ECE/ISR, University of Maryland

-----------------------
Optimal-LRC
-----------------------
Optimal-LRC is a recently proposed [1] full-LRC . In this code, k data blocks and m global parities are divided into groups of size r, and a local parity is added to each group, allowing repair of any lost block by the r surviving blocks in its group. r does not necessarily divide m + k, but Optimal-LRC requires that n mod (r + 1) != 1.

In 'Optimal LRC codes for all lenghts n<= q' [4] a new construction was developed based on the original construction. In the new construiction Optimal-LRC has optimal minimum distance for all n mod (r + 1) != 1. This folder contains the new implementation (with the exception of having the current decode function support only a single erasure in a local group, while Optimal-LRC can support multiple erasures).


-----------------------
Implementation content
-----------------------

### Ceph ###

ceph/src/erasure-code/optlrc/CMakeLists.txt
* Build file

ceph/src/erasure-code/optlrc/ErasureCodeOptLrc.cc
* The implementation of Optimal-LRC plugin

ceph/src/erasure-code/optlrc/ErasureCodeOptLrc.h
* .h file of the implementaton of Optimal-LRC plugin, contains description of functions

ceph/src/erasure-code/optlrc/ErasureCodeOptLrc_configs.h
* Holds generator matrices of Optimal-LRC

ceph/src/erasure-code/optlrc/ErasureCodePluginOptLrc.cc
* Used for integration in Ceph's erasure-code infrastructure. 

ceph/src/erasure-code/optlrc/ErasureCodePluginOptLrc.h

ceph/src/erasure-code/optlrc/Makefile.am
* Build file

ceph/src/erasure-code/CMakeLists.txt
* Build file

### Matlab ###

ceph/src/erasure-code/optlrc/matlab/calc_optlrc.m
* The main function for calculating the generator matrix

ceph/src/erasure-code/optlrc/matlab/calc_optlrc_coef.m
* Greedy algorithm to calculate linear coefficients for the generator matrix.

ceph/src/erasure-code/optlrc/matlab/g_for_r_5.m
* r=5 is a special case as described in the original paper [1]. This function calculated the generator polynomial for this case.

ceph/src/erasure-code/optlrc/matlab/ppower.m
* Retunrs a polynomial in the power of 'p'

-----------------------
Implementation
-----------------------
ErasureCodeOptLrc.cc contains the main implementation of Optimal-LRC. It contains existing Ceph erasure-code functions used to intergrate with the Ceph erasure-code plugin and CRUSH, and also implementation of encode and decode functions for Optimal-LRC.
For encoding we use the external function jerasure_matrix_encode from the Jerasure library [2].

The encode proecess works the following:
1) Pass pointer to first chunk of data and coding chunks to a special optlrc encode function
2) optlrc_encode fetches the relevant generator matrix from ErasureCodeOptLrc_configs.h based on the code's n,k,r values
3) Convert the generator matrix to the format acceptable by jerasure_matrix_encode
4) Pass the generator matrix, pointers to data and coding chunks and blocksize to jerasure_matrix_encode for encoding

Decode process:
Our implementation of Optimal-LRC was devised to simulate only a single erasure in a local group, while Optimal-LRC in general supports repair of multiple erasures.

minimum_to_decode: Calculates the minimum number and IDs of chunks required to decode an erased chunk. It's based on the main assumption of our implementation, that we have at most one erasure in each local group.
If a chunk in a local group is erased, compiles a list of all surviving chunks.

decode_chunks:
1) Detects all erased chunks in the stripe (assumed to have at most one erasure in a local group)
2) Per each erasure, compiles a list of pointers to surviving blocks in its local group and passess them to optlrc_decode_local

optlrc_decode_local:
The structure of Optimal-LRC allows us to decode a chunk using a XOR operation on all other chunk it its local group. The function supports decode only in the case of a single erasure in a local group.
The function has a list of all surviving chunks in the local group and it goes over them one by one, XORing each one with the XOR of all previous chunks. The result is placed in the location of the erased chunk finalizing the reconstruction process.

-----------------------
Comments
-----------------------
- Non optimal implementation in:
ceph/src/erasure-code/lrc/ErasureCodeLrc.cc  (line 637)

Described in 'On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes' [3] section 5, this is the resulting fix:

          set_difference(i->chunks_as_set.begin(), i->chunks_as_set.end(),
             erasures_not_recovered.begin(), erasures_not_recovered.end(),
             inserter(layer_minimum, layer_minimum.end()));
       
Was fixed to:

          set<int> layer_maximum;
          set_difference(i->chunks_as_set.begin(), i->chunks_as_set.end(),
                        erasures_not_recovered.begin(),erasures_not_recovered.end(),
                        inserter(layer_maximum,layer_maximum.end()));
          unsigned int k=0;
          for (unsigned int j = 0; j < get_chunk_count(); ++j) {
              if (layer_maximum.count(j) != 0) {
                  k++;
                  layer_minimum.insert(j);
                  if ( k == i->erasure_code->get_data_chunk_count() )
                              break;
              }
          }
          
- The code was implemented on Ceph 12.0.2

- For the following (n,k,r) combinations the generator matrix has already been computed and is available in ErasureCodeOptLrc_configs.h: (9,4,2), (10,6,4), (10,6,3), (15,8,4), (15,8,5), (16,12,7), (18,12,3), (18,12,4), (15,10,4), (14,10,4), (12,8,3), (12,8,4), (17,12,4), (14,10,6), (14,10,5), (12,8,5), (17,12,5), (17,12,6), (15,10,5)

- Matlab code will generate a generator matrix for most (n,k,r), however for r=5 the code will generate a generator polynomial, but a generator matrix needs to be calculated manually.
The case of r=5 is mathematically unique since it doesn't fall into the general cases presented in the original paper [1]. It is also specifically mentioned in the original paper. 

-----------------------
Running instructions
-----------------------
## Matlab ##

Run with:
calc_optlrc (n,k,r)

## Build ##

Follow the instructions in /README.md

For the purpose of our research the source code was built with deb.

We have used the following instructions:

git clone https://github.com/olekol33/optlrc2018.git
cd optlrc2018
git submodule update --init --recursive
./install-deps.sh
dpkg-checkbuilddeps;
dpkg-buildpackage -j80


## Building Ceph Optimal-LRC pool ##

The running instructions are based on the original Ceph LRC plugin:
http://docs.ceph.com/docs/mimic/rados/operations/erasure-code-lrc/

For creating an Optimal-LRC pool use the following examle:

bin/ceph osd erasure-code-profile set myprofile \
     plugin=optlrc \
     mapping=DD_DD____ \
     n=9 \
     k=4 \
     r=2 \
     ruleset-steps='[
                     [ "chooseleaf", "osd", 9 ],
                    ]'	

-----------------------
Reference
-----------------------
[1] I. Tamo and A. Barg. A family of optimal locally recoverable codes. IEEE Transactions on Information Theory, 60(8):4661–4676, Aug 2014
[2] http://jerasure.org/jerasure-2.0/
[3] Kolosov, Oleg, Gala Yadgar, Matan Liram, Itzhak Tamo, and Alexander Barg. "On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes." 2018 USENIX Annual Technical Conference USENIX ATC 18. USENIX Association, 2018.
[4] Kolosov, Oleg, Alexander Barg, Itzhak Tamo, and Gala Yadgar. "Optimal LRC codes for all lenghts n<= q" arXiv preprint arXiv:1802.00157 (2018).
