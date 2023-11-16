/*

    成员函数	        说明
    store	        原子地以非原子对象替换原子对象的值
    load	        原子地获得原子对象的值
*/
/*    特化成员函数:
    特化成员函数	                            说明
    fetch_add	        原子地将参数加到存储于原子对象的值，并返回先前保有的值
    fetch_sub	        原子地进行参数和原子对象的值的逐位与，并获得先前保有的值
    fetch_or	        原子地进行参数和原子对象的值的逐位或，并获得先前保有的值
    fetch_xor	        原子地进行参数和原子对象的值的逐位异或，并获得先前保有的值
    operator++	        令原子值增加一
    operator++(int)	    令原子值增加一
    operator–	        令原子值减少一
    operator–(int)	    令原子值减少一
*/
/*
    typedef enum memory_order {
        memory_order_relaxed,宽松操作，没有同步或顺序制约，仅原子性
        memory_order_consume,
        memory_order_acquire,本线程中，所有后续的读操作必须在本条原子操作完成后执行
        memory_order_release,本线程中，所有之前的写操作完成后才能执行本条原子操作
        memory_order_acq_rel,同时包含acquire和release
        memory_order_seq_cst全部存取都按照顺序执行
    } memory_order;
*/
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<int> sharedCount{0};

void incrementCounter(int id)
{
    for (int i = 0; i < 5; ++i)
    {
        // 原子操作：增加共享计数器的值
        
        sharedCount.fetch_add(1, std::memory_order_release);
        // 模拟其他非原子操作
        std::this_thread::sleep_for(std::chrono::microseconds(10));

        // 打印每个线程的增加结果
        std::cout << "Thread " << id << ": Count = " << sharedCount.load(std::memory_order_seq_cst) << std::endl;
    }
}

int main(int argc, char **argv)
{
    const int numThreads = 3;

    std::vector<std::thread> threads;

    // 启动多个线程，每个线程调用incrementCounter函数
    for (int i = 0; i < numThreads; ++i)
    {
        threads.push_back(std::thread(incrementCounter, i));
    }
    for (auto &th : threads)
    {
        th.join();
    }
    return 0;
}