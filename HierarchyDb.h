//
// Created by victor on 27/10/17.
//

#ifndef RESTDBXX_HIERARCHYDB_H
#define RESTDBXX_HIERARCHYDB_H
#include <rocksdb/utilities/optimistic_transaction_db.h>

using std::string;
using std::vector;
using std::shared_ptr;
using std::map;
using rocksdb::DB;

class  Node {
 public:
  virtual string id() const=0 ;
  virtual string data() const =0;
  virtual vector<shared_ptr<Node>> GetChildren()const =0;
  virtual shared_ptr<Node > GetParent() const=0;
};


class HierarchyDb :public rocksdb::OptimisticTransactionDB {
 public:
  HierarchyDb(DB *db);
 public:


};

#endif //RESTDBXX_HIERARCHYDB_H
