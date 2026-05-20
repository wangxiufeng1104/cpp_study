#include <iostream>
#include <new>
#include <type_traits>

// ============================================================================
// placement new — 在已有内存上构造对象
// ============================================================================
// iceoryx 在共享内存上分配原始字节，然后用 placement new 在指定地址构造 C++ 对象。
//
// 普通 new：分配内存 + 构造对象（两步合一）
// placement new：在指定地址上构造（只构造，不分配）

struct RadarObject {
    double x;
    double y;
    double z;

    RadarObject(double x_, double y_, double z_) : x(x_), y(y_), z(z_)
    {
        std::cout << "  RadarObject 构造函数被调用: {" << x << ", " << y << ", " << z << "}" << std::endl;
    }

    ~RadarObject()
    {
        std::cout << "  RadarObject 析构函数被调用: {" << x << ", " << y << ", " << z << "}" << std::endl;
    }

    void detect() const
    {
        std::cout << "  探测到雷达目标: {" << x << ", " << y << ", " << z << "}" << std::endl;
    }
};

int main()
{
    std::cout << "\n========== placement new — 在已有内存上构造对象 ==========" << std::endl;

    // 1. 普通 new：分配堆内存 + 构造对象
    std::cout << "\n  1. 普通 new:" << std::endl;
    RadarObject* heapObj = new RadarObject{10.0, 20.0, 30.0};
    std::cout << "     堆地址: " << heapObj << std::endl;
    heapObj->detect();
    delete heapObj; // 释放堆内存 + 析构

    // 2. placement new：在指定内存上构造对象
    std::cout << "\n  2. placement new (模拟共享内存):" << std::endl;

    // 模拟 iceoryx 在共享内存中分配一块原始内存
    // aligned_storage_t 确保内存大小和对齐都正确
    std::aligned_storage_t<sizeof(RadarObject), alignof(RadarObject)> sharedMemoryBlock;

    std::cout << "     共享内存块地址: " << &sharedMemoryBlock << std::endl;
    std::cout << "     共享内存块大小: " << sizeof(sharedMemoryBlock) << " bytes" << std::endl;

    // 在该共享内存块上构造 RadarObject（只构造，不分配新内存）
    RadarObject* sharedObj = new (&sharedMemoryBlock) RadarObject{1.0, 2.0, 3.0};
    std::cout << "     placement new 后的指针: " << sharedObj << " (指向共享内存)" << std::endl;
    sharedObj->detect();

    // placement new 构造的对象必须手动析构（不能 delete，因为内存不归堆管）
    sharedObj->~RadarObject();

    // 3. 演示 memory pool 模式
    std::cout << "\n  3. 模拟 memory pool:" << std::endl;

    // 预分配 3 个对象大小的内存池
    constexpr int POOL_SIZE = 3;
    std::aligned_storage_t<sizeof(RadarObject), alignof(RadarObject)> pool[POOL_SIZE];

    // 模拟从池中分配
    int poolIndex = 0;
    std::cout << "     分配第 1 个对象..." << std::endl;
    RadarObject* obj1 = new (&pool[poolIndex++]) RadarObject{10.0, 0.0, 0.0};
    obj1->detect();

    std::cout << "     分配第 2 个对象..." << std::endl;
    RadarObject* obj2 = new (&pool[poolIndex++]) RadarObject{20.0, 0.0, 0.0};
    obj2->detect();

    // 释放回池中（手动析构）
    std::cout << "     释放对象..." << std::endl;
    obj1->~RadarObject();
    obj2->~RadarObject();

    // 4. placement new 需要手动管理
    std::cout << "\n  4. iceoryx loan() 内部流程:" << std::endl;
    std::cout << "     1) 从共享内存分配一块原始内存 (userPayload)" << std::endl;
    std::cout << "     2) placement new 在该地址构造用户类型" << std::endl;
    std::cout << "     3) 返回 expected<Sample<T>, AllocationError>" << std::endl;
    std::cout << "     4) 用户 publish() 后，对象被析构，内存归还共享内存池" << std::endl;

    return 0;
}
