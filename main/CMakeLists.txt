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


idf_component_register(
    SRCS 
        main.cpp 
        ${SCREEN_SOURCES}
        ${BLUETOOTH_SOURCES}
        ${COMMON_SOURCES}
        ${STORAGE_SOURCES}
        ${DISPLAY_SOURCES}

    INCLUDE_DIRS 
        .
        ${SCREEN_DIR}
        ${BLUETOOTH_DIR}
        ${COMMON_DIR}
        ${STORAGE_DIR}
        ${DISPLAY_DIR}
)
