#include <iostream>
#include <mosquitto.h>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>  // Windows socket library
    #pragma comment(lib, "ws2_32.lib") // Link with Winsock library on Windows
#endif

const char *host = "localhost";
int port = 1883;
const char *topic = "test/topic";

// Callback for when a message is received
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    std::cout << "Message received on topic " << message->topic << ": " 
              << (char*)message->payload << std::endl;
}

int main() {
    #ifdef _WIN32
        // Initialize Winsock on Windows
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return 1;
        }
    #endif

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosquitto *mosq = mosquitto_new(nullptr, true, nullptr);
    if (!mosq) {
        std::cerr << "Failed to create Mosquitto client instance." << std::endl;
        return 1;
    }

    // Set the message callback
    mosquitto_message_callback_set(mosq, on_message);

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to broker." << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }

    // Subscribe to the MQTT topic
    int ret = mosquitto_subscribe(mosq, nullptr, topic, 0);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to subscribe to topic: " << mosquitto_strerror(ret) << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }

    // Start the Mosquitto loop (this keeps the client connected and waiting for messages)
    mosquitto_loop_start(mosq);

    std::cout << "Subscribed to topic " << topic << ". Waiting for messages..." << std::endl;

    // Keep the subscriber running
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    // Stop the loop and disconnect
    mosquitto_loop_stop(mosq, true);
    mosquitto_disconnect(mosq);

    // Cleanup and free resources
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    #ifdef _WIN32
        // Cleanup Winsock on Windows
        WSACleanup();
    #endif

    return 0;
}