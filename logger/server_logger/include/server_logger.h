#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <logger.h>
#include <unordered_map>
#include <httplib.h>
#include <string>

class server_logger_builder;
class server_logger final : public logger
{
    std::string _format;
    std::string _host;
    int _port;
    httplib::Client _client;
    std::unordered_map<logger::severity, std::pair<std::string, bool>> _streams;

    server_logger(
        const std::string& host,
        int port,
        const std::string& format,
        const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams);

    friend server_logger_builder;

    static int inner_getpid();

public:
    server_logger(server_logger const &other);
    server_logger &operator=(server_logger const &other);
    server_logger(server_logger &&other) noexcept;
    server_logger &operator=(server_logger &&other) noexcept;
    ~server_logger() noexcept final;

    [[nodiscard]] logger& log(const std::string &message, logger::severity severity) & override;

private:
    std::string format_message(const std::string& message, logger::severity severity) const;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
