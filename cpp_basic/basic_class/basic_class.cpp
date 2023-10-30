#include <iostream>
using namespace std;

// explicit 禁止隐式类型转换
// delete  禁用或删除某个特定函数
// 重载 编译器选择参数类型匹配的那个函数
class myClass
{
private:
    /* data */
    int m_data{0};
    int m_data_1{0};
    const int mId ; // const 数据成员需要在ctor-initializer初始化

public:
    // explicit 禁止隐式类型转换，myClass m = 4 提示 error: conversion from ‘int’ to non-scalar type ‘myClass’ requested
    explicit myClass(int data) : m_data(data),mId(data) { cout << "int constructor function" << endl; }
    explicit myClass(const string &str) : m_data(stoi(str)),mId(stoi(str)) { cout << "string constructor function" << endl; }
    //myClass(const string& str) = delete;   
    // 委托构造函数
    myClass(int a, const string &str) : m_data(a), m_data_1(stoi(str)),mId(a) { cout << "int and string constructor" << endl; }
    myClass() : myClass(1234, "4321"s) { cout << "delegating constructor function" << endl; }
    void dis() const { cout << m_data << endl; }
    // 拷贝构造函数
    myClass(const myClass &src) : mId(100)
    {
        cout << "copy constructor function" << endl;
        this->m_data = src.m_data;
    }
    myClass &operator=(const myClass &src)
    {
        this->m_data = src.m_data;
        this->m_data_1 = src.m_data_1;
        cout << "赋值运算符" << endl;
        return *this;
    }
    myClass &operator+(const myClass& my)
    {
        this->m_data += my.m_data;
        return *this;
    }
    myClass &operator+=(const myClass& my)
    {
        this->m_data += my.m_data;
        return *this;
    }
    ~myClass() {cout << "desconstor" << m_data << endl;}
};
std::ostream& operator<<(std::ostream& os, const myClass& obj) 
{
    obj.dis();
    return os;
}
void dump_class(const myClass &my)
{
    cout << "address " << &my << endl <<"dump " << my << endl;
}

int main()
{
    // 1、重载
    myClass a(3);
    a.dis();
    myClass aa("123"s);
    aa.dis();
    // 2、拷贝构造
    myClass aaa = a;
    aaa.dis();

    myClass aaaa(1111, "3222");
    aaaa.dis();
    // 无参构造函数对象后面不能有括号，会导致认为是函数
    myClass aaaaa;
    aaaaa.dis();

    cout << "演示复制运算符" << endl;
    myClass a1;
    cout << "赋值前" << endl;
    a1.dis();
    a1 = aa;
    cout << "赋值后" << endl;
    a1.dis();

    cout << "addr1 " << &a1 << endl;
    dump_class(a1);


    // operate+
    cout << "################operate + ###############" << endl;
    myClass obj1(333),obj2(444);
    obj1 = obj1 + obj2;
    obj1.dis();
    cout << "################operate + ###############" << endl;


    cout << "good bye" << endl;
}