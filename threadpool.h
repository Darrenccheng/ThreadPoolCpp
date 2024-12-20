//
// Created by pengf on 2024/11/25.
//
#pragma once
#include "TaskQueue.h"
#include "TaskQueue.cpp"

template<typename T>
class threadpool {
public:
    // 创建线程池的函数（初始化）
    threadpool(int min, int max);

    // 销毁线程池的函数
    ~threadpool();

    // 添加任务的函数
    void addTask(Task<T> task);

    // 获取线程池中的线程个数
    inline int getBusyNumber() {
        int res;
        pthread_mutex_lock(&mutexPool_);
        res = busyNum_;
        pthread_mutex_unlock(&mutexPool_);
        return res;
    }

    // 获取线程池中活着的线程个数
    inline int getAliveNumber() {
        int res;
        pthread_mutex_lock(&mutexPool_);
        res = liveNum_;
        pthread_mutex_unlock(&mutexPool_);
        return res;
    }

private:
    // 消费者任务函数
    static void* worker(void* arg);

    // 管理者线程任务函数
    static void* manager(void* arg);

    void threadExit();

private:
    TaskQueue<T>* taskQ_;   // 任务队列

    pthread_t managerID_;    // 管理者线程ID
    pthread_t *threadIDs_;   // 工作的线程ID
    int minNum_;             // 最小线程数---》需要用户传递
    int maxNum_;             // 最大线程数---》需要用户传递
    int busyNum_;            // 忙的线程个数
    int liveNum_;            // 存活的线程个数
    int exitNum_;            // 要销毁的线程个数
    pthread_mutex_t mutexPool_;  // 锁整个线程池

    bool shutdown_;   // 为1表示要将线程池销毁

    pthread_cond_t notEmpty_;    // 线程池是不是空的
};


