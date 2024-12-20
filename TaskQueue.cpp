//
// Created by pengf on 2024/11/25.
//
#include "TaskQueue.h"

template<typename T>
TaskQueue<T>::TaskQueue() {
    pthread_mutex_init(&mutex_, NULL);
}

template<typename T>
TaskQueue<T>::~TaskQueue() {
    pthread_mutex_destroy(&mutex_);
}

// 添加任务
template<typename T>
void TaskQueue<T>::addTask(Task<T> task) {
    pthread_mutex_lock(&mutex_);
    taskQ_.push(std::move(task));
    pthread_mutex_unlock(&mutex_);
}

template<typename T>
void TaskQueue<T>::addTask(callback f, void* arg) {
    pthread_mutex_lock(&mutex_);
    Task<T> t{f, arg};
    taskQ_.push(t);
    pthread_mutex_unlock(&mutex_);
}

template<typename T>
Task<T> TaskQueue<T>::takeTask() {
    Task<T> t;
    pthread_mutex_lock(&mutex_);
    if (!taskQ_.empty()) {
        t = taskQ_.front();
        taskQ_.pop();
    }
    pthread_mutex_unlock(&mutex_);
    return t;
}