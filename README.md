# Multi-Launcher Extended

## Project Overview

**Multi-Launcher Extended** is a robust firmware update utility for ESP32 microcontrollers, designed to manage and perform updates from an SD card. This library allows users to upload firmware files, monitor memory usage, and log actions effectively.

### Key Features

- **Firmware Updates**: Easily update firmware from an SD card.
- **Logging**: Integrated logging capabilities to track actions and errors.
- **Heap Usage Monitoring**: Get the current heap memory usage.
- **Timestamp Generation**: Generate formatted timestamps for logging and debugging.

## Libraries

### LoaderLib
The `LoaderLib` class handles firmware updates from the SD card and integrates a logging utility.

- **Initialization**: Handles SD card initialization and checks for card availability.
- **Firmware Update**: Updates firmware from a specified file on the SD card.
- **Program Listing**: Lists all available programs stored in the `/Programs` directory.

### EssentialsLib
The `EssentialsLib` class provides essential utility functions.

- **Timestamp Generation**: Returns a formatted timestamp since the program started.
- **Heap Usage Monitoring**: Returns the amount of heap memory currently in use.

### LoggerLib
The `LoggerLib` class is a custom logging utility that provides various logging functionalities.

- **Logging Actions**: Records log messages for various operations, helping track the execution flow and errors.

## Getting Started

### Prerequisites

- **Arduino IDE**: Ensure you have the Arduino IDE installed on your machine.
- **ESP32 Board**: Add the ESP32 board support to your Arduino IDE.
- **SD Card**: Format your SD card to FAT32 and ensure it has the required firmware files.

### Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/Vankanatora/MultiLauncher-Extended.git
   cd MultiLauncher-Extended
   ```

2. **Open in Arduino IDE**:
   Open the Arduino IDE and navigate to `File` > `Open`, then select the cloned project folder.

3. **Install Required Libraries**:
   Ensure that you have the following libraries installed:
   - SD library
   - SPI library
   - Update library
   - **LoggerLib** (your custom logging library)

### Usage

1. **Connect Your SD Card**:
   Insert the SD card containing your firmware files into the appropriate slot on the ESP32.

2. **Modify Your Firmware Files**:
   Ensure that your firmware files are named correctly (preferably `firmware.bin` for ease of access) and are placed in the `/Programs` directory on the SD card.

3. **Compile and Upload**:
   Connect your ESP32 board to your computer and select the appropriate board and port in the Arduino IDE. Compile and upload the sketch.

4. **Logging Output**:
   You can view the log messages in the Serial Monitor (make sure to set the baud rate as per your configuration).

## Example Code

Here's an example of how to use `LoaderLib` in conjunction with `LoggerLib` in your sketch:

```cpp
#include <LoaderLib.h>
#include <LoggerLib.h>

LoggerLib logger(5, 2, 19, 18); // Create an instance of the logger
LoaderLib loader(5, 2, 19, 18, &logger); // Initialize LoaderLib with SD pin settings

void setup() {
    logger.begin(115200); // Initialize logger
    logger.log("Starting Multi-Launcher Extended...");

    // Initialize the SD card
    if (loader.begin()) {
        logger.log("SD Card initialized.");
    } else {
        logger.log("Failed to initialize SD Card.");
        return;
    }

    // List all available programs
    std::list<String> programs = loader.listAllPrograms();
    for (const String& program : programs) {
        logger.log("Program found: " + program);
    }

    // Perform firmware update
    loader.update("firmware.bin", [](int status) {
        if (status == 1) {
            logger.log("Firmware updated successfully!");
        } else {
            logger.log("Firmware update failed.");
        }
    });
}

void loop() {
    // Leace empty if you use RTOS
}
```

## Contributing

Contributions are welcome! If you have suggestions or improvements, feel free to create a pull request or open an issue.

## License

This project is licensed under the GPL-3.0 license. See the LICENSE file for more details.

## Acknowledgments

- [Arduino](https://www.arduino.cc/) - The open-source electronics platform.
- [ESP32](https://www.espressif.com/en/products/hardware/esp32/overview) - A powerful microcontroller for IoT applications.
- **LoggerLib** - My custom jank logging library that provides essential logging functionalities for this project.