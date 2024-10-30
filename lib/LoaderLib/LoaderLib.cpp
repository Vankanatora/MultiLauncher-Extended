#include "LoaderLib.h"

// Constructor to initialize SD card and logger
LoaderLib::LoaderLib(int SD_CS, int SD_MISO, int SD_MOSI, int SD_SCK, LoggerLib* logger)
: _SD_CS(SD_CS), _SD_MISO(SD_MISO), _SD_MOSI(SD_MOSI), _SD_SCK(SD_SCK), _logger(logger) {
    _logger = logger; // Store logger reference

    _logger->log("Initializing SD card...");
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    
    if (!SD.begin(SD_CS)) {
        _logger->log("Card Mount Failed");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        _logger->log("No SD card attached");
        return;
    }

    _logger->log("SD Card initialized");
}

bool LoaderLib::begin() {
    // Initialize SPI for SD card
    SPI.begin(_SD_SCK, _SD_MISO, _SD_MOSI, _SD_CS);

    _logger->log("Initializing SD card...");
    
    if (!SD.begin(_SD_CS)) {
        _logger->log("Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        _logger->log("No SD card attached");
        return false;
    }

    _logger->log("SD Card initialized successfully");
    return true;
}

void LoaderLib::update(String fileName, void (*completion)(int status)) {
    // Store the completion callback
    _completion = completion;
    File firmware = SD.open("/" + fileName);

    // Check if fileName ends with ".bin", if not, add it
    if (!fileName.endsWith(".bin") && !fileName.endsWith(".bin/")) {
        String tempFileName = fileName + ".bin";
        // Attempt to open the firmware file with the current fileName
        firmware = SD.open("/" + tempFileName);
        _logger->log("Verifying update file " + tempFileName);
        if (firmware) {fileName = tempFileName;} 
    }

    if (!firmware) {
        // If the file doesn't exist, attempt "/firmware.bin"
        String tempFileName = fileName + "/firmware.bin";
        _logger->log("File doesn't exist, trying to search for " + tempFileName);
        firmware = SD.open(tempFileName);

        if (!firmware) {
            // If still doesn't exist, log the error and call the completion callback with a status of 0
            _logger->log("Firmware file not found");
            if (_completion) _completion(0);
            return;  // Exit if no file is found
        }else{
            fileName = tempFileName;
        }
    }

    // File is found
    _logger->log("Found firmware: " + fileName);
    _updateFromFS(SD, fileName);

    // Close the firmware file after use
    firmware.close();
}

#include <list>
#include <FS.h> // Include the FS library if not already included

std::list<String> LoaderLib::listAllPrograms() {
    std::list<String> programs; // List to store program paths

    // Open the /Programs directory
    File dir = SD.open("/Programs");
    if (!dir) {
        _logger->log("Failed to open /Programs directory");
        return programs; // Return empty list if the directory can't be opened
    }

    // Check if the opened file is a directory
    if (!dir.isDirectory()) {
        _logger->log("/Programs is not a directory");
        return programs; // Return empty list if it isn't a directory
    }

    // Iterate through the entries in the /Programs directory
    File entry = dir.openNextFile();
    while (entry) {
        if (entry.isDirectory()) {
            String programName = entry.name(); // Get the name of the directory

            // Check for index.html file in the current directory
            File indexFile = SD.open("/Programs/" + programName + "/firmware.bin");
            if (indexFile) {
                // If index.html exists, add the program path to the list
                programs.push_back(programName);
                _logger->log("Found program: " + programName);
                indexFile.close(); // Close index.html after checking
            } else {
                _logger->log("firmware.bin not found in " + programName);
            }
        }
        entry.close(); // Close the current entry before moving to the next
        entry = dir.openNextFile(); // Move to the next entry
    }

    dir.close(); // Close the /Programs directory after finishing
    return programs; // Return the list of program directories
}

// Other methods remain unchanged, just replace Serial prints with logger
void LoaderLib::_updateFromFS(fs::FS &fs, String fileName) {
    File updateBin = fs.open("/" + fileName);
    if (updateBin) {
        if (updateBin.isDirectory()) {
            _logger->log("Error, " + fileName + " is not a file");
            updateBin.close();
            return;
        }

        size_t updateSize = updateBin.size();
        if (updateSize > 0) {
            _logger->log("Trying to start update");
            _performUpdate(updateBin, updateSize);
        } else {
            _logger->log("Error, file is empty");
        }

        updateBin.close();
        _rebootEspWithReason("finished update");
    } else {
        _logger->log("Could not load " + fileName + " from sd root");
    }
}

void LoaderLib::_performUpdate(Stream &updateSource, size_t updateSize) {
    if (Update.begin(updateSize)) {      
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize) {
            _logger->log("Written : " + String(written) + " successfully");
        } else {
            _logger->log("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
            if (_completion) _completion(0);
        } 
        if (Update.end()) {
            _logger->log("OTA done!");
            if (Update.isFinished()) {
                _logger->log("Update successfully completed. Rebooting.");
            } else {
                _logger->log("Update not finished? Something went wrong!");
                if (_completion) _completion(0);
            }
        } else {
            _logger->log("Error Occurred. Error #: " + String(Update.getError()));
            if (_completion) _completion(0);
        }
    } else {
        _logger->log("Not enough space to begin OTA");
        if (_completion) _completion(0);
    }
}

void LoaderLib::_rebootEspWithReason(String reason) {
    _logger->log(reason);
    delay(1000);
    ESP.restart();
}