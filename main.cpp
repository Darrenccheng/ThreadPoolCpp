#include <stdio.h>
#include "threadpool.h"
#include "threadpool.cpp"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void testFunc(void* arg) {
    int num = *(int*)arg;
    printf("线程id为为：%ld， tid为：%ld\n", pthread_self(), num);
    sleep(1);
}

int main() {
    threadpool<int> pool(3, 10);
    for(int i = 0; i < 100; i++) {
        int* num = new int(1 + 100);

        pool.addTask(Task<int>(testFunc, num));
    }

    sleep(30);
    return 0;
}
