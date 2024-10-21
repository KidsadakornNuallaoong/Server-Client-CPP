#include <iostream>
#include <boost/asio.hpp>

#ifdef _WIN32
    #include <Windows.h>
#endif

using namespace boost::asio;
using namespace boost::asio::ip;

void send_http_request(const std::string& host, const std::string& port, const std::string& path) {
    io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    connect(socket, endpoint_iterator);

    // Create HTTP GET request
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n\r\n";

    // Send the request
    boost::asio::write(socket, buffer(request));

    // Receive the response
    boost::system::error_code error;
    while (true) {
        char buf[512];
        size_t len = socket.read_some(buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;  // Connection closed by peer
        else if (error)
            throw boost::system::system_error(error);  // Some other error

        std::cout.write(buf, len);
    }
}

int main() {
    send_http_request("example.com", "80", "/"); // Make a request to example.com
    return 0;
}
