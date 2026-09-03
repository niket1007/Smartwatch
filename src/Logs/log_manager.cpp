#include "log_manager.h"

#include <Arduino.h>

void Log::init()
{
    serial.begin(115200);
    delay(1000);
}

void Log::info(std::string tag, std::string msg)
{
    serial.println((tag + ":: " + msg).c_str());
}

void Log::error(std::string tag, std::string msg)
{
    serial.println(("ERROR:: " + tag + ":: " + msg).c_str());
}

void Log::warning(std::string tag, std::string msg)
{
    serial.println(("WARNING:: " + tag + ":: " + msg).c_str());
}