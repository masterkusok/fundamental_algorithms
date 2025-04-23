#include "../include/client_logger_builder.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>

logger_builder& client_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity) &
{
    auto search = _output_streams.find(severity);
    if (search == _output_streams.end()) {
        _output_streams[severity] = std::make_pair(std::forward_list<client_logger::refcounted_stream>(), false);
    } else {
        for(auto stream : search->second.first) {
            if (stream._stream.first == stream_file_path) {
                return *this;
            }
        }
    }
    client_logger::refcounted_stream stream(stream_file_path);
    _output_streams[severity].first.push_front(stream);
    return *this;
}

logger_builder& client_logger_builder::add_console_stream(
    logger::severity severity) &
{
    auto search = _output_streams.find(severity);
    if (search == _output_streams.end()) {
        _output_streams[severity] = std::make_pair(std::forward_list<client_logger::refcounted_stream>(), true);
    }
    return *this;
}

/*
    JSON FORMAT:
    {
        ...
        "logger": {
            "format": "value",
            "streams": [
                {
                    "severity": "info"
                    "has_console": true,
                    "file_streams": [
                        "value_1",
                        "value_2",
                        ...
                    ]
                },
                ...
            ]
        }
    }
*/
logger_builder& client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    std::ifstream config(configuration_file_path);
    if (!config.is_open()) {
        throw std::runtime_error("cannot open config path: " + configuration_file_path);
    }
    nlohmann::json data = nlohmann::json::parse(config).at(configuration_path);

    if(data.contains("streams")) {
        std::vector<nlohmann::json> streams = data["streams"];
        for (auto stream_config : streams) {
            if (!stream_config.contains("severity")) {
                throw std::runtime_error("severity field is neccessary");
            }

            client_logger::severity sev;
            try {
                parse_severity(sev, stream_config["severity"]);
            } catch (std::exception e) {
                throw std::runtime_error("unknown severity");
            }

            if(stream_config.contains("has_console")) {
                bool has_console = stream_config["has_console"];
                if (has_console) {
                    add_console_stream(sev);
                }
            }

            if (stream_config.contains("file_streams")) {
                std::vector<std::string> files = stream_config["file_streams"];
                for (auto file : files) {
                    add_file_stream(file, sev);
                }
            }
        }
    }

    if (data.contains("format")) {
        _format = data["format"];
    }

    config.close();
    return *this;
}

logger_builder& client_logger_builder::clear() &
{
    _format = "";
    _output_streams.clear();
    return *this;
}

logger* client_logger_builder::build() const
{
    return new client_logger(_output_streams, _format);
}

logger_builder& client_logger_builder::set_format(std::string const &format) &
{
    _format = format;
    return *this;
}

void client_logger_builder::parse_severity(logger::severity sev, nlohmann::json& j)
{
    std::string string_severity = j;
    if (string_severity == "info") {
        sev = logger::severity::information;
    } else if(string_severity == "debug") {
        sev = logger::severity::debug;
    } else if(string_severity == "trace") {
        sev = logger::severity::trace;
    } else if(string_severity == "warning") {
        sev = logger::severity::warning;
    } else if(string_severity == "critical") {
        sev = logger::severity::critical;
    } else if(string_severity == "error") {
        sev = logger::severity::error;
    } else {
        throw std::runtime_error("unknown severity");
    }
}

logger_builder& client_logger_builder::set_destination(std::string const& format) &
{
}
