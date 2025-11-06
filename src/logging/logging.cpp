#include "raytracer/logging/logging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <mutex>

namespace {
    std::shared_ptr<spdlog::logger> coreLogger;
    std::shared_ptr<spdlog::logger> rendererLogger;
    std::once_flag initOnlyOnce;
}


namespace Log {
void init() {
    std::call_once(initOnlyOnce, [](){
        // sinks
        constexpr size_t MAX_LOG_FILE_SIZE{ 1024 * 1024 * 5 };
        #ifndef IS_TEST_SUITE   // production sinks
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(LOGFILE_NAME,
                                                                                   MAX_LOG_FILE_SIZE, 5);
            std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink };
        #else  //  during gtest -> don't use stdout sink
            auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(LOGFILE_NAME,
                                                                                   MAX_LOG_FILE_SIZE, 5);
            std::vector<spdlog::sink_ptr> sinks{ fileSink };
        #endif

        // loggers
        spdlog::init_thread_pool(8192, 1);
        coreLogger = std::make_shared<spdlog::async_logger>(
            "CORE", sinks.begin(), sinks.end(), spdlog::thread_pool(),
            spdlog::async_overflow_policy::discard_new);
        rendererLogger = std::make_shared<spdlog::async_logger>(
            "RENDERER", sinks.begin(), sinks.end(),spdlog::thread_pool(),
            spdlog::async_overflow_policy::discard_new);

        // levels
        coreLogger->set_level(spdlog::level::trace);
        rendererLogger->set_level(spdlog::level::trace);
        spdlog::set_level(spdlog::level::trace);

        // flush on warnings or higher
        coreLogger->flush_on(spdlog::level::err);
        rendererLogger->flush_on(spdlog::level::err);

        spdlog::register_logger(coreLogger);
        spdlog::register_logger(rendererLogger);
        spdlog::set_default_logger(coreLogger);
        spdlog::flush_every(std::chrono::seconds{5});
    });
}

std::shared_ptr<spdlog::logger> core() { return coreLogger; }
std::shared_ptr<spdlog::logger> renderer() { return rendererLogger; }
}
