#include "../include/server_logger_builder.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <regex>

using json = nlohmann::json;

logger_builder& server_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity) &
{
    _output_streams[severity] = {stream_file_path, false};
    return *this;
}

logger_builder& server_logger_builder::add_console_stream(
    logger::severity severity) &
{
    _output_streams[severity] = {"", true};
    return *this;
}

/*
{
    "logger_config": {
        "destination": "http://127.0.0.1:9200",
        "format": value,
        "streams": {
            "trace": {
                "type": "console"
            },
            "debug": {
                "type": "file",
                "path": "debug.log"
            },
            ...
        }
    }
}

*/

logger_builder& server_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    std::ifstream config_file(configuration_file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Cannot open configuration file");
    }

    try {
        json config = json::parse(config_file);
        json logger_config = config.at(configuration_path);

        clear();

        if (logger_config.contains("destination")) {
            set_destination(logger_config["destination"]);
        }

        if (logger_config.contains("format")) {
            set_format(logger_config["format"]);
        }

        for (auto& [severity_str, stream] : logger_config["streams"].items()) {
            logger::severity severity;
            if (severity_str == "trace") severity = logger::severity::trace;
            else if (severity_str == "debug") severity = logger::severity::debug;
            else if (severity_str == "information") severity = logger::severity::information;
            else if (severity_str == "warning") severity = logger::severity::warning;
            else if (severity_str == "error") severity = logger::severity::error;
            else if (severity_str == "critical") severity = logger::severity::critical;
            else continue;

            if (stream["type"] == "file") {
                add_file_stream(stream["path"], severity);
            } else if (stream["type"] == "console") {
                add_console_stream(severity);
            }
        }
    } catch (const json::exception& e) {
        throw std::runtime_error("Configuration error: " + std::string(e.what()));
    }

    return *this;
}

logger_builder& server_logger_builder::clear() &
{
    _output_streams.clear();
    _format = "%m";
    _destination = "http://127.0.0.1:9200";
    return *this;
}

logger* server_logger_builder::build() const
{
    if (_output_streams.empty()) {
        throw std::runtime_error("No output streams configured");
    }

    std::string url;
    int port;

    size_t pos = _destination.rfind(':');

    if (pos != std::string::npos) {
        url = _destination.substr(0, pos);
        port = std::atoi(_destination.substr(pos+1, _destination.size()).c_str());
        if (pos+1 == _destination.size()){
            throw std::runtime_error("invalid destination");
        }
    } else {
        throw std::runtime_error("invalid destination");
    }

    return new server_logger(url, port, _format, _output_streams);
}

logger_builder& server_logger_builder::set_destination(const std::string& dest) &
{
    _destination = dest;
    return *this;
}

logger_builder& server_logger_builder::set_format(const std::string &format) &
{
    _format = format;
    return *this;
}
