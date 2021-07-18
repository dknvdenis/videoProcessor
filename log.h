#pragma once

#include <iostream>

#define PRINT_ERROR(msg) \
    std::cerr << msg \
              << " " << __FILE__ << ": " << __LINE__ \
              << std::endl

#define PRINT_LOG(msg) \
    std::cout << msg \
              << " " << __FILE__ << ": " << __LINE__ \
              << std::endl

#define PRINT_LOG_SIMPLE(msg) \
    std::cout << msg
