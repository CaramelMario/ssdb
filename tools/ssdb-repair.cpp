/*
Copyright (c) 2012-2015 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
// use rocksdb instead of leveldb
#define leveldb rocksdb
#include "include.h"

#include <string>
#include <vector>

#include <rocksdb/db.h>
#include <rocksdb/env.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/iterator.h>

#include "util/log.h"
#include "util/file.h"
#include "util/strings.h"

void welcome(){
	printf("ssdb-repair - SSDB repair tool\n");
	printf("Copyright (c) 2013-2015 ssdb.io\n");
	printf("\n");
}

void usage(int argc, char **argv){
	printf("Usage:\n");
	printf("    %s leveldb_folder\n", argv[0]);
	printf("\n");
}

int main(int argc, char **argv){
	welcome();

	set_log_level(Logger::LEVEL_MIN);

	if(argc <= 1){
		usage(argc, argv);
		return 0;
	}
	std::string leveldb_folder(argv[1]);

	if(!file_exists(leveldb_folder.c_str())){
		printf("leveldb_folder[%s] not exists!\n", leveldb_folder.c_str());
		return 0;
	}
	
	leveldb::Status status;

	std::shared_ptr<leveldb::Logger> p_logger;

	status = leveldb::Env::Default()->NewLogger("repair.log", &p_logger);
	// NewLogger return a Logger with p_logger

	if(!status.ok()){
		printf("logger error!\n");
		return 0;
	}
	printf("writing repair log into: repair.log\n");

	leveldb::Options options;
	options.info_log = p_logger;
	status = leveldb::RepairDB(leveldb_folder.c_str(), options);
	if(!status.ok()){
		printf("repair leveldb: %s error!\n", leveldb_folder.c_str());
		return 0;
	}
	
	printf("leveldb repaired.\n");

	return 0;
}
