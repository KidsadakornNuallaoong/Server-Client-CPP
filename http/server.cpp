#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h> // For close() (Linux)
#ifdef _WIN32
#include <winsock2.h> // For Windows
#include <ws2tcpip.h> // For Windows
#else
#include <arpa/inet.h> // For Linux
#include <netinet/in.h>
#endif
// #include "json.hpp"

const int PORT = 3000; // The port to listen on

std::string json_prase (std::string data, std::string key) {
    std::string value;
    size_t password_pos = data.find(key);
    if (password_pos != std::string::npos) {
        size_t start = data.find('"', data.find(':', password_pos)) + 1;
        size_t end = data.find('"', start);
        value = data.substr(start, end - start);
    }
    return value;
}

// ! Easy way to parse JSON data
// ! Need to install nlohmann/json or use the json.hpp file
/*
void handleClient(int client_socket) {
    char buffer[1024] = {0};
    recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    // Print the received data
    // std::cout << "Received request:\n" << buffer << std::endl;

    // TODO Parse content from the request
    std::string content(buffer);
    std::string delimiter = "\r\n\r\n";
    size_t pos = content.find(delimiter);
    if (pos == std::string::npos) {
        std::cerr << "Invalid request\n";
        return;
    }
    std::string data = content.substr(pos + delimiter.length());
    // std::cout << data << std::endl;

    // TODO Parse the JSON data username and password
    nlohmann::json json = nlohmann::json::parse(data);
    std::string username = json["username"];
    std::string password = json["password"];

    // * Print the username and password
    std::cout << "Username: " << username << std::endl;
    std::cout << "Password: " << password << std::endl;

    // * Prepare a simple HTTP response
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nData received successfully!";
    
    // TODO Send the response back to the client
    send(client_socket, response.c_str(), response.length(), 0);
    
    #ifdef _WIN32
    closesocket(client_socket); // Close the client socket
    #else
    close(client_socket); // Close the client socket (Linux)
    #endif
}
*/

// ! Hardcoded way to parse JSON data
void handleClient(int client_socket) {
    char buffer[1024] = {0};

    // Receive data from the client
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        std::cerr << "Error receiving data\n";
        return;
    }

    // Null-terminate the buffer to prevent overflow
    buffer[bytes_received] = '\0';

    // Parse HTTP request and extract JSON body
    std::string content(buffer);
    std::string delimiter = "\r\n\r\n";
    size_t pos = content.find(delimiter);
    if (pos == std::string::npos) {
        std::cerr << "Invalid request, missing headers\n";
        return;
    }
    std::string data = content.substr(pos + delimiter.length());

    // Manually parse the JSON data (hardcoded parsing)
    std::string username, password;

    // Parse the JSON data
    username = json_prase(data, "username");
    password = json_prase(data, "password");

    // Print the parsed data
    std::cout << "Username: " << username << std::endl;
    std::cout << "Password: " << password << std::endl;

    // Prepare an HTTP response
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nData received successfully!";
    
    // Send the response back to the client
    send(client_socket, response.c_str(), response.length(), 0);

    // Close the client socket
    #ifdef _WIN32
    closesocket(client_socket);
    #else
    close(client_socket);
    #endif
}

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); // Initialize Winsock
    #endif

    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt))) {
        std::cerr << "setsockopt failed\n";
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return -1;
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\n";
        return -1;
    }

    std::cout << "Server is running on port " << PORT << std::endl;

    // Main loop to accept connections
    while (true) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            return -1;
        }
        handleClient(client_socket);
    }

    #ifdef _WIN32
    closesocket(server_fd); // Close the server socket
    WSACleanup(); // Clean up Winsock
    #else
    close(server_fd); // Close the server socket (Linux)
    #endif

    return 0;
}
