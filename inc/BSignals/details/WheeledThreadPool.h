
/* 
 * File:   WheeledThreadPool.h
 * Author: Barath Kannan
 *
 * Created on 13 June 2016, 3:36 PM
 */

#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include "BSignals/details/SafeQueue.hpp"
#include "BSignals/details/SafeQueueVariant.hpp"
#include "BSignals/details/MPMCQueue.h"
#include "BSignals/details/ProducerConsumerQueue.h"
#include "BSignals/details/blockingconcurrentqueue.h"
#include "BSignals/details/Wheel.h"

#include <iostream>
#include "BSignals/details/BasicTimer.h"
using std::cout;
using std::endl;

#ifndef WHEELEDTHREADPOOL_H
#define WHEELEDTHREADPOOL_H

namespace BSignals{ namespace details{

class WheeledThreadPool {
public:
    
//    template <typename... Args>
//    static void run(const std::function<void(Args...)> &task, const Args &... p){
//        threadPooledFunctions.getSpoke().enqueue([task, p...](){task(p...);});
//    }
    
    static void run(const std::function<void()> &task){
        threadPooledFunctions.getSpoke().enqueue(task);
    }
    
    //only invoke start up if a thread pooled slot has been connected
    static void startup();
    
private:
    static void queueListener(uint32_t index);
    static class _init {
    public:
        ~_init(); //destructor
    } _initializer;
    
    static const uint32_t nThreads{4};
    static std::mutex tpLock;
    static bool isStarted;
    static BSignals::details::Wheel<moodycamel::BlockingConcurrentQueue<std::function<void()>>, nThreads> threadPooledFunctions;
    static std::vector<std::thread> queueMonitors;
};
}}


#endif /* WHEELEDTHREADPOOL_H */

