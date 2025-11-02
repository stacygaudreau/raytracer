#include "gtest/gtest.h"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"


TEST(SpdlogAsync, FileSink) {
    // https://github.com/gabime/spdlog#asynchronous-logging
    // default thread pool settings can be modified *before* creating the async logger:
    spdlog::init_thread_pool(8192, 1); // queue with 8k items and 1 backing thread.
    auto async_file_logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/filesink_log.txt");
    async_file_logger->set_level(spdlog::level::info);
    spdlog::set_default_logger(async_file_logger);
    spdlog::info("async file sink created for spdlog");
}

TEST(SpdlogAsync, MultiLoggerAsync) {
    // multiple logger sinks in async mode
    // https://github.com/gabime/spdlog#asynchronous-logger-with-multi-sinks
    spdlog::init_thread_pool(8192, 1);
    auto sink_stdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto sink_rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/rotating.log", 5*(1024*1024), 5);
    auto sinks = std::vector<spdlog::sink_ptr>{ sink_stdout, sink_rotating };
    auto logger = std::make_shared<spdlog::async_logger>("async_logger", sinks.begin(), sinks.end(),
        spdlog::thread_pool(), spdlog::async_overflow_policy::discard_new);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

    spdlog::info("multi sink async logger created");
    spdlog::warn("will log to stdout and rotating file");
    spdlog::error("if buffer is overrun, will not block - overflow messages are discarded");
}