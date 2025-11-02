// example internal (not installed) header file

#pragma once

#include <iostream>

inline void internal_lib_fn() {
    std::cout << __FILE_NAME__ << "::" << __FUNCTION__ << "()\n";
}

void internal_lib_fn_2();