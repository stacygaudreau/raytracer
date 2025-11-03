#include "logging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
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
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("raytracer.log", true);

        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [t:%t] %v");
        consoleSink->set_formatter(formatter->clone());
        fileSink->set_formatter(std::move(formatter));

        std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink };

        // loggers
        coreLogger = std::make_shared<spdlog::logger>("CORE", sinks.begin(), sinks.end());
        rendererLogger = std::make_shared<spdlog::logger>("RENDERER", sinks.begin(), sinks.end());

        // levels
        coreLogger->set_level(spdlog::level::trace);
        rendererLogger->set_level(spdlog::level::trace);

        // flush on warnings or higher
        coreLogger->flush_on(spdlog::level::warn);
        rendererLogger->flush_on(spdlog::level::warn);

        spdlog::register_logger(coreLogger);
        spdlog::register_logger(rendererLogger);
        spdlog::set_default_logger(coreLogger);
        spdlog::set_level(spdlog::level::trace);
    });
}

std::shared_ptr<spdlog::logger> core() { return coreLogger; }
std::shared_ptr<spdlog::logger> renderer() { return rendererLogger; }
}
