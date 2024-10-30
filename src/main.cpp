#include <header.h>

// Create instances of the libraries
LoggerLib logger(CS_PIN, MISO_PIN, MOSI_PIN, CLK_PIN);
LoaderLib loader(CS_PIN, MISO_PIN, MOSI_PIN, CLK_PIN, &logger);
WebServerLib webServer("ESP32", "password", &logger, &loader); //Set AP name and password

SemaphoreHandle_t logFileMutex;
SemaphoreHandle_t latestLogFileMutex;
SemaphoreHandle_t logHTMLFileMutex;

// Task to handle web server requests
void taskHandleWebServer(void *pvParameters) {
    while (true) {
        webServer.handleClient(latestLogFileMutex, logHTMLFileMutex); // Handle web server requests
        vTaskDelay(pdMS_TO_TICKS(100 / portTICK_PERIOD_MS)); // Short delay to prevent blocking
    }
}

// Static wrapper to call the non-static taskLog method
void taskHandleLoggingWrapper(void *pvParameters) {
    LoggerLib *logger = static_cast<LoggerLib *>(pvParameters);
    logger->taskLog(logFileMutex, latestLogFileMutex, &pvParameters); // Call the non-static member function
    vTaskDelay(pdMS_TO_TICKS(1000 / portTICK_PERIOD_MS)); // Short delay to prevent blocking
}

void setup() {
    logFileMutex = xSemaphoreCreateMutex();
    latestLogFileMutex = xSemaphoreCreateMutex();
    logHTMLFileMutex = xSemaphoreCreateMutex();
    logger.begin(115200); // Initialize logger

    // Initialize the SD card
    if (loader.begin()) {
        logger.log("SD Card initialized.");
    } else {
        logger.log("Failed to initialize SD Card.");
        return;
    }

    webServer.begin();

    // Create tasks for HTML log updating and web server handling
    xTaskCreate(taskHandleWebServer, "HandleWebServer", 8192, NULL, 2, NULL);
    // Create the logging task
    xTaskCreate(taskHandleLoggingWrapper, "HandleLogging", 10240, &logger, 1, NULL);}

void loop() {
    // Do nothing here, everything is handled by tasks
}
