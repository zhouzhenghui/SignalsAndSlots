/* 
 * File:   SafeQueueVariant.hpp
 * Author: Barath Kannan
 *
 * Created on June 13, 2016, 11:51 PM
 */

#ifndef SAFEQUEUEVARIANT_HPP
#define SAFEQUEUEVARIANT_HPP

#include <queue>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>

namespace BSignals{ namespace details{
template <class T>
class SafeQueueVariant{
public:
    
    //Default constructed object is returned when forced stop is required
    //Otherwise, constructor parameters can be provided
    template<typename ...Args>
    SafeQueueVariant(Args&& ...args) : shutdownObject(args...), terminateFlag (false), q(), m(), c() {}
    
    ~SafeQueueVariant(){
        stop();
    }
    
    void enqueue(const T &t){
        std::shared_lock<std::shared_timed_mutex> lock(m);
        q.push(t);
        c.notify_one();
    }
    
    T dequeue(void){
        std::unique_lock<std::shared_timed_mutex> lock(m);
        if (terminateFlag) return shutdownObject;
        while (q.empty()){
            c.wait(lock);
            if (terminateFlag) return shutdownObject;
        }
        T val = q.front();
        q.pop();
        return val;
    }

    std::pair<T, bool> wait_for_dequeue(std::chrono::duration<double> timeout){
        std::pair<T, bool> ret;
        ret.second = false;
        std::unique_lock<std::shared_timed_mutex> lock(m);
        if (terminateFlag){
            return ret;
        }
        bool success = (c.wait_for(lock, timeout) == std::cv_status::no_timeout);
        if (!success) return ret;
        ret.first = q.front();
        q.pop();
        ret.second = true;
        return ret;
    }
    
    std::pair<T, bool> nonBlockingDequeue(void){
        std::unique_lock<std::shared_timed_mutex> lock(m);
        std::pair<T, bool> ret;
        if (!q.empty()){
            ret.first = q.front();
            q.pop();
            ret.second = true;
        }
        else{
            ret.second = false;
        }
        
        return ret;
    }
    
    void clear(){
        std::lock_guard<std::shared_timed_mutex> lock(m);
        while (!q.empty()){
            q.pop();
        }
    }
    
    void wakeWaiters(){
        std::unique_lock<std::shared_timed_mutex> lock(m);
        c.notify_all();
    }
    
    void wait(){
        std::unique_lock<std::shared_timed_mutex> lock(m);
        if (terminateFlag) return;
        if (q.empty()){
            c.wait(lock);
        }
    }
    
    bool wait(std::chrono::duration<double> timeout){
        auto start = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::shared_timed_mutex> lock(m);
        if (!q.empty()) return true;
        if (terminateFlag) return false;
        auto now = std::chrono::high_resolution_clock::now();
        return (c.wait_for(lock, timeout - (now - start)) == std::cv_status::no_timeout);
    }
    
    void stop(){
        terminateFlag = true;
        wakeWaiters();
        std::unique_lock<std::shared_timed_mutex> lock(m);
        c.notify_all();
    }
    
    bool isStopped(){
        return terminateFlag;
    }
    
    unsigned int size(){
        std::lock_guard<std::shared_timed_mutex> lock(m);
        return q.size();
    }
    
    bool isEmpty(){
        std::lock_guard<std::shared_timed_mutex> lock(m);
        return q.empty();
    }
    
private:
    T shutdownObject;
    bool terminateFlag;
    std::queue<T> q;
    std::shared_timed_mutex m;
    std::condition_variable_any c;
};
}}
#endif /* SAFEQUEUE_HPP */
