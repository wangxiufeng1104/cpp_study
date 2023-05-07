#include <iostream>
#include <vector>
#include <algorithm>
int main() {
    std::vector<int> vec = {1, 2, 3, 4};

    // 使用C++11中的for循环遍历vector
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;

    // 使用C++11中的范围for循环遍历vector
    for (auto elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // 使用迭代器遍历vector
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // 使用std::for_each算法遍历vector,需要使用 <algorithm> 头文件
    std::for_each(vec.begin(), vec.end(), [](int elem) {
        std::cout << elem << " ";
    });
    std::cout << std::endl;

    // 使用std::size函数获取vector的大小，需要编译器支持C++17
    for (size_t i = 0; i < std::size(vec); i++) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
