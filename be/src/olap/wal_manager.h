// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once
#include <gen_cpp/PaloInternalService_types.h>

#include <condition_variable>
#include <memory>

#include "common/config.h"
#include "gen_cpp/FrontendService.h"
#include "gen_cpp/FrontendService_types.h"
#include "gen_cpp/HeartbeatService_types.h"
#include "olap/wal_reader.h"
#include "olap/wal_table.h"
#include "olap/wal_writer.h"
#include "runtime/exec_env.h"
#include "runtime/stream_load/stream_load_context.h"
#include "util/thread.h"

namespace doris {
class WalManager {
    ENABLE_FACTORY_CREATOR(WalManager);

public:
    enum WAL_STATUS {
        PREPARE = 0,
        REPLAY,
        CREATE,
    };

public:
    WalManager(ExecEnv* exec_env, const std::string& wal_dir);
    ~WalManager();
    Status delete_wal(int64_t wal_id);
    Status init();
    Status scan_wals(const std::string& wal_path);
    Status replay();
    Status create_wal_reader(const std::string& wal_path, std::shared_ptr<WalReader>& wal_reader);
    Status create_wal_writer(int64_t wal_id, std::shared_ptr<WalWriter>& wal_writer);
    Status scan();
    size_t get_wal_table_size(const std::string& table_id);
    Status add_recover_wal(const std::string& db_id, const std::string& table_id,
                           std::vector<std::string> wals);
    Status add_wal_path(int64_t db_id, int64_t table_id, int64_t wal_id, const std::string& label);
    Status get_wal_path(int64_t wal_id, std::string& wal_path);
    Status get_wal_status_queue_size(const PGetWalQueueSizeRequest* request,
                                     PGetWalQueueSizeResponse* response);
    Status get_all_wal_status_queue_size(const PGetWalQueueSizeRequest* request,
                                         PGetWalQueueSizeResponse* response);
    void add_wal_status_queue(int64_t table_id, int64_t wal_id, WAL_STATUS wal_status);
    Status erase_wal_status_queue(int64_t table_id, int64_t wal_id);
    void print_wal_status_queue();
    void stop();
    bool is_running();
    void stop_relay_wal();
    void add_wal_column_index(int64_t wal_id, std::vector<size_t>& column_index);
    void erase_wal_column_index(int64_t wal_id);
    Status get_wal_column_index(int64_t wal_id, std::vector<size_t>& column_index);

private:
    ExecEnv* _exec_env = nullptr;
    std::shared_mutex _lock;
    scoped_refptr<Thread> _replay_thread;
    CountDownLatch _stop_background_threads_latch;
    std::map<std::string, std::shared_ptr<WalTable>> _table_map;
    std::vector<std::string> _wal_dirs;
    std::shared_mutex _wal_lock;
    std::shared_mutex _wal_status_lock;
    std::unordered_map<int64_t, std::string> _wal_path_map;
    std::unordered_map<int64_t, std::shared_ptr<WalWriter>> _wal_id_to_writer_map;
    std::shared_ptr<std::atomic_size_t> _all_wal_disk_bytes;
    std::unordered_map<int64_t, std::unordered_map<int64_t, WAL_STATUS>> _wal_status_queues;
    std::atomic<bool> _stop;
    std::unordered_map<int64_t, std::vector<size_t>&> _wal_column_id_map;
    std::shared_ptr<std::condition_variable> _cv;
};
} // namespace doris