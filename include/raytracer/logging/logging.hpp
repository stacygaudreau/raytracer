/**
 *  
 *  Raytracer Lib - Logging utilities
 *
 *  @file logging.hpp
 *  @brief Consolidates logging functionality into one place
 *  @author Stacy Gaudreau
 *  @date 2025.11.02
 *
 */


#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace Log {
    /** @brief Call once to initialize loggers at startup. */
    void init();
    std::shared_ptr<spdlog::logger> core();
    std::shared_ptr<spdlog::logger> renderer();
    inline const std::string LOGFILE_NAME{ "log/raytracer.log" };
}

#define CORE_TRACE(fmt, ...)    Log::core()->trace   ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define CORE_DEBUG(fmt, ...)    Log::core()->debug   ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define CORE_INFO(fmt, ...)     Log::core()->info    ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define CORE_WARN(fmt, ...)     Log::core()->warn    ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define CORE_ERROR(fmt, ...)    Log::core()->error   ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define CORE_CRITICAL(fmt, ...) Log::core()->critical("[{}] " fmt, __func__, ##__VA_ARGS__)

#define RENDER_TRACE(fmt, ...)    Log::renderer()->trace   ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define RENDER_DEBUG(fmt, ...)    Log::renderer()->debug   ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define RENDER_INFO(fmt, ...)     Log::renderer()->info    ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define RENDER_WARN(fmt, ...)     Log::renderer()->warn    ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define RENDER_ERROR(fmt, ...)    Log::renderer()->error   ("[{}] " fmt, __func__, ##__VA_ARGS__)
#define RENDER_CRITICAL(fmt, ...) Log::renderer()->critical("[{}] " fmt, __func__, ##__VA_ARGS__)


