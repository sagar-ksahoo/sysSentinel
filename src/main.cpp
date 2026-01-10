#include <iostream>
#include <vector>
#include <thread>

#ifdef __cpp_lib_format
#include <format>
#else
#include <string>
#endif

int main() {
    std::cout << "SysSentinel Service Starting..." << std::endl;
    
    unsigned int num_cores = std::thread::hardware_concurrency();
    std::cout << "Number of CPU cores: " << num_cores << std::endl;
    
    return 0;
}
