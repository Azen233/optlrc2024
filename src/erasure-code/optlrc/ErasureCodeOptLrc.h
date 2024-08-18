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

#ifndef CEPH_ERASURE_CODE_OPTLRC_H
#define CEPH_ERASURE_CODE_OPTLRC_H
#include "erasure-code/ErasureCode.h"
#include "include/err.h"
#include "json_spirit/json_spirit.h"
#include "include/buffer.h"

#define DEFAULT_RULESET_ROOT "default"
#define DEFAULT_RULESET_FAILURE_DOMAIN "host"
#define ERROR_LRC_ARRAY			-(MAX_ERRNO + 1)
#define ERROR_LRC_OBJECT		-(MAX_ERRNO + 2)
#define ERROR_LRC_INT			-(MAX_ERRNO + 3)
#define ERROR_LRC_STR			-(MAX_ERRNO + 4)
#define ERROR_LRC_PLUGIN		-(MAX_ERRNO + 5)
#define ERROR_LRC_DESCRIPTION		-(MAX_ERRNO + 6)
#define ERROR_LRC_PARSE_JSON		-(MAX_ERRNO + 7)
#define ERROR_LRC_MAPPING		-(MAX_ERRNO + 8)
#define ERROR_LRC_MAPPING_SIZE		-(MAX_ERRNO + 9)
#define ERROR_LRC_FIRST_MAPPING		-(MAX_ERRNO + 10)
#define ERROR_LRC_COUNT_CONSTRAINT	-(MAX_ERRNO + 11)
#define ERROR_LRC_CONFIG_OPTIONS	-(MAX_ERRNO + 12)
#define ERROR_LRC_LAYERS_COUNT		-(MAX_ERRNO + 13)
#define ERROR_LRC_RULESET_OP		-(MAX_ERRNO + 14)
#define ERROR_LRC_RULESET_TYPE		-(MAX_ERRNO + 15)
#define ERROR_LRC_RULESET_N		-(MAX_ERRNO + 16)
#define ERROR_LRC_ALL_OR_NOTHING	-(MAX_ERRNO + 17)
#define ERROR_LRC_GENERATED		-(MAX_ERRNO + 18)
#define ERROR_LRC_K_M_MODULO		-(MAX_ERRNO + 19)
#define ERROR_LRC_K_MODULO		-(MAX_ERRNO + 20)
#define ERROR_LRC_M_MODULO		-(MAX_ERRNO + 21)

class ErasureCodeOptLrc : public ErasureCode {
public:
  int n;
  std::string DEFAULT_N;
  int k;
  std::string DEFAULT_K;
  int r;
  std::string DEFAULT_R;
  std::string ruleset_root;
  std::vector<int> mapping;
  struct Step {
    Step(std::string _op, std::string _type, int _n) :
      op(_op),
      type(_type),
      n(_n) {}
    std::string op;
    std::string type;
    int n;
  };
  std::vector<Step> ruleset_steps;
  explicit ErasureCodeOptLrc() :
	  n(0),
	  DEFAULT_N("9"),
	  k(0),
	  DEFAULT_K("4"),
	  r(0),
	  DEFAULT_R("2")
  {
    ruleset_steps.push_back(Step("chooseleaf", "host", 0));
  }
~ErasureCodeOptLrc() override {}
int init(ErasureCodeProfile &profile, std::ostream *ss) override;
unsigned int get_chunk_count() const override {
	  return n;
  }
unsigned int get_data_chunk_count() const override {
	  return k;
  }
unsigned int get_alignment() const;
unsigned int get_chunk_size(unsigned int object_size) const override ;
/*
 * Name: encode_chunks
 *
 * Description:
 * This function is a wrapper for optlrc_encode. 
 *
 * Parameters:
 * want_to_encode -  want_to_read chunk indexes to be decoded
 * encoded - the map we will encode
 *
 * Return:
 * 0 on success
 * -EINVAL on invalid configuration
 *
 * Notes:
 *
 */
int encode_chunks(const std::set<int> &want_to_encode,
      	    std::map<int, bufferlist> *encoded) override;
/*
 * Name: optlrc_decode_local
 *
 * Description:
 * This function performs a XOR operation on surviving chunk in a local group to 
 * reconstruct a lost chunk.
 *
 * Parameters:
 * erased - ID of the erased chunk
 * decoded - map chunk indexes to chunk data
 * group_size - the size of the local group
 * blocksize - the size of a chunk in bytes
 * 
 * Return:
 * 0 on success
 * -EINVAL on invalid configuration
 *
 * Notes:
 *
 */
int optlrc_decode_local(const int erased,
				char **decoded, int group_size, int blocksize);

int parse_nkr(ErasureCodeProfile &profile, std::ostream *ss);
int parse_ruleset(ErasureCodeProfile &profile, std::ostream *ss);

int parse_ruleset_step(std::string description_string,
			 json_spirit::mArray description,
			 std::ostream *ss);
/*
 * Name: decode_chunks
 *
 * Description:
 * This function decodes chunks using optlrc_decode_local.
 * It collects all erased chunk. 
 * Then, per each erased chunk, it constructs an array of surviving chunks from its local group.
 * The array is passed to optlrc_decode_local for the decoding procedure.
 *
 * Parameters:
 * want_to_read - chunk indexes to be decoded
 * chunks - map chunk indexes to chunk data
 * decoded - map chunk indexes to chunk data
 * Return:
 * 0 on success
 * -EINVAL on invalid configuration
 *
 * Notes:
 *
 */
int decode_chunks(const std::set<int> &want_to_read,
			    const std::map<int, bufferlist> &chunks,
			    std::map<int, bufferlist> *decoded) override;

/*
 * Name: optlrc_encode
 *
 * Description:
 * This function encodes data in Optimal-LRC using a pre-calculated generator matrix
 *
 * Parameters:
 * data - the pointers of the data chunks
 * coding - the pointers of the coding chunks
 * blocksize - the size of a chunk in bytes
 *
 * Return:
 * 0 on success
 * -EINVAL on invalid configuration
 *
 * Notes:
 *
 */
	
void optlrc_encode(char **data, char **coding, int blocksize);

int minimum_to_decode(const std::set<int> &want_to_read,
		const std::set<int> &available,
		std::set<int> *minimum) override;

int create_ruleset(const std::string &name,
			CrushWrapper &crush,
			std::ostream *ss) const override;
protected:
  virtual int parse(ErasureCodeProfile &profile, std::ostream *ss);
};

#endif
