#include <iostream>
#include <mosquitto.h>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>  // Windows socket library
    #pragma comment(lib, "ws2_32.lib") // Link with Winsock library on Windows
#endif

const char *host = "localhost";  // Change to broker IP if needed
int port = 1883;
const char *topic = "test/topic";
const char *message = "Hello from Mosquitto MQTT Publisher!";

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

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to broker." << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }

    // Publish a message to the MQTT topic
    int ret = mosquitto_publish(mosq, nullptr, topic, strlen(message), message, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message: " << mosquitto_strerror(ret) << std::endl;
    } else {
        std::cout << "Message published to topic " << topic << std::endl;
    }

    // Disconnect from the broker
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