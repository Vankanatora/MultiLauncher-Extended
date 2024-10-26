/**
 * @file WebServerLib.h
 * @brief Web server library for serving HTML content over Wi-Fi, with integrated logging capabilities.
 */

#ifndef WEB_SERVER_LIB
#define WEB_SERVER_LIB

#include <WiFi.h>
#include <SD.h>
#include "LoggerLib.h"
#include "LoaderLib.h"

/**
 * @class WebServerLib
 * @brief Manages a Wi-Fi server to serve HTML content stored on an SD card, with logging support.
 */
class WebServerLib {
public:
    /**
     * @brief Constructs a WebServerLib instance with Wi-Fi credentials and a logger.
     * @param ssid Wi-Fi SSID for network connection.
     * @param password Wi-Fi password for network connection.
     * @param logger Pointer to an instance of LoggerLib for logging server events.
     */
    WebServerLib(const char* ssid, const char* password, LoggerLib* logger, LoaderLib* loader);

    /**
     * @brief Initializes the Wi-Fi connection and checks for the presence of the index.html file on the SD card.
     *        If the file is missing, creates a basic index.html file.
     * @param mainHTMLFileMutex Semaphore for controlling access to the main HTML file.
     */
    void begin();

    /**
     * @brief Handles incoming client connections, serving HTML files and logging the connection status.
     * @param logFileMutex Semaphore for controlling access to log files.
     * @param logHTMLFileMutex Semaphore for controlling access to the log HTML file on the SD card.
     */
    void handleClient(SemaphoreHandle_t &logFileMutex, SemaphoreHandle_t &logHTMLFileMutex);

private:
    WiFiServer server;        /**< Wi-Fi server instance */
    LoggerLib* _logger;       /**< Pointer to LoggerLib instance for logging */
    LoaderLib* _loader;       /**< Pointer to LoggerLib instance for logging */
    const char* _ssid;        /**< Wi-Fi SSID */
    const char* _password;    /**< Wi-Fi password */

    /**
     * @brief Serves the requested HTML page to the client.
     * @param client Wi-Fi client requesting the HTML page.
     * @param latestLogFileMutex Semaphore for controlling access to the latest log file.
     * @param logHTMLFileMutex Semaphore for controlling access to HTML files on the SD card.
     */
    void _serveHTML(WiFiClient &client, SemaphoreHandle_t &latestLogFileMutex, SemaphoreHandle_t &logHTMLFileMutex);

    /**
     * @brief Parses the HTTP header to extract the requested file path.
     * @param header The HTTP header containing the client's request.
     * @return The requested file path, defaulting to "/index.html" if no specific file is requested.
     */
    String _getRequestedFile(const String &header);

    /// @brief Generates an index.html for changing the software on the ESP32.
    /// @param header The HTTP header containing the client's request.
    /// @return The contents of the HTML file for the index.html file.
    void _generateMainMenuFile();
};

#endif // WEB_SERVER_LIB
