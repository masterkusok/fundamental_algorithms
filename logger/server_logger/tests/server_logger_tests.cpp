#include "server.h"
#include <server_logger_builder.h>

int main()
{
    server_logger_builder builder;

    builder.add_file_stream("a.txt", logger::severity::trace).add_file_stream("b.txt", logger::severity::debug).
            add_console_stream(logger::severity::trace).add_file_stream("a.txt", logger::severity::information).set_format("[ KRUTOE OTFORMATIROVANNOE SOOBSHENIYE: [%s: %d %t: %m ] ]").set_destination("127.0.0.1:9200");

    std::unique_ptr<logger> log(builder.build());

    log->trace("good").debug("debug");

    log->trace("IT is a very long strange message !!!!!!!!!!%%%%%%%%\tzdtjhdjh").
		information("bfldknbpxjxjvpxvjbpzjbpsjbpsjkgbpsejegpsjpegesjpvbejpvjzepvgjs");
}
