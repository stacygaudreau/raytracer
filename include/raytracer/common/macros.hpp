/**
 *  
 *  Raytracer Lib
 *
 *  @file macros.hpp
 *  @brief Common helper macros to use across the project
 *  @author Stacy Gaudreau
 *  @date 2025.11.03
 *
 */


#pragma once

#include <iostream>
#include <cstdlib>

/**
 * @brief Assertion macro which also logs
 */
#ifndef NDEBUG
#define ASSERT(cond, msg) \
    do { \
        if(!(cond)) [[unlikely]] { \
            std::cerr << "--ABORT--\n" \
                      << "assertion: " << msg << "\ncondition: " << #cond \
                      << "\n@location: " << __FILE__ << ":" << __LINE__ \
                      << "\n@function: " << __func__ << "\n\n"; \
            std::abort(); \
        } \
    } while(0)
#else
    #define ASSERT(cond, msg) do { (void)sizeof(cond); } while(0)
#endif

/**
 * @brief Delete copy/move ctor and assignment
 * operators for a given classname, making it non-copyable
 * and non-movable
 */
#define DELETE_COPY_AND_MOVE(ClassName) \
public: \
ClassName(const ClassName&) = delete; \
ClassName(const ClassName&&) = delete; \
ClassName& operator=(const ClassName&) = delete; \
ClassName& operator=(const ClassName&&) = delete;

/**
 * @brief Qualify class members as public if under test, else private.
 */
#ifdef IS_TEST_SUITE
#define PRIVATE_IN_PRODUCTION public:
#else
#define PRIVATE_IN_PRODUCTION private:
#endif
