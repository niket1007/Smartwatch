#pragma once

#include "Preferences.h"
#include <array>
#include <string>

class StorageManager
{
    private: 
        Preferences preferences;
    
    public:
        void store_wifi_credentials();
        std::array<std::string, 2> fetch_wifi_credentials();
};