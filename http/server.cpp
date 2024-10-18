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
#include <cstdint> // For std::int32_t, std::int64_t
#include <nlohmann/json.hpp> // For JSON parsing

#ifdef _WIN32
    #include <thread>
    #include <conio.h> // For _kbhit and _getch
    
    bool running = true;
    bool trainning_display = false;

    void checkInput() {
        while (running) {
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 'q' || ch == 'Q') {
                    running = false;
                }

                if (ch == 'd' || ch == 'D') {
                    trainning_display = !trainning_display;
                }
            }
        }
    }

#elif __linux__
    #include <thread>
    #include <atomic>
    #include <unistd.h>
    #include <fcntl.h>
    #include <termios.h>

    std::atomic<bool> running(true);
    std::atomic<bool> trainning_display(false);

    bool kbhit() {
        struct termios oldt, newt;
        int ch;
        int oldf;

        // Get the current terminal settings
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        // Disable canonical mode and echo
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        // Set stdin to non-blocking mode
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        // Check for input
        ch = getchar();

        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if(ch != EOF) {
            ungetc(ch, stdin);
            return true;
        }

        return false;
    }

    void checkInput() {
        while (running) {
            if (kbhit()) {
                char ch = getchar();
                if (ch == 'q' || ch == 'Q') {
                    running = false;
                }

                if (ch == 'd' || ch == 'D') {
                    trainning_display = !trainning_display;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay to avoid high CPU usage
        }
    }
#endif

const int PORT = 3000; // The port to listen on

// ! Example of parsing JSON data manually
/*
template <typename T>
T json_parse(std::string data, std::string key) {
    T value;
    size_t key_pos = data.find(key);

    if (key_pos != std::string::npos) {
        // Find the colon after the key
        size_t start = data.find(':', key_pos) + 1;

        // Skip any leading spaces
        while (data[start] == ' ') {
            start++;
        }

        if constexpr (std::is_same<T, std::string>::value) {
            // If the value is expected to be a string
            if (data[start] == '"') {
                size_t end = data.find('"', start + 1);
                value = data.substr(start + 1, end - start - 1);
            }
        } else if constexpr (std::is_arithmetic<T>::value) {
            // If the value is expected to be a number
            if (data[start] != '"') {
                size_t end = data.find_first_of(",}", start);
                std::string number_str = data.substr(start, end - start);
                if constexpr (std::is_integral<T>::value) {
                    value = std::stoll(number_str);  // Convert to integer type (e.g., int, long)
                } else {
                    value = std::stod(number_str);  // Convert to floating-point type (e.g., float, double)
                }
            }
        }
    }

    return value;
}

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
    std::string username;
    std::string password;

    // Parse the JSON data
    username = json_parse<std::string>(data, "username");
    password = json_parse<std::string>(data, "password");

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
*/

// ! Easy way to parse JSON data
// ! Need to install nlohmann/json or use the json.hpp file
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

    // {
    //     "username" : "Kim",
    //     "password": "1234",
    //     "data": {
    //         "dat1": [1, 2, 3],
    //         "dat2": "Hello World",
    //         "dat3": 12345,
    //         "dat4": {
    //             "sibling1":"test",
    //             "sibling2": 1234
    //         },
    //         "dat5": [
    //             "Hello",
    //             "World"
    //         ]
    //     }
    // }

    // TODO Parse the JSON data username and password
    nlohmann::json json_data = nlohmann::json::parse(data);
    std::string username = json_data["username"];
    std::string password = json_data["password"];
    std::vector<int> dat1 = json_data["data"]["dat1"];

    // * Print the parsed data
    std::cout << "Username: " << username << std::endl;
    std::cout << "Password: " << password << std::endl;
    std::cout << "Data1: ";
    for (int i : dat1) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

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

    // Start a thread to check for user input
    std::thread input_thread(checkInput);

    // Main loop to accept connections
    while (running) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            return -1;
        }
        // * If running is false, break out of the loop
        // * in handling the client is running just break handling the client
        handleClient(client_socket);
    }

    // Wait for the input thread to finish
    input_thread.join();

    // Close the server socket
    #ifdef _WIN32
    closesocket(server_fd);
    WSACleanup(); // Cleanup Winsock
    #else
    close(server_fd);
    #endif

    return 0;
}
