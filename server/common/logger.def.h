#pragma once

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <yaml-cpp/yaml.h>

inline std::shared_ptr<spdlog::logger> CreateDefaultLogger(const spdlog::sink_ptr &consoleSink, const std::string &logName, const std::string &path) {
    const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(path, 2, 30);
    fileSink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

    const auto logger = std::make_shared<spdlog::logger>(logName, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    register_logger(logger);
    return logger;
}

inline void InitLogger(const YAML::Node& config) {
    const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);

    const auto logRootDir = config["log_dir"].as<std::string>();

// #ifdef _DEBUG
//     CreateDefaultLogger(consoleSink, "base_logger", logRootDir + "base/base.log");
// #endif

    CreateDefaultLogger(consoleSink, "client_command_logger", logRootDir + "command/client/client.log");
    CreateDefaultLogger(consoleSink, "operate_command_logger", logRootDir + "command/operate/operate.log");
}