// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0


#ifndef __DELAYED_TASK_H__
#define __DELAYED_TASK_H__

#include <iostream>
#include <queue>
#include <mutex>
#include <thread>

class delayTask
{
    public:
        void add_delayed_free_memory_task(void *trans);
        void* delete_delayed_free_memory_task(int *size);
        static delayTask* Instance() {
            static delayTask inst;
            return &inst;
        }
        int num_elements_free_queue() {
            return delayed_memory_free_m.size(); 
        }
    private:
        delayTask() {}
    private:
        std::mutex free_mtx;
        std::queue<void *> delayed_memory_free_m;
        
};


#endif
