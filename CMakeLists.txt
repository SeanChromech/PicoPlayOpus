cmake_minimum_required(VERSION 3.12)
set(ENV{PICO_SDK_PATH} ../pico-sdk)
set(ENV{CMAKE_GENERATOR} "Unix Makefiles")
set(ENV{PICO_BOARD} "pico_w")

include(pico-sdk/external/pico_sdk_import.cmake)
include(FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

set(PROJECT PicoPlayOpus)

project(${PROJECT} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR 1)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
    -Wno-format
    -Wno-unused-function
    )

# Silence some warnings in FreeRTOS.
target_compile_options(
        FreeRTOS-Kernel INTERFACE 
        -Wno-unused-variable
        -Wno-array-bounds
    )

add_executable(${PROJECT}
               main.c
               ogg_stripper.c
               usb_descriptors.c
               msc_disk.c
               freertos_hook.c
               )

target_link_libraries(${PROJECT}
                      FreeRTOS-Kernel
                      FreeRTOS-Kernel-Heap4
                      pico_stdlib
                      pico_multicore
                      pico_bootrom
                      pico_unique_id
                      pico_cyw43_arch_none
                      hardware_dma
                      hardware_pio
                      tinyusb_device
                      )

pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

pico_add_extra_outputs(${PROJECT})
