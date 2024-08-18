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

#include <errno.h>
#include <algorithm>
#include <math.h>

#include "ErasureCodeOptLrc.h"
#include "ErasureCodeOptLrc_configs.h"
#include "include/str_map.h"
#include "common/debug.h"
#include "crush/CrushWrapper.h"
#include "osd/osd_types.h"
#include "include/stringify.h"
#include "erasure-code/ErasureCodePlugin.h"
#include "json_spirit/json_spirit_writer.h"
#include "include/ceph_assert.h"

extern "C" {
#include "erasure-code/jerasure/jerasure/include/jerasure.h"
#include "erasure-code/jerasure/jerasure/include/reed_sol.h"
#include "erasure-code/jerasure/jerasure/include/galois.h"
#include "erasure-code/jerasure/jerasure/include/liberation.h"
}

using namespace std;

#define dout_context g_ceph_context
#define dout_subsys ceph_subsys_osd
#undef dout_prefix
#define dout_prefix _prefix(_dout)
#define BASIC_BLOCK_SIZE 67108864
#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

static ostream& _prefix(std::ostream* _dout)
{
  return *_dout << "ErasureCodeOptLrc: ";
}
int ErasureCodeOptLrc::create_rule(const string &name,
					CrushWrapper &crush,
					ostream *ss) const
{
  if (crush.rule_exists(name)) {
    *ss << "rule " << name << " exists";
    return -EEXIST;
  }
  if (!crush.name_exists(ruleset_root)) {
    *ss << "root item " << ruleset_root << " does not exist";
    return -ENOENT;
  }
  int root = crush.get_item_id(ruleset_root);

  int ruleset = 0;
  int rno = 0;
  for (rno = 0; rno < crush.get_max_rules(); rno++) {
    if (!crush.rule_exists(rno) && !crush.rule_exists(rno))
       break;
  }
  ruleset = rno;

  int steps = 4 + ruleset_steps.size();
  int min_rep = 3;
  int max_rep = get_chunk_count();
  int ret;
  ret = crush.add_rule(rno, steps, pg_pool_t::TYPE_ERASURE);
  //ret = crush.add_rule(steps, ruleset, pg_pool_t::TYPE_ERASURE,
	//	  min_rep, max_rep, rno);
  ceph_assert(ret == rno);
  int step = 0;

  ret = crush.set_rule_step(rno, step++, CRUSH_RULE_SET_CHOOSELEAF_TRIES, 5, 0);
  ceph_assert(ret == 0);
  ret = crush.set_rule_step(rno, step++, CRUSH_RULE_SET_CHOOSE_TRIES, 100, 0);
  ceph_assert(ret == 0);
  ret = crush.set_rule_step(rno, step++, CRUSH_RULE_TAKE, root, 0);
  ceph_assert(ret == 0);
  for (vector<Step>::const_iterator i = ruleset_steps.begin();
       i != ruleset_steps.end();
       ++i) {
    int op = i->op == "chooseleaf" ?
      CRUSH_RULE_CHOOSELEAF_INDEP : CRUSH_RULE_CHOOSE_INDEP;
    int type = crush.get_type_id(i->type);
    if (type < 0) {
      *ss << "unknown crush type " << i->type;
      return -EINVAL;
    }
    ret = crush.set_rule_step(rno, step++, op, i->n, type);
    ceph_assert(ret == 0);
  }
  ret = crush.set_rule_step(rno, step++, CRUSH_RULE_EMIT, 0, 0);
  ceph_assert(ret == 0);
  crush.set_rule_name(rno, name);
  return ruleset;
}
unsigned int ErasureCodeOptLrc::get_alignment() const
{
	return k*8*sizeof(int);
}

unsigned int ErasureCodeOptLrc::get_chunk_size(unsigned int object_size) const
{
    unsigned int alignment = get_alignment();
    unsigned int data_chunk_count = k;
    unsigned int chunk_size = (object_size + data_chunk_count - 1) / data_chunk_count;

    unsigned int modulo = chunk_size % alignment;
    if (modulo) {
    dout(20) << __func__ << "get_chunk_size: " << chunk_size
             << " padded to " << chunk_size + alignment - modulo << dendl;
        chunk_size += alignment - modulo;
    }
    return chunk_size;
}

int ErasureCodeOptLrc::init(ErasureCodeProfile& profile, ostream *ss)
{
  int err = 0;
  err = parse_nkr(profile, ss);
  if (err)
    return err;
  err |= parse(profile, ss);
  if (err)
    return err;
  ErasureCode::init(profile, ss);
  return err;
}

int ErasureCodeOptLrc::parse_nkr(ErasureCodeProfile &profile,
			      ostream *ss)
{
  int err = 0;
  err |= to_int("n", profile, &n, DEFAULT_N, ss);
  err |= to_int("k", profile, &k, DEFAULT_K, ss);
  err |= to_int("r", profile, &r, DEFAULT_R, ss);
  if (chunk_mapping.size() > 0 && (int)chunk_mapping.size() != n) {
    *ss << "mapping " << profile.find("mapping")->second
	<< " maps " << chunk_mapping.size() << " chunks instead of"
	<< " the expected " << n << " and will be ignored" << std::endl;
    chunk_mapping.clear();
    err = -EINVAL;
  }
  ErasureCodeProfile::const_iterator parameter;
  string ruleset_locality;
  parameter = profile.find("ruleset-locality");
  if (parameter != profile.end())
    ruleset_locality = parameter->second;
  string ruleset_failure_domain = "host";
  parameter = profile.find("ruleset-failure-domain");
  if (profile.count("mapping") == 0) {
    *ss << "the 'mapping' profile is missing from " << profile;
    return ERROR_LRC_MAPPING;
  }
  string mapstr = profile.find("mapping")->second;
  int ind = 0;
  vector<int> coding_mapping;
  for(std::string::iterator it = mapstr.begin(); it != mapstr.end(); ++it) {
    if (*it == 'D')
      mapping.push_back(ind);
    else
      coding_mapping.push_back(ind);
    ind++;
  }
  mapping.insert(mapping.end(),
  		 coding_mapping.begin(),
  		 coding_mapping.end());

  if (parameter != profile.end())
    ruleset_failure_domain = parameter->second;

  int local_group_count = ceil(n/(r+1));
  if (ruleset_locality != "") {
    ruleset_steps.clear();
    ruleset_steps.push_back(Step("choose", ruleset_locality,
				 local_group_count));
    ruleset_steps.push_back(Step("chooseleaf", ruleset_failure_domain,
				 r + 1));
  } else if (ruleset_failure_domain != "") {
    ruleset_steps.clear();
    ruleset_steps.push_back(Step("chooseleaf", ruleset_failure_domain, 0));
  }
  return err;
}

int ErasureCodeOptLrc::parse(ErasureCodeProfile &profile,
			       ostream *ss)
{
  int err = ErasureCode::parse(profile, ss);
  err |= sanity_check_k_m(k, 1, ss);//Assume m == 1
  if (err)
        return err;
  return parse_ruleset(profile, ss);
}

int ErasureCodeOptLrc::parse_ruleset(ErasureCodeProfile &profile,
				  ostream *ss)
{
  int err = 0;
  err |= to_string("ruleset-root", profile,
		   &ruleset_root,
		   "default", ss);

  if (profile.count("ruleset-steps") != 0) {
    ruleset_steps.clear();
    string str = profile.find("ruleset-steps")->second;
    json_spirit::mArray description;
    try {
      json_spirit::mValue json;
      json_spirit::read_or_throw(str, json);

      if (json.type() != json_spirit::array_type) {
	*ss << "ruleset-steps='" << str
	    << "' must be a JSON array but is of type "
	    << json.type() << " instead" << std::endl;
	return ERROR_LRC_ARRAY;
      }
      description = json.get_array();
    } catch (json_spirit::Error_position &e) {
      *ss << "failed to parse ruleset-steps='" << str << "'"
	  << " at line " << e.line_ << ", column " << e.column_
	  << " : " << e.reason_ << std::endl;
      return ERROR_LRC_PARSE_JSON;
    }

    int position = 0;
    for (vector<json_spirit::mValue>::iterator i = description.begin();
	 i != description.end();
	 ++i, position++) {
      if (i->type() != json_spirit::array_type) {
	stringstream json_string;
	json_spirit::write(*i, json_string);
	*ss << "element of the array "
	    << str << " must be a JSON array but "
	    << json_string.str() << " at position " << position
	    << " is of type " << i->type() << " instead" << std::endl;
	return ERROR_LRC_ARRAY;
      }
      int r = parse_ruleset_step(str, i->get_array(), ss);
      if (r)
	return r;
    }
  }
  return 0;
}

int ErasureCodeOptLrc::parse_ruleset_step(string description_string,
				       json_spirit::mArray description,
				       ostream *ss)
{
  stringstream json_string;
  json_spirit::write(description, json_string);
  string op;
  string type;
  int n = 0;
  int position = 0;
  for (vector<json_spirit::mValue>::iterator i = description.begin();
       i != description.end();
       ++i, position++) {
    if ((position == 0 || position == 1) &&
	i->type() != json_spirit::str_type) {
      *ss << "element " << position << " of the array "
	  << json_string.str() << " found in " << description_string
	  << " must be a JSON string but is of type "
	  << i->type() << " instead" << std::endl;
      return position == 0 ? ERROR_LRC_RULESET_OP : ERROR_LRC_RULESET_TYPE;
    }
    if (position == 2 && i->type() != json_spirit::int_type) {
      *ss << "element " << position << " of the array "
	  << json_string.str() << " found in " << description_string
	  << " must be a JSON int but is of type "
	  << i->type() << " instead" << std::endl;
      return ERROR_LRC_RULESET_N;
    }

    if (position == 0)
      op = i->get_str();
    if (position == 1)
      type = i->get_str();
    if (position == 2)
      n = i->get_int();
  }
  ruleset_steps.push_back(Step(op, type, n));
  return 0;
}
int ErasureCodeOptLrc::encode_chunks(const set<int> &want_to_encode,
				       map<int, bufferlist> *encoded)
{
	char *chunks[n];
        int ind=0;
        for (auto i = mapping.begin(); i != mapping.end(); ++i) {
                chunks[ind] = (*encoded)[*i].c_str();
                ind++;
        }
	// send mapping to data and coding chunks for the encode process
        optlrc_encode(&chunks[0], &chunks[k], (*encoded->begin()).second.length());

	return 0;
}

void ErasureCodeOptLrc::optlrc_encode(char **data, char **coding, int blocksize)
{
	OptLRC_Configs optlrc_configs;
	// Fetch pre-calculated generator matrix for n,k,r
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	int *matrix = talloc(int, (n-k)*k);
	if (matrix == NULL) {
	  free(matrix);
	  std::exit(0);
	}

        int ind=0;
	// adjust generator matrix format for jerasure_matrix_encode
        for (auto i = mapping.begin(); i != mapping.end(); ++i) {
        	if (ind<k){
        		ind++;
        		continue;
        	}
	        for (int j=0 ; j<k; j++)
	        	matrix[k*(ind-k)+j]= pOptLRC_G->optlrc_encode[*i][j];
	        ind++;
	}
	// encode data
	jerasure_matrix_encode(k, n-k, 8, matrix, data, coding, blocksize);

	free(matrix);
}

int ErasureCodeOptLrc::_minimum_to_decode(const set<int> &want_to_read,
				      const set<int> &available_chunks,
				      set<int> *minimum)
{
  dout(1) << __func__ << " want_to_read " << want_to_read
	   << " available_chunks " << available_chunks << dendl;

        set<int> erasures_total;
        OptLRC_Configs optlrc_configs;
        POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
        set<int> failed_groups;
        set<int> erasures_want;
        for (unsigned int i = 0; i < get_chunk_count(); ++i) {
          if (available_chunks.count(i) == 0) {
            erasures_total.insert(i);
	  if (want_to_read.count(i) != 0)
	    erasures_want.insert(i);
          }
        }
    //
    // Case 1:
    //
    // When no chunk is missing there is no need to read more than what
    // is wanted.
    //
    if (erasures_want.empty()) {
      *minimum = want_to_read;
      dout(1) << __func__ << " minimum == want_to_read == "
	       << want_to_read << dendl;
      return 0;
    }

    //check to which group the failed node belongs
    for (set<int>::iterator it = erasures_want.begin(); it != erasures_want.end(); ++it) {
            if (failed_groups.count(*it /(r+1)) == 1 ) {
                        dout(20) << __func__ << " twice failed in the same group "
                                 << *it /(r+1) << dendl;
                        return -EIO;
        }
        failed_groups.insert( *it /(r+1));

    //
    // Case 2:
    //
    // We have one erasure in local group
    // We assume we cannot have more than one erasure in a local group
    //
    for (set<int>::iterator it1 = failed_groups.begin(); it1 != failed_groups.end(); ++it1){
        for (set<int>::iterator it2 = available_chunks.begin(); it2 != available_chunks.end(); ++it2){
            //if chunk is available and in required group - add to minimum
		if ((*it2 / (r+1)) == *it1 )
                	minimum->insert(*it2);
        }
    }
    }

    minimum->insert(want_to_read.begin(), want_to_read.end());
    for (set<int>::const_iterator i = erasures_total.begin();
	i != erasures_total.end();
	++i) {
        if (minimum->count(*i))
            minimum->erase(*i);
    }
    dout(1) << __func__ << " minimum = " << *minimum << dendl;
    dout(0) << __func__ << " debug:want_to_read = " << want_to_read.size() << " minimum = " << minimum->size() << dendl;
    return 0;

}

int ErasureCodeOptLrc::optlrc_decode_local(const int erased, char **decoded, int group_size, int blocksize) {

	int init=0;
	OptLRC_Configs optlrc_configs;
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	int loc_erased = erased % (r+1);
	char *dst = decoded[loc_erased];
	// go over all chunks in local group
	for (int i=0;i<group_size;i++){
		// if not the erased chunk then we have it
		if (i!=loc_erased){
			char *src = decoded[i];
			// XOR surviving chunks to decode erased chunk
			if (init==0) {
				memcpy(dst, src, blocksize);
				init=1;
			}
			else
                                galois_region_xor(src,decoded[loc_erased],blocksize);
		}
	}
	return 0;
}

int ErasureCodeOptLrc::decode_chunks(const set<int> &want_to_read,
				       const map<int, bufferlist> &chunks,
				       map<int, bufferlist> *decoded)
{
	int blocksize = (*chunks.begin()).second.length();
	char *local[r+1];
	int failed_group;
	OptLRC_Configs optlrc_configs;
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	int m=0;
        set<int> erasures;
	// Collect erased chunks
        for (unsigned int i = 0; i < get_chunk_count(); ++i) {
          if (chunks.count(i) == 0){
              if (want_to_read.count(i) != 0)
                      erasures.insert(i);

          }
        }
        int erasures_init=erasures.size();
	// Handle erasures one by one
        for (set<int>::iterator it = erasures.begin(); it != erasures.end(); ++it) {
                int group_size = 0;
	        int erased = *it;
	        //calculate failed group from real location
	        failed_group = erased /(r+1);
                for (int i=0;i<n;i++){
                	//find rest of failed group from real location
                	if ((i / (r+1)) == failed_group) {
                                group_size++;
                                m= i % (r+1);
                		//collect local group
                		local[m] = (*decoded)[i].c_str();
                	}
                }
		// Proceed to decoding itself
                optlrc_decode_local(erased, local, group_size, blocksize);
                //reset in case more than 1 erasure
                m=0;
                erasures_init--;
        }

        if (erasures_init > 0) {
    derr << __func__ << " want to read " << want_to_read
	 << " end up being unable to read " << erasures_init << dendl;
    return -EIO;
        } else
	        return 0;
}
