#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
class Singleton
{
protected:
    Singleton(std::string newValue): value_(newValue)
    {

    }

    static Singleton *singleton_ ;
    std::string value_;
public:
    // 单例模式不可以被克隆
    // Singleton s1;
    // Singleton s2 = s1 这种方式不被允许
    Singleton(Singleton &other) = delete;
    // 单例模式的关键特性之一是，在程序的整个生命周期内只能存在一个实例。
    // 通过删除赋值操作符，你可以确保单例对象不会被复制到另一个对象，从而维持单例模式的完整性。
    void operator=(const Singleton &) = delete;


    static Singleton *GetInstance(const std::string& value);
    void SomeBusinessLogic()
    {
        // ...
    }
    std::string value() const{
        return value_;
    } 
};
class SingletonThreadSafety
{

    /**
     * The SingletonThreadSafety's constructor/destructor should always be private to
     * prevent direct construction/desctruction calls with the `new`/`delete`
     * operator.
     */
private:
    static SingletonThreadSafety * pinstance_;
    static std::mutex mutex_;

protected:
    SingletonThreadSafety(const std::string value): value_(value)
    {
    }
    ~SingletonThreadSafety() {}
    std::string value_;

public:
    /**
     * SingletonThreadSafety should not be cloneable.
     */
    SingletonThreadSafety(SingletonThreadSafety &other) = delete;
    /**
     * SingletonThreadSafety should not be assignable.
     */
    void operator=(const SingletonThreadSafety &) = delete;
    /**
     * This is the static method that controls the access to the SingletonThreadSafety
     * instance. On the first run, it creates a SingletonThreadSafety object and places it
     * into the static field. On subsequent runs, it returns the client existing
     * object stored in the static field.
     */

    static SingletonThreadSafety *GetInstance(const std::string& value);
    /**
     * Finally, any singleton should define some business logic, which can be
     * executed on its instance.
     */
    void SomeBusinessLogic()
    {
        // ...
    }
    
    std::string value() const{
        return value_;
    } 
};
void ThreadFoo(){
    // Following code emulates slow initialization.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Singleton* singleton = Singleton::GetInstance("FOO");
    std::cout << singleton->value() << "\n";
}

void ThreadBar(){
    // Following code emulates slow initialization.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Singleton* singleton = Singleton::GetInstance("BAR");
    std::cout << singleton->value() << "\n";
}
void ThreadArno(){
    // Following code emulates slow initialization.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Singleton* singleton = Singleton::GetInstance("Arno");
    std::cout << singleton->value() << "\n";
}
void ThreadFoo1(){
    // Following code emulates slow initialization.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    SingletonThreadSafety* singleton = SingletonThreadSafety::GetInstance("FOO");
    std::cout << singleton->value() << "\n";
}

void ThreadBar1(){
    // Following code emulates slow initialization.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    SingletonThreadSafety* singleton = SingletonThreadSafety::GetInstance("BAR");
    std::cout << singleton->value() << "\n";
}
int main()
{
    std::cout <<"If you see the same value, then singleton was reused (yay!\n" <<
                "If you see different values, then 2 singletons were created (booo!!)\n\n" <<
                "RESULT:\n";   
    std::thread t1(ThreadFoo);
    std::thread t2(ThreadBar);
    std::thread t3(ThreadArno);
    t1.join();
    t2.join();
    t3.join();
    std::thread t4(ThreadFoo1);
    std::thread t5(ThreadBar1);
    t4.join();
    t5.join();
    return 0;
}
// 静态成员在类中仅仅是声明，没有初始化内存空间，因此需要在类外面定义，给变量分配内存。
Singleton* Singleton::singleton_= nullptr;
Singleton *Singleton::GetInstance(const std::string &value)
{
    if(singleton_ == nullptr)
    {
        singleton_ = new Singleton(value);   
    }
    return singleton_;
}
SingletonThreadSafety *SingletonThreadSafety::pinstance_ = nullptr;
std::mutex SingletonThreadSafety::mutex_;
SingletonThreadSafety *SingletonThreadSafety::GetInstance(const std::string &value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(pinstance_ == nullptr)
    {
        pinstance_ = new SingletonThreadSafety(value);   
    }
    return pinstance_;
}