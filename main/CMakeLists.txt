set(I2C_DIR ./I2C)
file(GLOB_RECURSE I2C_SOURCES ${I2C_DIR}/*.cpp )

set(DISPLAY_DIR ./Display)
file(GLOB_RECURSE DISPLAY_SOURCES ${DISPLAY_DIR}/*.cpp )

set(SCREEN_DIR ./Screen)
file(GLOB_RECURSE SCREEN_SOURCES ${SCREEN_DIR}/*.cpp ${SCREEN_DIR}/*.c)

set(COMMON_DIR ./Common)
file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

set(BLUETOOTH_DIR ./Bluetooth)
file(GLOB_RECURSE BLUETOOTH_SOURCES ${BLUETOOTH_DIR}/*.cpp)

set(STORAGE_DIR ./Storage)
file(GLOB_RECURSE STORAGE_SOURCES ${STORAGE_DIR}/*.cpp)

set(RTC_DIR ./RTC)
file(GLOB_RECURSE RTC_SOURCES ${RTC_DIR}/*.cpp)

set(WIFI_DIR ./WiFi)
file(GLOB_RECURSE WIFI_SOURCES ${WIFI_DIR}/*.cpp)

set(BATTERY_DIR ./Battery)
file(GLOB_RECURSE BATTERY_SOURCES ${BATTERY_DIR}/*.cpp)

set(PS_DIR ./Power_Saver)
file(GLOB_RECURSE PS_SOURCES ${PS_DIR}/*.cpp)


idf_component_register(
    SRCS 
        main.cpp
        ${I2C_SOURCES}
        ${SCREEN_SOURCES}
        ${BLUETOOTH_SOURCES}
        ${COMMON_SOURCES}
        ${STORAGE_SOURCES}
        ${DISPLAY_SOURCES}
        ${RTC_SOURCES}
        ${WIFI_SOURCES}
        ${BATTERY_SOURCES}
        ${PS_SOURCES}

    INCLUDE_DIRS 
        .
        ${I2C_DIR}
        ${SCREEN_DIR}
        ${BLUETOOTH_DIR}
        ${COMMON_DIR}
        ${STORAGE_DIR}
        ${DISPLAY_DIR}
        ${RTC_DIR}
        ${WIFI_DIR}
        ${BATTERY_DIR}
        ${PS_DIR}
)
