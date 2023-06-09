cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(usb_passworder C CXX ASM)
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/usb_device.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/src/MFRC522.cpp
)

target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_add_extra_outputs(${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME}
PUBLIC
pico_stdlib
hardware_spi
tinyusb_device
tinyusb_board
)

pico_enable_stdio_uart(${PROJECT_NAME} 1)
