#include <iostream>
namespace TEST1
{
    class Super
    {
    public:
        Super() {}
        virtual void someMethod() { std::cout << "this is super someMethod" << std::endl; }

    protected:
        int mProtectedInt;

    private:
        int mPrivateInt;
    };
    class Sub : public Super
    {
    public:
        Sub() {}
        Sub(int a) { mProtectedInt = a; }
        virtual void someMethod() override { std::cout << "this is sub someMethod" << std::endl; }
        void someOtherMethod() { std::cout << "this sub someOtherMethod" << std::endl; }
    };
};
/*
    TEST2 展示了对象的创建顺序
    1、基类构造函数
    2、成员变量
    3、派生类的构造函数
*/

namespace TEST2
{
    class Something
    {
    public:
        Something() { std::cout << "2" << std::endl; }
    };

    class Parent
    {
    public:
        Parent() { std::cout << "1" << std::endl; }
    };
    class Child : public Parent
    {
    public:
        Child() : mDataMember{} { std::cout << "3" << std::endl; }

    private:
        Something mDataMember;
    };
};
/*
    TEST3 展示了对象的销毁顺序
    1、派生类的构造函数
    2、成员变量
    3、基类构造函数
*/
namespace TEST3
{
    class Something
    {
    public:
        Something() { std::cout << "2" << std::endl; }
        virtual ~Something() { std::cout << "2" << std::endl; }
    };

    class Parent
    {
    public:
        Parent() { std::cout << "1" << std::endl; }
        virtual ~Parent() { std::cout << "1" << std::endl; }
    };
    class Child : public Parent
    {
    public:
        Child() { std::cout << "3" << std::endl; }
        virtual ~Child() { std::cout << "3" << std::endl; }

    private:
        Something mDataMember;
    };
};

int main()
{
    // TEST1::Sub mySub;
    // TEST1::Super &ref = mySub;
    // ref.someMethod();
    
    // TEST2::Child myChild;

    // TEST3::Child myChild1;
    TEST3::Parent *ptr = new TEST3::Child();
    delete ptr;
    return 0;
}