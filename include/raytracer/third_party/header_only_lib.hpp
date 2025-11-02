#pragma once

#include <iostream>

inline void header_lib_hello() {
    std::cout << __FILE_NAME__ << "::" << __FUNCTION__ << "::()\n";
}