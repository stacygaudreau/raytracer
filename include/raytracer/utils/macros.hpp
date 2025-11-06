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
