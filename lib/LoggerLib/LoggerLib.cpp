#include "LoggerLib.h"
#include <EssentialsLib.h>
#include <vector>

#define LOG_MESSAGE_SIZE 1024
#define LOG_MESSAGE_SIZE_HTML 1024
#define MAX_LOG_DISPLAY_SIZE_HTML 4096 
#define MAX_LOG_MESSAGES 100 
#define LATEST_LOG "/log/latest.log" 

// Add this member to the LoggerLib class
QueueHandle_t logQueue;

String logMessages[MAX_LOG_MESSAGES];
int logIndex = 0;  // Points to the current position in the circular buffer
bool bufferFull = false;  // Indicates if the buffer has wrapped around

// Constructor to set the SD card pins and log file name
LoggerLib::LoggerLib(int SD_CS, int SD_MISO, int SD_MOSI, int SD_SCK, String logFileName) {
    _SD_CS = SD_CS;
    _SD_MISO = SD_MISO;
    _SD_MOSI = SD_MOSI;
    _SD_SCK = SD_SCK;
    _logFileName = "/log/"+logFileName;

    // Create a queue for logging messages
    logQueue = xQueueCreate(100, LOG_MESSAGE_SIZE); // Queue for storing log messages
}

// Begin method to initialize Serial and SD card
void LoggerLib::begin(long baudRate) {
    // Start Serial communication
    Serial.begin(baudRate);
    while (!Serial) {
        // Wait for Serial to be ready (on boards that need it)
    }

    // Print a message indicating Serial is ready
    Serial.println("Logger started. Initializing SD card...");

    // Initialize SD card
    if (!_initializeSD()) {
        Serial.println("Failed to initialize SD card.");
    } else {
        Serial.println("SD card initialized.");
    }
}

void LoggerLib::taskLog(SemaphoreHandle_t &logFileMutex, SemaphoreHandle_t &latestLogFileMutex, void *pvParameters) {
    char message[LOG_MESSAGE_SIZE];
    while (true) {
        if (xQueueReceive(logQueue, &message, portMAX_DELAY) == pdTRUE) {
            String logMessage = String(message);  // Convert the received message back to String
            
            // Write log to Serial and SD card
            _writeToSerial(logMessage);
            _writeToSD(logMessage, logFileMutex);

            // Store the log message in the circular buffer
            logMessages[logIndex] = logMessage;
            logIndex = (logIndex + 1) % MAX_LOG_MESSAGES;  // Update the circular buffer index
            
            // Optional: Check if the buffer has wrapped around
            if (logIndex == 0) {
                bufferFull = true;
            }

            // Write the circular buffer (latest log) to a separate file
            _writeLatestLogToSD(latestLogFileMutex);
        }
    }
}

// Method to log messages to both Serial and SD card
void LoggerLib::log(String message) {
    String timestamp = EssentialsLib::getTimestamp();
    String logMessage = "[" + timestamp + "] " + message;

    // Ensure the message fits within the buffer size, including space for the null terminator
    if (logMessage.length() >= LOG_MESSAGE_SIZE - 1) {
        logMessage = logMessage.substring(0, LOG_MESSAGE_SIZE - 2); // Truncate if too long
    }

    // Create a char array for the log message
    char logBuffer[LOG_MESSAGE_SIZE];
    
    // Convert String to char array and ensure proper null termination
    logMessage.toCharArray(logBuffer, LOG_MESSAGE_SIZE);

    // Send the message to the logging task
    if (xQueueSend(logQueue, logBuffer, portMAX_DELAY) != pdPASS) {
        Serial.println("Failed to send log to queue.");
    }
}

// Internal method to initialize the SD card
bool LoggerLib::_initializeSD() {
    // Begin SPI communication with custom pins for SD card
    SPI.begin(_SD_SCK, _SD_MISO, _SD_MOSI, _SD_CS);

    // Initialize SD card
    if (!SD.begin(_SD_CS)) {
        Serial.println("Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }

    // Check for existing log files and determine the next log number
    int logNumber = 1; // Start with log_1
    String existingLogName;
    
    // Loop through all files on the SD card
    File root = SD.open("/Old logs");
    File file = root.openNextFile();
    
    while (file) {
        String fileName = file.name();
        
        // Check if the file name matches the log pattern
        if (fileName.startsWith("log_")) {
            // Extract the number from the filename
            int numberIndex = fileName.indexOf('_') + 1;
            int numberEndIndex = fileName.indexOf('.', numberIndex);
            if (numberEndIndex == -1) numberEndIndex = fileName.length(); // If there's no dot, get to the end of the name

            // Parse the number and update logNumber if it's larger
            int fileLogNumber = fileName.substring(numberIndex, numberEndIndex).toInt();
            if (fileLogNumber >= logNumber) {
                logNumber = fileLogNumber + 1; // Increment for the next available log number
            }
        }
        
        file.close();
        file = root.openNextFile(); // Continue to the next file
    }
    
    // Rename the existing log file to "log_#"
    if (SD.exists(_logFileName)) {
        existingLogName = _logFileName; // Store the current log file name
        String newLogFileName = "/Old logs/log_" + String(logNumber) + ".txt"; // Create new log file name

        // Open the existing log file for reading
        File sourceFile = SD.open(_logFileName, FILE_READ);
        if (!sourceFile) {
            Serial.println("Failed to open source log file.");
        }

        // Create the new log file for writing
        File backupFile = SD.open(newLogFileName, FILE_WRITE);
        if (!backupFile) {
            Serial.println("Failed to create backup log file.");
            sourceFile.close();
        }

        Serial.println("Copying data to " + newLogFileName + ", from " + existingLogName);

        while (sourceFile.available())
        {
            backupFile.write(sourceFile.read());
        }

        // Close both files after copying
        sourceFile.close();
        backupFile.close();
        Serial.println("Finished copying to the new log file.");
    }

    // Create a new log file
    File logFile = SD.open(_logFileName, FILE_WRITE, true); // Open the new log file for writing
    if (logFile) {
        logFile.close();  // Just open and close to create the file
        Serial.println("New log file created.");
    } else {
        Serial.println("Failed to open new log file.");
    }

    return true;
}

// Simplified method to append log messages directly to SD card
void LoggerLib::_writeToSD(String message, SemaphoreHandle_t &logFileMutex) {
    if (xSemaphoreTake(logFileMutex, portMAX_DELAY) == pdTRUE) {
        File logFile = SD.open(_logFileName, FILE_APPEND);
        if (logFile) {
            logFile.println(message);
            logFile.flush();
            logFile.close();
        } else {
            Serial.println("Failed to open log file.");
        }
        xSemaphoreGive(logFileMutex);  // Release the mutex after writing
    } else {
        Serial.println("Failed to acquire SD mutex.");
    }
}

void LoggerLib::_writeLatestLogToSD(SemaphoreHandle_t &latestLogFileMutex) {
    if (xSemaphoreTake(latestLogFileMutex, portMAX_DELAY) == pdTRUE) {
        File logFile = SD.open("/log/latest.log", FILE_WRITE, true);
        if (logFile) {
            logFile.seek(0);  // Overwrite the entire file
            for (int i = 0; i < MAX_LOG_MESSAGES; i++) {
                int idx = (logIndex + i) % MAX_LOG_MESSAGES;  // Circular buffer indexing
                if (bufferFull || idx < logIndex) {
                    logFile.println(logMessages[idx]);  // Write message to SD
                }
            }
            logFile.flush();
            logFile.close();
        } else {
            Serial.println("Failed to open latest_log.txt for writing.");
        }
        xSemaphoreGive(latestLogFileMutex);
    }
}

// Internal method to print log messages to Serial
void LoggerLib::_writeToSerial(String message) {
    Serial.println(message);
}

// Function to update the index.html file with the latest log
void LoggerLib::updateHtmlLog(SemaphoreHandle_t &latestLogFileMutex, SemaphoreHandle_t &logHTMLFileMutex) {
    if (xSemaphoreTake(latestLogFileMutex, portMAX_DELAY) == pdTRUE) {
        File logFile = SD.open("/log/latest.log");
        if (!logFile) {
            log("Failed to open latest_log.txt for reading");
            xSemaphoreGive(latestLogFileMutex);
            return;
        }

        String htmlContent = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bulma@1.0.2/css/bulma.min.css\">";
        htmlContent += "<pre>";

        while (logFile.available()) {
            String logLine = logFile.readStringUntil('\n');
            htmlContent += logLine + "<br>";
        }

        logFile.close();  // Close the log file
        xSemaphoreGive(latestLogFileMutex);  // Release the mutex

        htmlContent += "</pre></body></html>";

        if (xSemaphoreTake(logHTMLFileMutex, portMAX_DELAY) == pdTRUE) {
            File htmlFile = SD.open("/log/index.html", FILE_WRITE);
            if (!htmlFile) {
                log("Failed to open /log/index.html for writing");
                return;
            }

            htmlFile.print(htmlContent);  // Write the HTML content
            htmlFile.close();  // Close the HTML file
            xSemaphoreGive(logHTMLFileMutex);  // Release the mutex
        }
    }
}
