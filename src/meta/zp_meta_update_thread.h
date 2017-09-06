#ifndef ZP_META_UPDATE_THREAD_H
#define ZP_META_UPDATE_THREAD_H
#include <deque>
#include <string>
#include <unordered_map>
#include <glog/logging.h>
#include "pink/include/bg_thread.h"
#include "slash/include/slash_string.h"
#include "slash/include/slash_mutex.h"
#include "slash/include/slash_status.h"
#include "include/zp_meta.pb.h"
#include "src/meta/zp_meta_info_store.h"

enum ZPMetaUpdateOP : unsigned int {
  kOpUpNode,  // ip_port
  kOpDownNode,  // ip_port
  kOpAddTable,  // table parition num
  kOpRemoveTable,
  kOpAddSlave,  // ip_port table partition
  kOpRemoveSlave,  // ip_port table partition
  kOpRemoveDup,  // ip_port table partition
  kOpSetMaster,  // ip_port table partition
  kOpSetStuck  //table partition
};

typedef void (*update_t) (bool complete);

struct UpdateTask {
  ZPMetaUpdateOP op;
  std::string ip_port;
  std::string table;
  int partition;  // or partiiton num for kOpAddTable
  update_t callback;

  UpdateTask(ZPMetaUpdateOP o, const std::string& ip,
      const std::string&t, int p, update_t c = NULL)
    : op(o), ip_port(ip), table(t), partition(p),
    callback(c) {
    }

  UpdateTask(ZPMetaUpdateOP o, const std::string& ip,
      update_t c = NULL)
    : op(o), ip_port(ip),
    callback(c) { 
    }
};

typedef std::deque<UpdateTask> ZPMetaUpdateTaskDeque;

class ZPMetaUpdateThread {
public:
  explicit ZPMetaUpdateThread(ZPMetaInfoStore* is);
  ~ZPMetaUpdateThread();

  Status PendingUpdate(const UpdateTask& task);

private:
  std::atomic<bool> is_stuck_;
  pink::BGThread* worker_;
  slash::Mutex task_mutex_;
  ZPMetaUpdateTaskDeque task_deque_;
  ZPMetaInfoStore* info_store_;
  static void UpdateFunc(void *p);
  Status ApplyUpdates(ZPMetaUpdateTaskDeque& task_deque);

};

#endif
