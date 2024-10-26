#ifndef SD_CARD_LIB
#define SD_CARD_LIB

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <Update.h>
#include <list>
#include "LoggerLib.h" // Include the LoggerLib

/**
 * @class LoaderLib
 * @brief A class to handle firmware updates from an SD card, utilizing a logging utility.
 */
class LoaderLib {
    public:
        /**
         * @brief Constructs a LoaderLib instance and initializes SD card pins and a logger.
         * @param SD_CS Chip select pin for the SD card. Default is 5.
         * @param SD_MISO MISO pin for the SD card. Default is 2.
         * @param SD_MOSI MOSI pin for the SD card. Default is 19.
         * @param SD_SCK Clock pin for the SD card. Default is 18.
         * @param logger Pointer to a LoggerLib instance for logging. Default is nullptr.
         */
        LoaderLib(int SD_CS = 5, int SD_MISO = 2, int SD_MOSI = 19, int SD_SCK = 18, LoggerLib* logger = nullptr);

        /**
         * @brief Initializes the SD card and prepares it for file operations.
         * @return true if the SD card was successfully initialized; false otherwise.
         */
        bool begin(); 

        /**
         * @brief Starts a firmware update process from a file on the SD card.
         * @param fileName Name of the firmware file on the SD card.
         * @param completion Optional callback function to report the status of the update. Default is nullptr.
         *        The callback receives an integer status code: 1 for success, 0 for failure.
         */
        void update(String fileName, void (*completion)(int status) = nullptr);

        std::list<String> listAllPrograms();

    private:
        void (*_completion)(int status) = nullptr; ///< Completion callback

        void _updateFromFS(fs::FS &fs, String fileName); ///< Internal method to perform update from filesystem
        void _rebootEspWithReason(String reason); ///< Internal method to reboot ESP32 with a log reason
        void _performUpdate(Stream &updateSource, size_t updateSize); ///< Internal method to handle the update stream

        int _SD_CS; ///< SD card chip select pin
        int _SD_MISO; ///< SD card MISO pin
        int _SD_MOSI; ///< SD card MOSI pin
        int _SD_SCK; ///< SD card clock pin
        
        LoggerLib* _logger; ///< Pointer to LoggerLib instance for logging
};

#endif
