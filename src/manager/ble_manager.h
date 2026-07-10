#pragma once

#include "globals.h"

// Initialize BLE radio and create a basic server
void ble_manager_init();

// Start advertising so your phone can find the watch
void ble_start_advertising();

// Check if a phone is currently connected
bool ble_is_connected();

// Ble deinitialised
void ble_manager_deinit();

void update_ble_passkey_display();