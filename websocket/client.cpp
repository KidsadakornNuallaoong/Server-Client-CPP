#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>

namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
    WebSocketClient(asio::io_context& ioc)
        : resolver_(ioc), ws_(ioc) {}

    // Function to initiate the connection
    void start(const std::string& host, const std::string& port) {
        // Start the asynchronous resolve operation
        resolver_.async_resolve(host, port,
            beast::bind_front_handler(&WebSocketClient::on_resolve, shared_from_this()));
    }

private:
    void on_resolve(const boost::system::error_code& ec, tcp::resolver::results_type results) {
        if (ec) {
            std::cerr << "Error on resolve: " << ec.message() << "\n";
            return;
        }

        // Start the asynchronous connection using the correct signature
        asio::async_connect(ws_.next_layer(), results,
            beast::bind_front_handler(&WebSocketClient::on_connect, shared_from_this()));
    }

    void on_connect(const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
        if (ec) {
            std::cerr << "Error on connect: " << ec.message() << "\n";
            return;
        }

        // Perform the WebSocket handshake
        ws_.async_handshake("localhost", "/",
            beast::bind_front_handler(&WebSocketClient::on_handshake, shared_from_this()));
    }

    void on_handshake(const boost::system::error_code& ec) {
        if (ec) {
            std::cerr << "Error on handshake: " << ec.message() << "\n";
            return;
        }

        // Send a message
        std::string message = "Hello from client!";
        ws_.async_write(asio::buffer(message),
            beast::bind_front_handler(&WebSocketClient::on_write, shared_from_this()));
    }

    void on_write(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Error on write: " << ec.message() << "\n";
            return;
        }
        std::cout << "Sent: " << bytes_transferred << " bytes\n";
        do_read();
    }

    void do_read() {
        ws_.async_read(buffer_,
            beast::bind_front_handler(&WebSocketClient::on_read, shared_from_this()));
    }

    void on_read(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Error on read: " << ec.message() << "\n";
            return;
        }
        std::cout << "Received: " << beast::buffers_to_string(buffer_.data()) << "\n";
        buffer_.consume(bytes_transferred);
        do_read();
    }

    beast::websocket::stream<tcp::socket> ws_;
    tcp::resolver resolver_;
    beast::flat_buffer buffer_;
};

int main() {
    try {
        asio::io_context ioc;
        auto client = std::make_shared<WebSocketClient>(ioc);
        client->start("localhost", "8080");  // Start the connection
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
