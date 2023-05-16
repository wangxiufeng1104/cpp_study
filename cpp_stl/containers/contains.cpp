//---------------------------------------------------
using namespace std;
#include <cstdlib> //RAND_MAX
#include <string>
#include <array>
#include <iostream>
#include <ctime>
#include <cstdlib> //qsort, bsearch, NULL
#include <memory.h>

// 以下 MyString 是為了測試 containers with moveable elements 效果.
class MyString
{
public:
    static size_t DCtor; // 累計 default-ctor 的呼叫次數
    static size_t Ctor;  // 累計 ctor      的呼叫次數
    static size_t CCtor; // 累計 copy-ctor 的呼叫次數
    static size_t CAsgn; // 累計 copy-asgn 的呼叫次數
    static size_t MCtor; // 累計 move-ctor 的呼叫次數
    static size_t MAsgn; // 累計 move-asgn 的呼叫次數
    static size_t Dtor;  // 累計 dtor 的呼叫次數
private:
    char *_data;
    size_t _len;
    void _init_data(const char *s)
    {
        _data = new char[_len + 1];
        memcpy(_data, s, _len);
        _data[_len] = '\0';
    }

public:
    // default ctor
    MyString() : _data(NULL), _len(0) { ++DCtor; }

    // ctor
    MyString(const char *p) : _len(strlen(p))
    {
        ++Ctor;
        _init_data(p);
    }

    // copy ctor
    MyString(const MyString &str) : _len(str._len)
    {
        ++CCtor;
        _init_data(str._data); // COPY
    }

    // move ctor, with "noexcept"
    MyString(MyString &&str) noexcept : _data(str._data), _len(str._len)
    {
        ++MCtor;
        str._len = 0;
        str._data = NULL; // 避免 delete (in dtor)
    }

    // copy assignment
    MyString &operator=(const MyString &str)
    {
        ++CAsgn;
        if (this != &str)
        {
            if (_data)
                delete _data;
            _len = str._len;
            _init_data(str._data); // COPY!
        }
        else
        {
            // Self Assignment, Nothing to do.
        }
        return *this;
    }

    // move assignment
    MyString &operator=(MyString &&str) noexcept
    {
        ++MAsgn;
        if (this != &str)
        {
            if (_data)
                delete _data;
            _len = str._len;
            _data = str._data; // MOVE!
            str._len = 0;
            str._data = NULL; // 避免 deleted in dtor
        }
        return *this;
    }

    // dtor
    virtual ~MyString()
    {
        ++Dtor;
        if (_data)
        {
            delete _data;
        }
    }

    bool
    operator<(const MyString &rhs) const // 為了讓 set 比較大小
    {
        return std::string(this->_data) < std::string(rhs._data); // 借用事實：string 已能比較大小.
    }
    bool
    operator==(const MyString &rhs) const // 為了讓 set 判斷相等.
    {
        return std::string(this->_data) == std::string(rhs._data); // 借用事實：string 已能判斷相等.
    }

    char *get() const { return _data; }
};
size_t MyString::DCtor = 0;
size_t MyString::Ctor = 0;
size_t MyString::CCtor = 0;
size_t MyString::CAsgn = 0;
size_t MyString::MCtor = 0;
size_t MyString::MAsgn = 0;
size_t MyString::Dtor = 0;
namespace std // 必須放在 std 內
{
    template <>
    struct hash<MyString> // 這是為了 unordered containers
    {
        size_t
        operator()(const MyString &s) const noexcept
        {
            return hash<string>()(string(s.get()));
        }
        // 借用現有的 hash<string> (in ...\include\c++\bits\basic_string.h)
    };
}
//-----------------
// 以下 MyStrNoMove 是為了測試 containers with no-moveable elements 效果.
class MyStrNoMove
{
public:
    static size_t DCtor; // 累計 default-ctor 的呼叫次數
    static size_t Ctor;  // 累計 ctor      的呼叫次數
    static size_t CCtor; // 累計 copy-ctor 的呼叫次數
    static size_t CAsgn; // 累計 copy-asgn 的呼叫次數
    static size_t MCtor; // 累計 move-ctor 的呼叫次數
    static size_t MAsgn; // 累計 move-asgn 的呼叫次數
    static size_t Dtor;  // 累計 dtor 的呼叫次數
private:
    char *_data;
    size_t _len;
    void _init_data(const char *s)
    {
        _data = new char[_len + 1];
        memcpy(_data, s, _len);
        _data[_len] = '\0';
    }

public:
    // default ctor
    MyStrNoMove() : _data(NULL), _len(0)
    {
        ++DCtor;
        _init_data("jjhou");
    }

    // ctor
    MyStrNoMove(const char *p) : _len(strlen(p))
    {
        ++Ctor;
        _init_data(p);
    }

    // copy ctor
    MyStrNoMove(const MyStrNoMove &str) : _len(str._len)
    {
        ++CCtor;
        _init_data(str._data); // COPY
    }

    // copy assignment
    MyStrNoMove &operator=(const MyStrNoMove &str)
    {
        ++CAsgn;

        if (this != &str)
        {
            if (_data)
                delete _data;
            _len = str._len;
            _init_data(str._data); // COPY!
        }
        else
        {
            // Self Assignment, Nothing to do.
        }
        return *this;
    }

    // dtor
    virtual ~MyStrNoMove()
    {
        ++Dtor;
        if (_data)
        {
            delete _data;
        }
    }

    bool
    operator<(const MyStrNoMove &rhs) const // 為了讓 set 比較大小
    {
        return string(this->_data) < string(rhs._data); // 借用事實：string 已能比較大小.
    }

    bool
    operator==(const MyStrNoMove &rhs) const // 為了讓 set 判斷相等.
    {
        return string(this->_data) == string(rhs._data); // 借用事實：string 已能判斷相等.
    }

    char *get() const { return _data; }
};
size_t MyStrNoMove::DCtor = 0;
size_t MyStrNoMove::Ctor = 0;
size_t MyStrNoMove::CCtor = 0;
size_t MyStrNoMove::CAsgn = 0;
size_t MyStrNoMove::MCtor = 0;
size_t MyStrNoMove::MAsgn = 0;
size_t MyStrNoMove::Dtor = 0;

namespace std // 必須放在 std 內
{
    template <>
    struct hash<MyStrNoMove> // 這是為了 unordered containers
    {
        size_t
        operator()(const MyStrNoMove &s) const noexcept
        {
            return hash<string>()(string(s.get()));
        }
        // 借用現有的 hash<string> (in ...\4.9.2\include\c++\bits\basic_string.h)
    };
}

const long ASIZE = 500000L;
#include <ctime> //clock_t, clock()
//--------------------------------------------------
#include <typeinfo> //typeid()
template <typename T>
void output_static_data(const T &myStr)
{
    cout << typeid(myStr).name() << " -- " << endl;
    cout << " CCtor=" << T::CCtor
         << " MCtor=" << T::MCtor
         << " CAsgn=" << T::CAsgn
         << " MAsgn=" << T::MAsgn
         << " Dtor=" << T::Dtor
         << " Ctor=" << T::Ctor
         << " DCtor=" << T::DCtor
         << endl;
}

template <typename M, typename NM>
void test_moveable(M c1, NM c2, long &value)
{
    char buf[10];

    // 測試 move
    cout << "\n\ntest, with moveable elements" << endl;
    typedef typename iterator_traits<typename M::iterator>::value_type V1type;
    clock_t timeStart = clock();
    for (long i = 0; i < value; ++i)
    {
        snprintf(buf, 10, "%d", rand());
        auto ite = c1.end();
        c1.insert(ite, V1type(buf));
    }
    cout << "construction, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
    cout << "size()= " << c1.size() << endl;
    output_static_data(*(c1.begin()));

    timeStart = clock();
    M c11(c1);
    cout << "copy, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

    timeStart = clock();
    M c12(std::move(c1));
    cout << "move copy, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

    timeStart = clock();
    c11.swap(c12);
    cout << "swap, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

    // 測試 non-moveable
    cout << "\n\ntest, with non-moveable elements" << endl;
    typedef typename iterator_traits<typename NM::iterator>::value_type V2type;
    timeStart = clock();
    for (long i = 0; i < value; ++i)
    {
        snprintf(buf, 10, "%d", rand());
        auto ite = c2.end();
        c2.insert(ite, V2type(buf));
    }

    cout << "construction, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
    cout << "size()= " << c2.size() << endl;
    output_static_data(*(c2.begin()));

    timeStart = clock();
    NM c21(c2);
    cout << "copy, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

    timeStart = clock();
    NM c22(std::move(c2));
    cout << "move copy, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

    timeStart = clock();
    c21.swap(c22);
    cout << "swap, milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
}
//-----------------
long get_a_target_long()
{
    long target = 0;

    cout << "target (0~" << RAND_MAX << "): ";
    cin >> target;
    return target;
}
string get_a_target_string()
{
    long target = 0;
    char buf[10];

    cout << "target (0~" << RAND_MAX << "): ";
    cin >> target;
    snprintf(buf, 10, "%ld", target);
    return string(buf);
}

int compareLongs(const void *a, const void *b)
{
    return (*(long *)a - *(long *)b);
}

int compareStrings(const void *a, const void *b)
{
    if (*(string *)a > *(string *)b)
        return 1;
    else if (*(string *)a < *(string *)b)
        return -1;
    else
        return 0;
}

namespace jj01
{
    void test_array()
    {
        cout << "\ntest_array()............\n";
        array<long, ASIZE> c;

        clock_t timeStart = clock();
        for (long i = 0; i < ASIZE; i++)
        {
            c[i] = rand();
        }
        clock_t timeEnd = clock();
        double elapsedMilliseconds = static_cast<double>(timeEnd - timeStart) / CLOCKS_PER_SEC * 1000;
        cout << "milli-seconds : " << elapsedMilliseconds << endl; //
        cout << "array.size()= " << c.size() << endl;
        cout << "array.front()= " << c.front() << endl;   // 第一个元素
        cout << "array.back()= " << c.back() << endl;     // 最后一个元素
        cout << "array center= " << c[ASIZE / 2] << endl; // 中间元素
        cout << "array.data()= " << c.data() << endl;     // 数组首地址

        long target = get_a_target_long();

        timeStart = clock();                                  // clock() 函数返回程序使用的处理器时间的近似值。单位ms
        ::qsort(c.data(), ASIZE, sizeof(long), compareLongs); // 排序

        // bsearch 二分查找法搜索，前提数据已经进行过qort，如果数据是乱的，不能用bsearch
        long *pItem = (long *)::bsearch(&target, (c.data()), ASIZE, sizeof(long), compareLongs);
        timeEnd = clock();
        elapsedMilliseconds = static_cast<double>(timeEnd - timeStart) / CLOCKS_PER_SEC * 1000;
        cout << "qsort()+bsearch(), milli-seconds : " << (elapsedMilliseconds) << endl; //
        if (pItem != NULL)
            cout << "found, " << *pItem << endl;
        else
            cout << "not found! " << endl;
    }
}
//---------------------------------------------------
#include <vector>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
#include <algorithm> //sort()
namespace jj02
{
    void test_vector(long &value)
    {
        cout << "\ntest_vector().......... \n";

        vector<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.push_back(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                // 曾經最高 i=58389486 then std::bad_alloc
                abort();
            }
        }
        cout << "milli-seconds : " << static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000 << endl;
        cout << "vector.max_size()= " << c.max_size() << endl; // 1073747823
        cout << "vector.size()= " << c.size() << endl;
        cout << "vector.front()= " << c.front() << endl;
        cout << "vector.back()= " << c.back() << endl;
        cout << "vector.data()= " << c.data() << endl;
        cout << "vector.capacity()= " << c.capacity() << endl
             << endl;

        string target = get_a_target_string();
        {
            timeStart = clock();
            auto pItem = find(c.begin(), c.end(), target);
            cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

            if (pItem != c.end())
                cout << "found, " << *pItem << endl
                     << endl;
            else
                cout << "not found! " << endl
                     << endl;
        }

        {
            timeStart = clock();
            sort(c.begin(), c.end());
            cout << "sort(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

            timeStart = clock();
            string *pItem = (string *)::bsearch(&target, (c.data()),
                                                c.size(), sizeof(string), compareStrings);
            cout << "bsearch(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

            if (pItem != NULL)
                cout << "found, " << *pItem << endl
                     << endl;
            else
                cout << "not found! " << endl
                     << endl;
        }

        c.clear();
        test_moveable(vector<MyString>(), vector<MyStrNoMove>(), value);
    }
}
//---------------------------------------------------
#include <list>
#include <stdexcept>
#include <string>
#include <cstdlib>   //abort()
#include <cstdio>    //snprintf()
#include <algorithm> //find()
#include <iostream>
#include <ctime>
namespace jj03
{
    void test_list(long &value)
    {
        cout << "\ntest_list().......... \n";

        list<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.push_back(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "list.size()= " << c.size() << endl;
        cout << "list.max_size()= " << c.max_size() << endl; // 357913941
        cout << "list.front()= " << c.front() << endl;
        cout << "list.back()= " << c.back() << endl;

        string target = get_a_target_string();
        timeStart = clock();
        auto pItem = find(c.begin(), c.end(), target); // 顺序查找
        cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        if (pItem != c.end())
            cout << "found, " << *pItem << endl;
        else
            cout << "not found! " << endl;

        timeStart = clock();
        c.sort(); // 使用容器本身的sort
        cout << "c.sort(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        c.clear();
        test_moveable(list<MyString>(), list<MyStrNoMove>(), value);
    }
}
//---------------------单向链表------------------------------
#include <forward_list>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj04
{
    void test_forward_list(long &value)
    {
        cout << "\ntest_forward_list().......... \n";

        forward_list<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.push_front(string(buf)); // 单向链表，只提供头插
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "forward_list.max_size()= " << c.max_size() << endl; // 536870911
        cout << "forward_list.front()= " << c.front() << endl;

        string target = get_a_target_string();
        timeStart = clock();
        auto pItem = ::find(c.begin(), c.end(), target); // 顺序查找
        cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        if (pItem != c.end())
            cout << "found, " << *pItem << endl;
        else
            cout << "not found! " << endl;

        timeStart = clock();
        c.sort();
        cout << "c.sort(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        c.clear();
    }
}
//---------------------------------------------------
#include <deque>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj05
{
    void test_deque(long &value)
    {
        cout << "\ntest_deque().......... \n";

        deque<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.push_back(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "deque.size()= " << c.size() << endl;
        cout << "deque.front()= " << c.front() << endl;
        cout << "deque.back()= " << c.back() << endl;
        cout << "deque.max_size()= " << c.max_size() << endl; // 1073741821

        string target = get_a_target_string();
        timeStart = clock();
        auto pItem = find(c.begin(), c.end(), target);
        cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        if (pItem != c.end())
            cout << "found, " << *pItem << endl;
        else
            cout << "not found! " << endl;

        timeStart = clock();
        sort(c.begin(), c.end());
        cout << "sort(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        c.clear();
        test_moveable(deque<MyString>(), deque<MyStrNoMove>(), value);
    }
}
//-----------------------集合----------------------------
#include <set>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj06
{
    void test_multiset(long &value)
    {
        cout << "\ntest_multiset().......... \n";

        multiset<string> c;
        char buf[10];
        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.insert(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "multiset.size()= " << c.size() << endl;
        cout << "multiset.max_size()= " << c.max_size() << endl; // 214748364

        string target = get_a_target_string();
        {
            timeStart = clock();
            auto pItem = find(c.begin(), c.end(), target); // 比 c.find(...) 慢很多
            cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }

        {
            timeStart = clock();
            auto pItem = c.find(target); // 比 std::find(...) 快很多
            cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }

        c.clear();
        test_moveable(multiset<MyString>(), multiset<MyStrNoMove>(), value);
    }
}
//---------------------------------------------------
#include <map>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj07
{
    void test_multimap(long &value)
    {
        cout << "\ntest_multimap().......... \n";

        multimap<long, string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                // multimap 不可使用 [] 做 insertion
                c.insert(pair<long, string>(i, buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "multimap.size()= " << c.size() << endl;
        cout << "multimap.max_size()= " << c.max_size() << endl; // 178956970

        long target = get_a_target_long();
        timeStart = clock();
        auto pItem = c.find(target);
        cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        if (pItem != c.end())
            cout << "found, value=" << (*pItem).second << endl;
        else
            cout << "not found! " << endl;

        c.clear();
    }
}
//---------------------------------------------------
#include <unordered_set>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj08
{
    void test_unordered_multiset(long &value)
    {
        cout << "\ntest_unordered_multiset().......... \n";

        unordered_multiset<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.insert(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "unordered_multiset.size()= " << c.size() << endl;
        cout << "unordered_multiset.max_size()= " << c.max_size() << endl; // 357913941
        cout << "unordered_multiset.bucket_count()= " << c.bucket_count() << endl;
        cout << "unordered_multiset.load_factor()= " << c.load_factor() << endl;
        cout << "unordered_multiset.max_load_factor()= " << c.max_load_factor() << endl;
        cout << "unordered_multiset.max_bucket_count()= " << c.max_bucket_count() << endl;
        for (unsigned i = 0; i < 20; ++i)
        {
            cout << "bucket #" << i << " has " << c.bucket_size(i) << " elements.\n";
        }

        string target = get_a_target_string();
        {
            timeStart = clock();
            auto pItem = find(c.begin(), c.end(), target); // 比 c.find(...) 慢很多
            cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }

        {
            timeStart = clock();
            auto pItem = c.find(target); // 比 std::find(...) 快很多
            cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }

        c.clear();
        test_moveable(unordered_multiset<MyString>(), unordered_multiset<MyStrNoMove>(), value);
    }
}
//---------------------------------------------------
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj09
{
    void test_unordered_multimap(long &value)
    {
        cout << "\ntest_unordered_multimap().......... \n";

        unordered_multimap<long, string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                // multimap 不可使用 [] 進行 insertion
                c.insert(pair<long, string>(i, buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "unordered_multimap.size()= " << c.size() << endl;
        cout << "unordered_multimap.max_size()= " << c.max_size() << endl; // 357913941

        long target = get_a_target_long();
        timeStart = clock();
        auto pItem = c.find(target);
        cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        if (pItem != c.end())
            cout << "found, value=" << (*pItem).second << endl;
        else
            cout << "not found! " << endl;
    }
}
//---------------------------------------------------
#if 0
#include <ext\slist>
// 注意, 上一行並沒有引發警告訊息如 #include <ext\hash_set> 所引發者：
//...\4.9.2\include\c++\backward\backward_warning.h
//[Warning] ...

#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj10
{
    void test_slist(long &value)
    {
        cout << "\ntest_slist().......... \n";

        __gnu_cxx::slist<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.push_front(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
    }
}
#endif
#if 0
//---------------------------------------------------
/*
以下測試 hash_multiset, hash_multimap 過程中遇到阻礙：
headers <hash_set> 和 <hash_map> 各有兩個，
分別在 ...\4.9.2\include\c++\backward 和 ...\4.9.2\include\c++\ext，
不知要用哪一組!
用 <ext\...> 那一組會有問題
   ...\4.9.2\include\c++\backward\hashtable.h
   [Error] no match for call to '(const hasher {aka const __gnu_cxx::hash<std::basic_string<char> >}) (const key_type&)'
用 <backward\...> 那一組有相同的問題.
so, 放棄測試 (no insertion or push_back or ...).
*/

#include <ext\hash_set>
//...\4.9.2\include\c++\backward\backward_warning.h
//[Warning] #warning This file includes at least one deprecated or antiquated header
// which may be removed without further notice at a future date.
// Please use a non-deprecated interface with equivalent functionality instead.
// For a listing of replacement headers and interfaces, consult the file backward_warning.h.
// To disable this warning use -Wno-deprecated. [-Wcpp]

#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj11
{
    void test_hash_multiset(long &value)
    {
        cout << "\ntest_hash_multiset().......... \n";

        __gnu_cxx::hash_multiset<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                //! c.insert(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
    }
}
#endif
//---------------------------------------------------
#if 0
#include <ext\hash_map>
//...\4.9.2\include\c++\backward\backward_warning.h
//[Warning] #warning This file ... (如上個函數所言)
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj12
{
    void test_hash_multimap(long &value)
    {
        cout << "\ntest_hash_multimap().......... \n";

        __gnu_cxx::hash_multimap<long, string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                // c.insert(...
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;

        timeStart = clock();
        //! auto ite = c.find(...
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
    }
}
#endif
//---------------------------------------------------
#include <set>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj13
{
    void test_set(long &value)
    {
        cout << "\ntest_set().......... \n";

        set<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.insert(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "set.size()= " << c.size() << endl;
        cout << "set.max_size()= " << c.max_size() << endl; // 214748364

        string target = get_a_target_string();
        {
            timeStart = clock();
            auto pItem = find(c.begin(), c.end(), target); // 比 c.find(...) 慢很多
            cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }

        {
            timeStart = clock();
            auto pItem = c.find(target); // 比 std::find(...) 快很多
            cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }
    }
}
//---------------------------------------------------
#include <map>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj14
{
    void test_map(long &value)
    {
        cout << "\ntest_map().......... \n";

        map<long, string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c[i] = string(buf);
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "map.size()= " << c.size() << endl;
        cout << "map.max_size()= " << c.max_size() << endl; // 178956970

        long target = get_a_target_long();
        timeStart = clock();
        auto pItem = c.find(target);
        cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        if (pItem != c.end())
            cout << "found, value=" << (*pItem).second << endl;
        else
            cout << "not found! " << endl;

        c.clear();
    }
}
//---------------------------------------------------
#include <unordered_set>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj15
{
    void test_unordered_set(long &value)
    {
        cout << "\ntest_unordered_set().......... \n";

        unordered_set<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.insert(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "unordered_set.size()= " << c.size() << endl;
        cout << "unordered_set.max_size()= " << c.max_size() << endl; // 357913941
        cout << "unordered_set.bucket_count()= " << c.bucket_count() << endl;
        cout << "unordered_set.load_factor()= " << c.load_factor() << endl;
        cout << "unordered_set.max_load_factor()= " << c.max_load_factor() << endl;
        cout << "unordered_set.max_bucket_count()= " << c.max_bucket_count() << endl;
        for (unsigned i = 0; i < 20; ++i)
        {
            cout << "bucket #" << i << " has " << c.bucket_size(i) << " elements.\n";
        }

        string target = get_a_target_string();
        {
            timeStart = clock();
            auto pItem = find(c.begin(), c.end(), target); // 比 c.find(...) 慢很多
            cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }

        {
            timeStart = clock();
            auto pItem = c.find(target); // 比 std::find(...) 快很多
            cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
            if (pItem != c.end())
                cout << "found, " << *pItem << endl;
            else
                cout << "not found! " << endl;
        }
    }
}
//---------------------------------------------------
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj16
{
    void test_unordered_map(long &value)
    {
        cout << "\ntest_unordered_map().......... \n";

        unordered_map<long, string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c[i] = string(buf);
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "unordered_map.size()= " << c.size() << endl; // 357913941
        cout << "unordered_map.max_size()= " << c.max_size() << endl;

        long target = get_a_target_long();
        timeStart = clock();
        //! auto pItem = find(c.begin(), c.end(), target);	//map 不適用 std::find()
        auto pItem = c.find(target);

        cout << "c.find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        if (pItem != c.end())
            cout << "found, value=" << (*pItem).second << endl;
        else
            cout << "not found! " << endl;
    }
}
//---------------------------------------------------
#include <stack>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj17
{
    void test_stack(long &value)
    {
        cout << "\ntest_stack().......... \n";

        stack<string> c;
        char buf[10];

        clock_t timeStart = clock();
        for (long i = 0; i < value; ++i)
        {
            try
            {
                snprintf(buf, 10, "%d", rand());
                c.push(string(buf));
            }
            catch (exception &p)
            {
                cout << "i=" << i << " " << p.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (clock() - timeStart) << endl;
        cout << "stack.size()= " << c.size() << endl;
        cout << "stack.top()= " << c.top() << endl;
        c.pop();
        cout << "stack.size()= " << c.size() << endl;
        cout << "stack.top()= " << c.top() << endl;

        {
            stack<string, list<string>> c; // 以 list 為底層
            for (long i = 0; i < 10; ++i)
            {
                snprintf(buf, 10, "%d", rand());
                c.push(string(buf));
            }
            cout << "stack.size()= " << c.size() << endl;
            cout << "stack.top()= " << c.top() << endl;
            c.pop();
            cout << "stack.size()= " << c.size() << endl;
            cout << "stack.top()= " << c.top() << endl;
        }

        {
            stack<string, vector<string>> c; // 以 vector 為底層
            for (long i = 0; i < 10; ++i)
            {
                snprintf(buf, 10, "%d", rand());
                c.push(string(buf));
            }
            cout << "stack.size()= " << c.size() << endl;
            cout << "stack.top()= " << c.top() << endl;
            c.pop();
            cout << "stack.size()= " << c.size() << endl;
            cout << "stack.top()= " << c.top() << endl;
        }

        {
            stack<string, set<string>> c; // 以 set 為底層
            /*!
                for(long i=0; i< 10; ++i) {
                        snprintf(buf, 10, "%d", rand());
                        c.push(string(buf));
                }
                cout << "stack.size()= " << c.size() << endl;
                cout << "stack.top()= " << c.top() << endl;
                c.pop();
                cout << "stack.size()= " << c.size() << endl;
                cout << "stack.top()= " << c.top() << endl;

            //[Error] 'class std::set<std::basic_string<char> >' has no member named 'push_back'
            //[Error] 'class std::set<std::basic_string<char> >' has no member named 'back'
            //[Error] 'class std::set<std::basic_string<char> >' has no member named 'pop_back'
            */
        }

        //! stack<string, map(string>> c5;	////以 map 為底層, [Error] template argument 2 is invalid
        //! stack<string>::iterator ite1;  	//[Error] 'iterator' is not a member of 'std::stack<std::basic_string<char> >'
    }
}
//---------------------------------------------------
#include <set>
#include <stdexcept>
#include <string>
#include <cstdlib> //abort()
#include <cstdio>  //snprintf()
#include <iostream>
#include <ctime>
namespace jj18
{
    void test_set(long &value)
    {
        cout << "\n test_set() .....\n";
        set<uint32_t> c;
        clock_t timeStart = clock();
        uint32_t target;
        for (long i = 0; i < value; ++i)
        {
            try
            {
                uint32_t temp = rand();
                // cout << temp << endl;
                c.insert(temp);
                target = temp;
            }
            catch (const std::exception &e)
            {
                cout << "i=" << i << " " << e.what() << endl;
                abort();
            }
        }
        cout << "milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        cout << "set.size()= " << c.size() << endl;
        cout << "set.max_size()= " << c.max_size() << endl; // 214748364

        // for(auto it = c.begin(); it != c.end(); it ++)
        // {
        //     cout << *it << endl;
        // }

        timeStart = clock();
        auto pItem = find(c.begin(), c.end(), target); // 比 c.find(...) 慢很多
        cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        if (pItem != c.end())
            cout << "found, " << *pItem << endl;
        else
            cout << "not found! " << endl;
        timeStart = clock();
        pItem = c.find(target);
        cout << "std::find(), milli-seconds : " << (static_cast<double>(clock() - timeStart) / CLOCKS_PER_SEC * 1000) << endl;
        if (pItem != c.end())
            cout << "found, " << *pItem << endl;
        else
            cout << "not found! " << endl;
    }
}
//---------------------------------------------------
#include <cstdlib> //rand() and RAND_MAX
namespace jjxx
{
    void test_containers()
    {
        int choice;
        long value;

        srand(time(NULL));

        cout << "\n\ntest_containers()......... \n";
        cout << "select: \n";
        cout << "       (1)array (2)vector (3)list (4)forward_list (5)deque (6)multiset \n";
        cout << "       (7)multimap (8)unordered_multiset (9)unordered_multimap (10)slist \n";
        cout << "       (11)hash_multiset (12)hash_multimap (13)set (14)map (15)unordered_set \n";
        cout << "       (16)unordered_map (17)stack (18)queue \n";
        cout << " (2),(3),(5),(6),(8) will test also moveable elements. \n";
        cin >> choice;
        if (choice != 1) // 1 ==> array, use ASIZE
        {
            cout << "how many elements: ";
            cin >> value;
        }
        switch (choice)
        {
        case 1:
            jj01::test_array();
            break;
        case 2:
            jj02::test_vector(value);
            break;
        case 3:
            jj03::test_list(value);
            break;
        case 4:
            jj04::test_forward_list(value);
            break;
        case 5:
            jj05::test_deque(value);
            break;
        case 6:
            jj06::test_multiset(value);
            break;
        case 7:
            jj07::test_multimap(value);
            break;
        case 8:
            jj08::test_unordered_multiset(value);
            break;
        case 9:
            jj09::test_unordered_multimap(value);
            break;
        case 13:
            jj13::test_set(value);
            break;
        case 14:
            jj14::test_map(value);
            break;
        case 15:
            jj15::test_unordered_set(value);
            break;
        case 16:
            jj16::test_unordered_map(value);
            break;
        case 17:
            jj17::test_stack(value);
            break;
        case 18:
            jj18::test_set(value);
            break;
        default:
            break;
        }
    }
}

#include <cstdlib> //rand() and RAND_MAX

int main(int argc, char **argv)
{
    jjxx::test_containers();
}