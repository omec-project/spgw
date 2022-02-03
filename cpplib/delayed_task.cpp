// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#include "delayed_task.h"
#include <mutex>
#include "cp_log.h"
#include <unistd.h>

void delayTask::add_delayed_free_memory_task(void *trans)
{
    const std::lock_guard<std::mutex> lock(free_mtx);
    delayed_memory_free_m.push(trans);
}

void* delayTask::delete_delayed_free_memory_task(int *size)
{
    const std::lock_guard<std::mutex> lock(free_mtx);
    delayTask *instance = delayTask::Instance();
    void *temp = NULL;
    *size = 0;
    if(instance->delayed_memory_free_m.size() == 0) {
       return temp; 
    }
    temp = instance->delayed_memory_free_m.front(); 
    instance->delayed_memory_free_m.pop();
    *size = instance->delayed_memory_free_m.size();
    return temp;
}
