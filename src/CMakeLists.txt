set(COMMON_DIR ./Common)
file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

set(BATTERY_DIR ./Battery)
file(GLOB_RECURSE BATTERY_SOURCES ${BATTERY_DIR}/*.cpp)

set(LVGL_DIR ./LVGL)
file(GLOB_RECURSE LVGL_SOURCES ${LVGL_DIR}/*.cpp)

set(SCREEN_DIR ./Screen)
file(GLOB_RECURSE SCREEN_SOURCES ${SCREEN_DIR}/*.cpp ${SCREEN_DIR}/*.c)

set(BLUETOOTH_DIR ./Bluetooth)
file(GLOB_RECURSE BLUETOOTH_SOURCES ${BLUETOOTH_DIR}/*.cpp)

set(RTC_DIR ./RTC)
file(GLOB_RECURSE RTC_SOURCES ${RTC_DIR}/*.cpp)

set(WIFI_DIR ./WiFi)
file(GLOB_RECURSE WIFI_SOURCES ${WIFI_DIR}/*.cpp)

idf_component_register(
    SRCS 
        main.cpp
        ${COMMON_SOURCES}
        ${BATTERY_SOURCES}
        ${LVGL_SOURCES}
        ${SCREEN_SOURCES}
        ${BLUETOOTH_SOURCES}
        ${RTC_SOURCES}
        ${WIFI_SOURCES}

    INCLUDE_DIRS 
        .
        ${COMMON_DIR}
        ${BATTERY_DIR}
        ${LVGL_DIR}
        ${SCREEN_DIR}
        ${WIFI_DIR}
        ${RTC_DIR}
        ${BLUETOOTH_DIR}
)
