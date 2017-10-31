//
// Created by victor on 27/10/17.
//

#include "HierarchyDb.h"

using rocksdb::DB;

HierarchyDb::HierarchyDb(DB *db) : rocksdb::OptimisticTransactionDB(db) {}
