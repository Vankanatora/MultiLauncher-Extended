#include "WebServerLib.h"

WebServerLib::WebServerLib(const char* ssid, const char* password, LoggerLib* logger, LoaderLib* loader)
    : server(80), _ssid(ssid), _password(password), _logger(logger), _loader(loader) {}

void WebServerLib::begin() {
    // Connect to Wi-Fi network
    WiFi.softAP(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        _logger->log("Connecting to Wi-Fi...");
    }
    _logger->log("Connected to Wi-Fi, IP: " + String(WiFi.localIP().toString()));
    
    // Begin the server
    server.begin();
    
    //Generate the main menu for picking the new program to load.
    _generateMainMenuFile();
}

void WebServerLib::handleClient(SemaphoreHandle_t &logFileMutex, SemaphoreHandle_t &logHTMLFileMutex) {
    WiFiClient client = server.available();   // Listen for incoming clients

    if (client) {                             // If a new client connects,
        _logger->log("New Client connected.");
        // Set a timeout for reading client data
        client.setTimeout(5000); // 5 seconds timeout
        _serveHTML(client, logFileMutex, logHTMLFileMutex);                    // Serve the HTML page
        client.stop();                        // Close the connection
        _logger->log("Client disconnected.");
    }
}

void WebServerLib::_serveHTML(WiFiClient &client, SemaphoreHandle_t &latestLogFileMutex, SemaphoreHandle_t &logHTMLFileMutex) {
    String header;
    String currentLine = "";
    unsigned long startTime = millis(); // Start time for timeout check
    char lastc = client.read();

    while (client.connected() && (millis() - startTime < 5000)) { // Timeout after 5 seconds
        if (client.available()) {
            char c = client.read();
            header += c;

            if (c == '\n' && lastc == '\n') {
                _logger->log("Serving HTML.");

                // Send response headers
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println();

                // Parse the requested URL
                String fileName = _getRequestedFile(header);

                if (fileName == "/log/index.html"){_logger->log("Started creating an HTML file: " + fileName); _logger->updateHtmlLog(latestLogFileMutex, logHTMLFileMutex); _logger->log("Done creating an HTML file: " + fileName);}
                if (fileName.indexOf("load-preview") != -1) { fileName.replace("load-preview", "Programs"); fileName.replace("%20", " "); }
                if (fileName.indexOf("load-program") != -1) { fileName.replace("load-program", "Programs"); fileName.replace("%20", " "); fileName.replace("index.html", "firmware.bin"); _loader->update(fileName);}

                if (xSemaphoreTake(logHTMLFileMutex, portMAX_DELAY) == pdTRUE) {
                    // Read HTML file from SD card
                    File htmlFile = SD.open(fileName, FILE_READ);
                    if (htmlFile) {
                        _logger->log("Start reading " + fileName);
                        while (htmlFile.available()) {
                            client.write(htmlFile.read()); // Write the HTML content to the client
                        }
                        htmlFile.close(); // Make sure to close the file
                        _logger->log("Done reading " + fileName);
                    } else {
                        client.println("404: Page not found");
                        _logger->log("Error: Could not open " + fileName);
                    }
                    xSemaphoreGive(logHTMLFileMutex);
                } else {
                    client.println("403: Forbidden");
                    _logger->log("Error: Could not open " + fileName);
                }

                break; // Break out of the while loop after serving
            } else if (c != '\r') { // If you got anything else but a carriage return character,
                lastc = c;
                currentLine += c; // Add it to the end of the currentLine
            }
        }
    }
}

// Function to extract the requested file name from the HTTP header
String WebServerLib::_getRequestedFile(const String& header) {
    int startIndex = header.indexOf(' ') + 1; // Start after the GET
    int endIndex = header.indexOf(' ', startIndex); // Find the next space

    String requestedFile = header.substring(startIndex, endIndex);
    // If the requested file is just the root, return index.html
    if (requestedFile == "/") {
        return "/WebInterface/index.html";
    }

    // Check if the requested file has an extension by looking for a period (.)
    if (requestedFile.lastIndexOf('.') == -1) {
        // No extension found, so assume it's a directory
        return requestedFile + "/index.html";
    }

    // Return the requested file, ensuring it starts with a "/"
    return String(requestedFile);
}

// Function to extract the requested file name from the HTTP header
void WebServerLib::_generateMainMenuFile() {
    File htmlFile = SD.open("/WebInterface/index.html", FILE_WRITE); // Open file in write mode
    if (htmlFile) {
        {
            File tempFile = SD.open("/WebInterface/index-part1.html", FILE_READ); // Open file in write mode
            if (tempFile) {
                // Read content from the first part file and write to index.html
                while (tempFile.available()) {
                    htmlFile.write(tempFile.read());
                }
                tempFile.close(); // Close the first part file
                _logger->log("Successfully read from index-part1.html");
            } else {
                _logger->log("Error: Could not open index-part1.html");
            }
        }

        {
            // List all programs
            std::list<String> programs = _loader->listAllPrograms();

            // Generate the program list items
            htmlFile.println("<ul>"); // Start the unordered list
            for (const String& program : programs) {
                // Extract the directory name from the program path
                int lastSlashIndex = program.lastIndexOf('/');
                String programName = (lastSlashIndex != -1) ? program.substring(lastSlashIndex + 1) : program;

                // Construct the data-info and link; assuming your link format is defined
                String link = "load-program/" + program; // Adjust this as necessary
                String linkPreview = "load-preview/" + program; // Adjust this as necessary
                String listItem = "<li><a class=\"is-file\" href=\"#\" data-info=\"" + link + "\" onclick=\"changeIframeSrc('" + linkPreview + "', this)\">" + programName + "</a></li>";
                htmlFile.println(listItem); // Write the list item to the HTML file
            }
            htmlFile.println("</ul>"); // End the unordered list
        }

        {
            File tempFile = SD.open("/WebInterface/index-part2.html", FILE_READ); // Open file in write mode
            if (tempFile) {
                // Read content from the first part file and write to index.html
                while (tempFile.available()) {
                    htmlFile.write(tempFile.read());
                }
                tempFile.close(); // Close the first part file
                _logger->log("Successfully read from index-part2.html");
            } else {
                _logger->log("Error: Could not open index-part2.html");
            }
        }
        htmlFile.close();
        _logger->log("Successfully generated the index.html file!");
    } else {
        _logger->log("Error: Could not create index.html");
    }
}

