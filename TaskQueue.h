//
// Created by pengf on 2024/11/25.
//
// 任务结构体
#pragma once
#include <queue>
#include <pthread.h>

using callback = void(*)(void* arg);

template <typename T>
struct Task {
    Task() {
        function = nullptr;
        arg = nullptr;
    }

    Task(callback f, void* arg) {
        this->arg = static_cast<T*>(arg);
        function = f;

    }
    callback function;    // 任务函数指针
    T* arg;  // 任务函数的参数
};

template <typename T>
class TaskQueue {
public:
    TaskQueue();
    ~TaskQueue();

    // 添加任务
    void addTask(Task<T> task);
    void addTask(callback f, void* arg);
    // 取出一个任务
    Task<T> takeTask();

    // 获取任务个数
    inline int taskNumber() {
        return taskQ_.size();
    }
private:
    std::queue<Task<T>> taskQ_;    // 任务队列
    pthread_mutex_t mutex_;     // 任务队列的锁
};