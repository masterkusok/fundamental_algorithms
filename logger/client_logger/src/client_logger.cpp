#include <string>
#include <sstream>
#include <algorithm>
#include <utility>
#include "../include/client_logger.h"
#include <not_implemented.h>

std::unordered_map<std::string, std::pair<size_t, std::ofstream>> client_logger::refcounted_stream::_global_streams;

logger& client_logger::log(const std::string& message, logger::severity severity) &
{
    std::string formated_message = make_format(message, severity);

    auto [target_streams, has_console] = _output_streams[severity];

    if (has_console) {
        std::clog << formated_message << std::endl;
    }

    for(auto stream : target_streams) {
        if (stream._stream.second == nullptr || !stream._stream.second->is_open()) {
            throw std::runtime_error("try to log in not opened file");
        }
        *stream._stream.second << formated_message << std::endl;
    }

    return *this;
}

std::string client_logger::make_format(const std::string& message, severity sev) const
{
    std::string result;
    for(int i = 0; i < message.size(); i++) {
        if(message[i] != '%' || i + 1 == message.size()) {
            result += message[i];
            continue;
        }

        switch (char_to_flag(message[i+1]))
        {
        case flag::DATE:
            result += logger::current_date_to_string();
            break;
        case flag::MESSAGE:
            result += message;
            break;
        case flag::SEVERITY:
            result += logger::severity_to_string(sev);
            break;
        case flag::TIME:
            result += logger::current_time_to_string();
            break;
        default:
            result += message[i];
            continue;
            break;
        }
        i++;
    }
    return result;
}

client_logger::client_logger(
    const std::unordered_map<logger::severity, std::pair<std::forward_list<refcounted_stream>, bool>>& streams,
    std::string format)
    : _output_streams(streams), _format(format)
{
    const severity all_levels[] = {
        severity::trace,
        severity::debug,
        severity::information,
        severity::warning,
        severity::error,
        severity::critical
    };

    for(auto severity : all_levels) {
        auto search = _output_streams.find(severity);
        if (search != _output_streams.end()) {
            continue;
        }
        _output_streams[severity] = std::make_pair(std::forward_list<refcounted_stream>(), false);
    }
}

client_logger::flag client_logger::char_to_flag(char c) noexcept
{
    switch (c)
    {
    case 'd':
        return flag::DATE;
    case 't':
        return flag::TIME;
    case 's':
        return flag::SEVERITY;
    case 'm':
        return flag::MESSAGE;
    default:
        return flag::NO_FLAG;
    }
}

client_logger::client_logger(const client_logger& other)
    : _output_streams(other._output_streams), _format(other._format)
{
    for(auto [severity, streams] : _output_streams) {
        for (auto stream : streams.first){
            auto search = refcounted_stream::_global_streams.find(stream._stream.first);
            if (search == refcounted_stream::_global_streams.end()) {
                throw std::runtime_error("invalid state in other logger");
            }
            search->second.first++;
        }
    }
}

void client_logger::clear_streams() {
    for(auto [severity, streams] : _output_streams) {
        for (auto stream : streams.first){
            auto search = refcounted_stream::_global_streams.find(stream._stream.first);
            if (search == refcounted_stream::_global_streams.end()) {
                throw std::runtime_error("invalid state in logger");
            }
            if (--search->second.first == 0) {
                search->second.second.close();
                refcounted_stream::_global_streams.erase(search);
            }
        }
    }
}

client_logger& client_logger::operator=(const client_logger& other)
{
    if (this != &other) {
        clear_streams();
        _output_streams = other._output_streams;
        _format = other._format;
        for(auto [severity, streams] : _output_streams) {
            for (auto stream : streams.first){
                auto search = refcounted_stream::_global_streams.find(stream._stream.first);
                if (search == refcounted_stream::_global_streams.end()) {
                    throw std::runtime_error("invalid state in logger");
                }
                search->second.first++;
            }
        }
    }
    return *this;
}

client_logger::client_logger(client_logger&& other) noexcept
    : _output_streams(std::move(other._output_streams)),
      _format(std::move(other._format))
{}

client_logger& client_logger::operator=(client_logger&& other) noexcept
{
    if (this != &other) {
        clear_streams();
        _output_streams = std::move(other._output_streams);
        _format = std::move(other._format);
    }
    return *this;
}

client_logger::~client_logger() noexcept {
    clear_streams();
}

client_logger::refcounted_stream::refcounted_stream(const std::string &path): _stream{path, nullptr}{
    open();
}

void client_logger::refcounted_stream::open()
{
    if (_stream.second != nullptr) {
        return;
    }

    auto search = _global_streams.find(_stream.first);
    if (search != _global_streams.end()) {
        search->second.first++;
        _stream.second = &search->second.second;
        return;
    }

    _global_streams[_stream.first].second.open(_stream.first, std::ios::app);
    if (!_global_streams[_stream.first].second.is_open()) {
         throw std::runtime_error("failed to open file");
    }
    _global_streams[_stream.first].first = 1;
    _stream.second = &_global_streams[_stream.first].second;
}

client_logger::refcounted_stream::refcounted_stream(const client_logger::refcounted_stream &oth) : _stream{oth._stream.first, oth._stream.second}
{
    if (this == &oth) {
        return;
    }
    auto search = _global_streams.find(oth._stream.first);
    if (search == _global_streams.end()) {
        throw std::runtime_error("stream not found in global map");
    }
    search->second.first++;
}

client_logger::refcounted_stream& client_logger::refcounted_stream::operator=(const client_logger::refcounted_stream &oth)
{
    if (this != &oth) {
        auto search = _global_streams.find(_stream.first);
        if (search != _global_streams.end()) {
            if (--search->second.first == 0) {
                search->second.second.close();
                _global_streams.erase(search);
            }
        }

        search = _global_streams.find(oth._stream.first);
        if (search == _global_streams.end()) {
            throw std::runtime_error("stream not found in global map");
        }
        search->second.first++;

        _stream.first = oth._stream.first;
        _stream.second = oth._stream.second;
    }
    return *this;
}

client_logger::refcounted_stream::refcounted_stream(refcounted_stream&& oth) noexcept
    : _stream{std::move(oth._stream.first), oth._stream.second}
{
    if (this == &oth) {
        return;
    }
    oth._stream.second = nullptr;
}

client_logger::refcounted_stream& client_logger::refcounted_stream::operator=(refcounted_stream&& oth) noexcept
{
    if (this != &oth) {
        if (_stream.second != nullptr) {
            auto search = _global_streams.find(_stream.first);
            if (search != _global_streams.end()) {
                if (--search->second.first == 0) {
                    search->second.second.close();
                    _global_streams.erase(search);
                }
            }
        }

        _stream.first = std::move(oth._stream.first);
        _stream.second = oth._stream.second;
        oth._stream.second = nullptr;
    }
    return *this;
}

client_logger::refcounted_stream::~refcounted_stream()
{
    if (_stream.second != nullptr) {
        auto search = _global_streams.find(_stream.first);
        if (search != _global_streams.end()) {
            if (--search->second.first == 0) {
                search->second.second.close();
                _global_streams.erase(search);
            }
        }
    }
}
