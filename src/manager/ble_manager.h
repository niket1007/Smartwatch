#pragma once

#include "globals.h"

extern bool is_bluetooth_connected;

// Initialize BLE radio and create a basic server
void ble_manager_init();

// Start advertising so your phone can find the watch
void ble_start_advertising();

// Check if a phone is currently connected
bool ble_is_connected();

// Send Battery Percentage to GadgetBridege
// void send_battery_percentage_to_phone();

void update_ble_passkey_display();