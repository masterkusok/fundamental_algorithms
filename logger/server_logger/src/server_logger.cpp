#include "../include/server_logger.h"
#include <ctime>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <fstream>

server_logger::server_logger(
    const std::string& host,
    int port,
    const std::string& format,
    const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams)
    : _host(host),
      _port(port),
      _format(format.empty() ? "[%d] [%t] [%s] %m" : format),
      _client(host.c_str(), port),
      _streams(streams)
{
    if (!_client.is_valid()) {
        throw std::runtime_error("failed to initialize HTTP client");
    }
}

server_logger::~server_logger() noexcept = default;

logger& server_logger::log(
    const std::string &message,
    logger::severity severity) &
{
    auto it = _streams.find(severity);
    if (it == _streams.end()) {
        return *this;
    }

    std::string formatted = format_message(message, severity);
    if (it->second.second) {
        std::cout << formatted << std::endl;
    } else {
        std::ofstream file(it->second.first, std::ios::app);
        if (file) {
            file << formatted << std::endl;
        }
    }


    httplib::Params params;
    params.emplace("message", formatted);
    params.emplace("severity", severity_to_string(severity));
    params.emplace("stream_type", it->second.second ? "console" : "file");
    params.emplace("stream_path", it->second.first);
    params.emplace("sender", std::to_string(server_logger::inner_getpid()));

    auto res = _client.Post("/log", params);
    if (!res || res->status != 200) {
        throw std::runtime_error("failed to send log to server");
    }

    return *this;
}

server_logger::server_logger(const server_logger &other)
    : _host(other._host),
      _port(other._port),
      _format(other._format),
      _client(other._host.c_str(), other._port),
      _streams(other._streams)
{
}

server_logger &server_logger::operator=(const server_logger &other)
{
    if (this != &other) {
        _host = other._host;
        _port = other._port;
        _format = other._format;
        _client = httplib::Client(other._host.c_str(), other._port);
        _streams = other._streams;
    }
    return *this;
}

server_logger::server_logger(server_logger &&other) noexcept
    : _host(std::move(other._host)),
      _port(other._port),
      _format(std::move(other._format)),
      _client(std::move(other._client)),
      _streams(std::move(other._streams))
{
}

server_logger &server_logger::operator=(server_logger &&other) noexcept
{
    if (this != &other) {
        _host = std::move(other._host);
        _port = other._port;
        _format = other._format;
        _client = std::move(other._client);
        _streams = std::move(other._streams);
    }
    return *this;
}

int server_logger::inner_getpid()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return getpid();
#endif
}

std::string server_logger::format_message(
    const std::string& message,
    logger::severity severity) const
{
    std::string result;
    for(int i = 0; i < _format.size(); i++) {
        if(_format[i] != '%' || i + 1 == _format.size()) {
            result += _format[i];
            continue;
        }

        switch (_format[i+1])
        {
        case 'd':
            result += logger::current_date_to_string();
            break;
        case 'm':
            result += message;
            break;
        case 's':
            result += logger::severity_to_string(severity);
            break;
        case 't':
            result += logger::current_time_to_string();
            break;
        default:
            result += _format[i];
            continue;
            break;
        }
        i++;
    }
    return result;
}
