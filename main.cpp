#include <algorithm>
#include <execution>
#include <iostream>
#include <string>
#include <chrono>

int main() {

    std::vector<int> myInts{1, 2, 3, 4, 5};
    std::vector<std::string> myStrings(myInts.size());

    auto start = std::chrono::high_resolution_clock::now();
    std::transform(std::execution::par, myInts.begin(), myInts.end(), myStrings.begin(), [](int i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return "string=" + std::to_string(i);
    });
    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "Elapsed=" << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

    std::for_each(myStrings.begin(), myStrings.end(), [](auto s){
        std::cout << s << std::endl;
    });

    return 0;
}