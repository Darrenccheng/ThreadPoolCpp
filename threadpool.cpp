//
// Created by pengf on 2024/11/25.
//
#pragma once
#include "threadpool.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace std;

// 创建线程池的函数（初始化）
template<typename T>
threadpool<T>::threadpool(int min, int max) {
    do {
        // 初始化任务队列
        taskQ_ = new TaskQueue<T> ;
        if(taskQ_ == nullptr){
            cout << "创建线程池失败！" << endl;
            break;
        }

        threadIDs_ = new pthread_t[max];
        if(threadIDs_ == nullptr) {
            cout << "创建线程池失败！" << endl;
            break;
        }
        memset(threadIDs_, 0, sizeof(pthread_t) * max); // 初始化线程池中的管理线程队列

        minNum_ = min;
        maxNum_ = max;
        busyNum_ = 0;
        liveNum_ = min;
        exitNum_ = 0;

        // 初始化锁和条件变量
        if (pthread_mutex_init(&mutexPool_, NULL) ||
            pthread_cond_init(&notEmpty_, NULL) ) {
            cout << "创建线程池失败！" << endl;
            break;
        }

        shutdown_ = false;

        // 初始化线程（创建线程）
        pthread_create(&managerID_, NULL, manager, this);   // 此处，传入的函数指针必须为以及知道的地址
                                                                                        // ，要么为静态类成员函数，要么为外部的函数
        for(int i = 0; i < min; i++) {
            pthread_create(&threadIDs_[i], NULL, worker, this);
        }
        cout << "创建线程池成功" << endl;
        return;
    }while(0);

    if(threadIDs_) delete[] threadIDs_;
    if(taskQ_) delete taskQ_;
}

// 销毁线程池的函数
template<typename T>
threadpool<T>::~threadpool() {
    shutdown_ = true; // 关闭线程池
    // 阻塞回收管理者线程
    pthread_join(managerID_, NULL);

    // 唤醒消费者线程
    for(int i = 0; i < liveNum_; i++) {
        pthread_cond_signal(&notEmpty_);
    }

    // 释放堆内存
    if(taskQ_) {
        delete taskQ_;
    }
    if(threadIDs_) {
        delete[] threadIDs_;
    }

    // 释放信号量和锁
    pthread_mutex_destroy(&mutexPool_);
    pthread_cond_destroy(&notEmpty_);
}

// 添加任务的函数
template<typename T>
void threadpool<T>::addTask(Task<T> task) {
    if (shutdown_) {
        return;
    }

    // 添加任务
    taskQ_->addTask(std::move(task));
}

// 消费者任务函数
template<typename T>
void* threadpool<T>::worker(void* arg) {
    threadpool* pool = static_cast<threadpool*>(arg);

    while(1) {
        // 从线程池里拿数据
        pthread_mutex_lock(&pool->mutexPool_);
        // 线程池任务数量为0且线程池开着的
        while(pool->taskQ_->taskNumber() == 0 && !pool->shutdown_) {
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty_, &pool->mutexPool_);
            // 线程自杀
            if (pool->exitNum_ > 0) {
                pool->exitNum_--;
                if(pool->liveNum_ > pool->minNum_) {
                    pool->liveNum_--;
                    pthread_mutex_unlock(&pool->mutexPool_);
                    pool->threadExit();
                }
            }
        }

        if(pool->shutdown_) {
            pthread_mutex_unlock(&pool->mutexPool_);
            pool->threadExit();
        }

        // 从任务队列中取出数据
        cout << "从任务队列取出任务" << endl;
        Task<T> task = pool->taskQ_->takeTask();
        pool->busyNum_++;
        pthread_mutex_unlock(&pool->mutexPool_);

        // 开始执行任务函数
        cout << "开始执行任务函数" << endl;
        task.function(task.arg);    // 执行函数
        delete task.arg;
        task.arg = nullptr;

        cout << "任务函数执行完毕" << endl;
        pthread_mutex_lock(&pool->mutexPool_);
        pool->busyNum_--;
        pthread_mutex_unlock(&pool->mutexPool_);
    }
    return nullptr;
}

// 管理者线程任务函数
template<typename T>
void* threadpool<T>::manager(void* arg) {
    threadpool* pool = static_cast<threadpool*>(arg);

    while (!pool->shutdown_) {
        // 看是需要创建线程
        // 每间隔3秒钟检测一次
        sleep(3);

        // 取出线程池中任务数量和线程数量、忙着的线程数量
        pthread_mutex_lock(&pool->mutexPool_);
        int liveNum = pool->liveNum_;
        int busyNum = pool->busyNum_;
        pthread_mutex_unlock(&pool->mutexPool_);
        const int NUMEBER = 2;
        // 满足一定条件时，才可以添加线程，条件可以自己设置
        if(pool->taskQ_->taskNumber() > liveNum && busyNum < pool->maxNum_) {
            int counter = 0;    //  添加线程的次数
            pthread_mutex_lock(&pool->mutexPool_);
            for (int i = 0; i < pool->maxNum_ && counter < NUMEBER && pool->liveNum_ < pool->maxNum_; i++) {
                if (pool->threadIDs_[i] == 0) {
                    pthread_create(&pool->threadIDs_[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum_++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool_);

        }

        // 看是否需要销毁线程,也是需要一定的条件
        if (busyNum * 2 < liveNum && liveNum > pool->minNum_) {
            pthread_mutex_lock(&pool->mutexPool_);
            pool->exitNum_ = NUMEBER;
            pthread_mutex_unlock(&pool->mutexPool_);
            // 唤醒因为任务数量为0而阻塞的线程，让这些线程自杀
            for(int i = 0; i < NUMEBER; i++) {
                pthread_cond_signal(&pool->notEmpty_);
            }
        }
    }
    return nullptr;
}

template<typename T>
void threadpool<T>::threadExit() {
    pthread_t tid = pthread_self();
    for(int i = 0; i < maxNum_; i++) {
        if (tid == threadIDs_[i]) {
            threadIDs_[i] = 0;
            break;
        }
    }
    cout << "=====================线程 " << tid << "退出=====================" << endl;

    pthread_exit(NULL);
}