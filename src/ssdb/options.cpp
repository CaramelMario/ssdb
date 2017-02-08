/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#include "options.h"
#include "../util/strings.h"

#ifdef NDEBUG
	static const int LOG_QUEUE_SIZE  = 20 * 1000 * 1000;
#else
	static const int LOG_QUEUE_SIZE  = 10000;
#endif

Options::Options(): terark_enable(true),
                    index_nest_level(3),
                    check_sum_level(1),
                    entropy_algo("none"),
                    terark_zip_min_level(0),
                    use_suffix_array_local_match(false),
                    warm_up_index_on_open(true),
                    warm_up_value_on_open(false),
                    estimate_compression_ratio(0.2),
                    sample_ratio(0.03),
                    local_temp_dir("/tmp"),
                    index_type("IL256"),
                    soft_zip_working_mem_limit(16ull << 30),
                    hard_zip_working_mem_limit(32ull << 30),
                    small_task_memory(1200 << 20),
                    index_cache_ratio(0)
{
	Config c;
	this->load(c);
}

void Options::load(const Config &conf){
	cache_size = (size_t)conf.get_num("rocksdb.cache_size");
	max_open_files = (size_t)conf.get_num("rocksdb.max_open_files");
	write_buffer_size = (size_t)conf.get_num("rocksdb.write_buffer_size");
	block_size = (size_t)conf.get_num("rocksdb.block_size");
	compaction_speed = conf.get_num("rocksdb.compaction_speed");
	compression = conf.get_str("rocksdb.compression");
	std::string binlog = conf.get_str("replication.binlog");
	binlog_capacity = (size_t)conf.get_num("replication.binlog.capacity");

  //terark zip table
    std::string terark_enable = conf.get_str("terark_zip_table.terark_enable");
  index_nest_level = conf.get_num("terark_zip_table.index_nest_level");
  check_sum_level = conf.get_num("terark_zip_table.check_sum_level");
  entropy_algo = conf.get_str("terark_zip_table.entropy_algo");
  terark_zip_min_level = conf.get_num("terark_zip_table.terark_zip_min_level");
  std::string use_suffix_array_local_match = conf.get_str("terark_zip_table.use_suffix_array_local_match");
  std::string warm_up_index_on_open = conf.get_str("terark_zip_table.warm_up_index_on_open");
  std::string warm_up_value_on_open = conf.get_str("terark_zip_table.warm_up_value_on_open");
  estimate_compression_ratio = conf.get_double_num("terark_zip_table.estimate_compression_ratio");
  sample_ratio = conf.get_double_num("terark_zip_table.sample_ratio");
  local_temp_dir = conf.get_str("terark_zip_table.local_temp_dir");
  index_type = conf.get_str("terark_zip_table.index_type");
  soft_zip_working_mem_limit = conf.get_size_t("terark_zip_table.soft_zip_working_mem_limit");
  hard_zip_working_mem_limit = conf.get_size_t("terark_zip_table.hard_zip_working_mem_limit");
  small_task_memory = conf.get_size_t("terark_zip_table.small_task_memory");
  index_cache_ratio = conf.get_double_num("terark_zip_table.index_cache_ratio");

	strtolower(&compression);
	if(compression != "no"){
		compression = "yes";
	}
	strtolower(&binlog);
	if(binlog != "yes"){
		this->binlog = false;
	}else{
		this->binlog = true;
	}
	if(binlog_capacity <= 0){
		binlog_capacity = LOG_QUEUE_SIZE;
	}

	if(cache_size <= 0){
		cache_size = 16;
	}
	if(write_buffer_size <= 0){
		write_buffer_size = 16;
	}
	if(block_size <= 0){
		block_size = 16;
	}
	if(max_open_files <= 0){
		max_open_files = cache_size / 1024 * 300;
		if(max_open_files < 500){
			max_open_files = 500;
		}
		if(max_open_files > 1000){
			max_open_files = 1000;
		}
	}

    strtolower(&terark_enable);
    if(terark_enable == "no"){
        this->terark_enable = false;
    }else{
        this->terark_enable = true;
    }
  if(index_nest_level < 0){
      index_nest_level = 3;
  }
  if(check_sum_level < 0){
      check_sum_level = 1;
  }
  if(terark_zip_min_level < 0){
      terark_zip_min_level = 0;
  }
  strtolower(&use_suffix_array_local_match);
  if(use_suffix_array_local_match == "no"){
      this->use_suffix_array_local_match = false;
  }else{
      this->use_suffix_array_local_match = true;
  }
  strtolower(&warm_up_index_on_open);
  if(warm_up_index_on_open == "yes"){
      this->warm_up_index_on_open = true;
  }else{
      this->warm_up_index_on_open = false;
  }
  strtolower(&warm_up_value_on_open);
  if(warm_up_value_on_open == "no"){
      this->warm_up_value_on_open = false;
  }else{
      this->warm_up_value_on_open = true;
  }
  if(local_temp_dir == ""){
      local_temp_dir = "/tmp";
  }
  if(index_type == ""){
      index_type = "IL_256";
  }
  if(estimate_compression_ratio < 0){
      estimate_compression_ratio = 0.2;
  }
  if(sample_ratio < 0){
      sample_ratio = 0.03;
  }
  if(index_cache_ratio < 0){
      index_cache_ratio = 0;
  }
}
