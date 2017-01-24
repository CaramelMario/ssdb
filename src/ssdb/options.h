/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#ifndef SSDB_OPTION_H_
#define SSDB_OPTION_H_

#include "../util/config.h"

class Options
{
public:
	Options();
	~Options(){}
	
	void load(const Config &conf);

	size_t cache_size;
	size_t max_open_files;
	size_t write_buffer_size;
	size_t block_size;
	int compaction_speed;
	std::string compression;
	bool binlog;
	size_t binlog_capacity;

    //tarark zip table options
    int index_nest_level;
    int check_sum_level;
    std::string entropy_algo;
    int terark_zip_min_level;
    bool use_suffix_array_local_match;
    bool warm_up_index_on_open;
    bool warm_up_value_on_open;
    float estimate_compression_ratio;
    double sample_ratio;
    std::string local_temp_dir;
    std::string index_type;
    size_t soft_zip_working_mem_limit;
    size_t hard_zip_working_mem_limit;
    size_t small_task_memory;
    double index_cache_ratio;
};

#endif
