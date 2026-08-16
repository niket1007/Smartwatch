#------------------------------------------------------------------------------
# Source directories
#------------------------------------------------------------------------------

set(DISPLAY_DIR ./Display)
file(GLOB_RECURSE DISPLAY_SRCS
    ${DISPLAY_DIR}/*.cpp
    ${DISPLAY_DIR}/*.c
)

set(SCREEN_DIR ./Screen)
file(GLOB_RECURSE SCREEN_SRCS 
    ${SCREEN_DIR}/*.cpp
)

set(GRAPHICS_DIR ./Graphics)
file(GLOB_RECURSE GRAPHICS_SRCS 
    ${GRAPHICS_DIR}/*.cpp 
    ${GRAPHICS_DIR}/*.c
)

set(BATTERY_DIR ./Battery)
file(GLOB_RECURSE BATTERY_SRCS 
    ${BATTERY_DIR}/*.cpp
)

set(I2C_DIR ./I2C)
file(GLOB_RECURSE I2C_SRCS 
    ${I2C_DIR}/*.cpp
)

set(STORAGE_DIR ./Storage)
file(GLOB_RECURSE STORAGE_SRCS 
    ${STORAGE_DIR}/*.cpp
)

set(WIFI_DIR ./WiFi)
file(GLOB_RECURSE WIFI_SRCS 
    ${WIFI_DIR}/*.cpp
)

set(RTC_DIR ./RTC)
file(GLOB_RECURSE RTC_SRCS
    ${RTC_DIR}/*.cpp
)

set(TOUCH_DIR ./Touch)
file(GLOB_RECURSE TOUCH_SRCS
    ${TOUCH_DIR}/*.cpp
)

set(BLE_DIR ./Bluetooth)
file(GLOB_RECURSE BLE_SRCS
    ${BLE_DIR}/*.cpp
)

set(PS_DIR ./Power_Saver)
file(GLOB_RECURSE PS_SRCS
    ${PS_DIR}/*.cpp
)

#------------------------------------------------------------------------------
# Component
#------------------------------------------------------------------------------

idf_component_register(
    SRCS 
        "main.cpp"
        "Common/globals.cpp"
        ${DISPLAY_SRCS}
        ${GRAPHICS_SRCS}
        ${SCREEN_SRCS}
        ${BATTERY_SRCS}
        ${I2C_SRCS}
        ${STORAGE_SRCS}
        ${WIFI_SRCS}
        ${RTC_SRCS}
        ${TOUCH_SRCS}
        ${BLE_SRCS}
        ${PS_SRCS}

    INCLUDE_DIRS 
        "."
        ${DISPLAY_DIR}
        "Common"
        ${GRAPHICS_DIR}
        ${SCREEN_DIR}
        ${BATTERY_DIR}
        ${I2C_DIR}
        ${STORAGE_DIR}
        ${WIFI_DIR}
        ${RTC_DIR}
        ${TOUCH_DIR}
        ${BLE_DIR}
        ${PS_DIR}
)
