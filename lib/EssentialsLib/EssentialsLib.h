#ifndef ESSENTIALS_LIB
#define ESSENTIALS_LIB

#include <Arduino.h>

class EssentialsLib{
    public:
        EssentialsLib();
        static String getTimestamp();
        static unsigned long getUsedHeap();

    private:
        static unsigned long startTime; // Record the time when the program started
};

#endif //ESSENTIALS_LIB