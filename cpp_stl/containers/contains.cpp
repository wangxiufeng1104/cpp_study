//---------------------------------------------------
using namespace std;
#include <cstdlib> //RAND_MAX
#include <string>
#include <array>
#include <iostream>
#include <ctime>
#include <cstdlib> //qsort, bsearch, NULL

const long ASIZE = 500000L;
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