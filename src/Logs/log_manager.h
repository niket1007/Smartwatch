#pragma once

#include "HWCDC.h"
#include <string>

class Log
{
    private:
        HWCDC serial;

    public:
        void init();

        void info(std::string tag, std::string msg);
        void warning(std::string tag, std::string msg);
        void error(std::string tag, std::string msg);
};