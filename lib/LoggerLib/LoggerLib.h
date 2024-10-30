/**
 * @file LoggerLib.h
 * @brief Logger library for logging messages to both Serial and SD card, 
 *        with features for managing and updating logs.
 */

#ifndef LOGGER_LIB
#define LOGGER_LIB

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <queue.h>  // Include for FreeRTOS queue

/**
 * @class LoggerLib
 * @brief A library for managing logging on SD cards and Serial communication, 
 *        featuring queue-based logging and support for FreeRTOS.
 */
class LoggerLib {
    public:
        /**
         * @brief Constructs a LoggerLib instance.
         * @param SD_CS SD card chip-select pin.
         * @param SD_MISO SD card MISO pin.
         * @param SD_MOSI SD card MOSI pin.
         * @param SD_SCK SD card SCK pin.
         * @param logFileName Name of the primary log file (must not be "latest_log.txt").
         */
        LoggerLib(int SD_CS = 5, int SD_MISO = 2, int SD_MOSI = 19, int SD_SCK = 18, String logFileName = "full.log");

        /**
         * @brief Initializes the logger by starting Serial communication and 
         *        initializing the SD card.
         * @param baudRate The baud rate for Serial communication.
         */
        void begin(long baudRate = 115200);

        /**
         * @brief Task for handling the logging of messages from a queue.
         * @param logFileMutex Semaphore to protect the log file access.
         * @param latestLogFileMutex Semaphore to protect the latest log file access.
         * @param pvParameters Task parameters, generally unused.
         */
        void taskLog(SemaphoreHandle_t &logFileMutex, SemaphoreHandle_t &latestLogFileMutex, void *pvParameters);

        /**
         * @brief Logs a message to both Serial and the SD card by adding it to the queue.
         * @param message The message to log.
         */
        void log(String message);

        /**
         * @brief Updates the `index.html` file with the latest logs.
         * @param logFileMutex Semaphore to protect the log file access.
         * @param logHTMLFileMutex Semaphore to protect the HTML file access.
         */
        void updateHtmlLog(SemaphoreHandle_t &logFileMutex, SemaphoreHandle_t &logHTMLFileMutex);

    private:
        int _SD_CS;       /**< SD card chip-select pin */
        int _SD_MISO;     /**< SD card MISO pin */
        int _SD_MOSI;     /**< SD card MOSI pin */
        int _SD_SCK;      /**< SD card SCK pin */
        String _logFileName; /**< Log file name on the SD card */

        QueueHandle_t logQueue; /**< Queue to hold log messages for logging task */

        /**
         * @brief Initializes the SD card and sets up logging files.
         * @return True if the SD card was initialized successfully, false otherwise.
         */
        bool _initializeSD();

        /**
         * @brief Writes a message to the SD card.
         * @param message The message to write.
         * @param sdMutex Semaphore for SD card access control.
         */
        void _writeToSD(String message, SemaphoreHandle_t &sdMutex);

        /**
         * @brief Writes the latest circular buffer log to a separate file.
         * @param logFileMutex Semaphore for log file access control.
         */
        void _writeLatestLogToSD(SemaphoreHandle_t &logFileMutex);

        /**
         * @brief Outputs a message to the Serial monitor.
         * @param message The message to print to Serial.
         */
        void _writeToSerial(String message);
};

#endif // LOGGER_LIB
