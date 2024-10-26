#include "EssentialsLib.h"

// Define the static member outside the class definition
unsigned long EssentialsLib::startTime = 0;  // Initialize to 0

EssentialsLib::EssentialsLib() {
    EssentialsLib::startTime = millis();
}

String EssentialsLib::getTimestamp() {
    unsigned long elapsedTime = millis() - startTime;

    unsigned long elapsedHours = elapsedTime / 3600000;
    unsigned long elapsedMinutes = (elapsedTime % 3600000) / 60000;
    unsigned long elapsedSeconds = (elapsedTime % 60000) / 1000;
    unsigned long elapsedMillis = elapsedTime % 1000;

    // Format the timestamp as HH:MM:SS:MS
    char timeString[20];
    sprintf(timeString, "%02lu:%02lu:%02lu:%02lu", elapsedHours, elapsedMinutes, elapsedSeconds, elapsedMillis);

    return String(timeString);
}

unsigned long EssentialsLib::getUsedHeap() {
    // Get total heap size
    unsigned long totalHeap = ESP.getHeapSize();

    // Get free heap memory
    unsigned long freeHeap = ESP.getFreeHeap();

    // Calculate used heap memory
    unsigned long usedHeap = totalHeap - freeHeap;

    return usedHeap;
}
