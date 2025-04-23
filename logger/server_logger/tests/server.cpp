#include "server.h"
#include <logger_builder.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <string>

server::server(uint16_t port)
{
    CROW_ROUTE(app, "/log")
        .methods("POST"_method)
        ([this](const crow::request& req, crow::response& res) {
            std::string message = req.get_body_params().get("message");
            std::string severity_str = req.get_body_params().get("severity");
            std::string stream_type = req.get_body_params().get("stream_type");
            std::string stream_path = req.get_body_params().get("stream_path");
            std::string pid = req.get_body_params().get("sender");

            if (message.empty() || severity_str.empty() || stream_type.empty()) {
                res.code = 400;
                res.end("Missing parameters");
                return;
            }

            logger::severity severity;
            if (severity_str == "TRACE") severity = logger::severity::trace;
            else if (severity_str == "DEBUG") severity = logger::severity::debug;
            else if (severity_str == "INFORMATION") severity = logger::severity::information;
            else if (severity_str == "WARNING") severity = logger::severity::warning;
            else if (severity_str == "ERROR") severity = logger::severity::error;
            else if (severity_str == "CRITICAL") severity = logger::severity::critical;
            else {
                res.code = 400;
                res.end("Invalid severity level");
                return;
            }

            {
                //std::shared_lock<std::shared_mutex> lock(_mut);
                    std::cout << "RECIEVED LOG FROM PID" << pid << ": " << message << std::endl;
            }

            res.code = 200;
            res.end();
        });
    app.port(port).multithreaded().run_async();
}
