#include <iostream>
#include <memory>
class Simple
{
public:
    Simple() { std::cout << "Simple constructor called!" << std::endl; }
    Simple(int data): m_data(data) { std::cout << "Simple constructor called!" << std::endl; }
    void dis() {std::cout << "I`m Simple ,data is "<< m_data << std::endl;}
    ~Simple() { std::cout << "Simple destructor called!" << std::endl; }
private:
    int m_data = 0;
};
// 自定义deleter函数
void customDeleter(int* ptr) {
    std::cout << "自定义deleter被调用，释放资源：" << *ptr << std::endl;
    delete ptr;
}
void notLeaky()
{
    auto mySimpleSmartPtr {std::make_unique<Simple>(4)};
    mySimpleSmartPtr->dis();

    mySimpleSmartPtr.reset();
    mySimpleSmartPtr.reset(new Simple(1));
    mySimpleSmartPtr->dis();
    // reset 释放底层资源，并修改为另一个指针
    mySimpleSmartPtr.reset(new Simple(2));
    mySimpleSmartPtr->dis();
    // 使用release断开unique_ptr与底层的连接，需要手动释放资源
    Simple *simple = mySimpleSmartPtr.release();
    delete simple;
    simple = nullptr;

    // 使用自定义deleter的std::unique_ptr
    std::unique_ptr<int, void(*)(int*)> ptr(new int(42), customDeleter);

    // 使用ptr管理的资源
    std::cout << "资源值：" << *ptr << std::endl;

    // 使用自定义deleter的std::unique_ptr
    std::unique_ptr<int, decltype(&customDeleter)> ptr1(new int(43), customDeleter);

    // 使用自定义deleter的std::unique_ptr
    std::unique_ptr<int, decltype(&customDeleter)> ptr2(new int(44), customDeleter);

}

int main()
{
    Simple *mySimpleArrayPtr{new Simple[4]};
    delete[] mySimpleArrayPtr;
    mySimpleArrayPtr = nullptr;

    notLeaky();

    return 0;
}