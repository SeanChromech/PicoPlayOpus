cmake_minimum_required(VERSION 3.12)
set(ENV{PICO_SDK_PATH} ../pico-sdk)
set(ENV{CMAKE_GENERATOR} "Unix Makefiles")
set(ENV{PICO_BOARD} "pico_w")

include(pico-sdk/external/pico_sdk_import.cmake)
include(pico-extras/external/pico_extras_import.cmake)
include(FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

include(opus/cmake/OpusFunctions.cmake)
include_directories(opus/include opus opus/celt opus/silk opus/silk/fixed ogg-data)

# Get the list of files from the makefiles.
# Add the folder prefix to each file in the lists.
get_opus_sources(SILK_HEAD opus/silk_headers.mk silk_headers_base)
foreach(silk_header ${silk_headers_base})
    list(APPEND silk_headers "opus/${silk_header}")
endforeach()

get_opus_sources(SILK_SOURCES opus/silk_sources.mk silk_sources_base)
foreach(silk_source ${silk_sources_base})
    list(APPEND silk_sources "opus/${silk_source}")
endforeach()

get_opus_sources(SILK_SOURCES_FIXED opus/silk_sources.mk silk_sources_fixed_base)
foreach(silk_sources_fixed ${silk_sources_fixed_base})
    list(APPEND silk_sources_fixed "opus/${silk_sources_fixed}")
endforeach()

get_opus_sources(OPUS_HEAD opus/opus_headers.mk opus_headers_base)
foreach(opus_header ${opus_headers_base})
    list(APPEND opus_headers "opus/${opus_header}")
endforeach()

get_opus_sources(OPUS_SOURCES opus/opus_sources.mk opus_sources_base)
foreach(opus_source ${opus_sources_base})
    list(APPEND opus_sources "opus/${opus_source}")
endforeach()

get_opus_sources(SILK_SOURCES_ARM_RTCD opus/silk_sources.mk silk_sources_arm_rtcd_base)
foreach(silk_source_arm_rtcd ${silk_sources_arm_rtcd_base})
    list(APPEND silk_sources_arm_rtcd "opus/${silk_source_arm_rtcd}")
endforeach()

get_opus_sources(CELT_HEAD opus/celt_headers.mk celt_headers_base)
foreach(celt_header ${celt_headers_base})
    list(APPEND celt_headers "opus/${celt_header}")
endforeach()

get_opus_sources(CELT_SOURCES opus/celt_sources.mk celt_sources_base)
foreach(celt_source ${celt_sources_base})
    list(APPEND celt_sources "opus/${celt_source}")
endforeach()

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
               freertos_hook.c
               ogg-data/sample.c
               opus/src/opus_decoder.c
               opus/src/opus.c
               )

add_sources_group(${PROJECT} silk ${silk_headers} ${silk_sources})
add_sources_group(${PROJECT} silk ${silk_sources_fixed})
add_sources_group(${PROJECT} celt ${celt_headers} ${celt_sources})

target_compile_definitions(${PROJECT} PUBLIC
            -DOPUS_BUILD
            -DOPUS_ARM_ASM
            -DOPUS_FIXED_POINT
            -DFIXED_POINT
            -DHAVE_LRINT
            -DUSE_ALLOCA
            -DUSE_AUDIO_I2S=1
            -DPICO_AUDIO_I2S_MONO_INPUT=1
            )

target_link_libraries(${PROJECT}
                      FreeRTOS-Kernel
                      FreeRTOS-Kernel-Heap4
                      pico_stdlib
                      pico_multicore
                      pico_bootrom
                      pico_unique_id
                      pico_cyw43_arch_none
                      pico_audio_i2s
                      hardware_dma
                      hardware_pio
                      tinyusb_device
                      )

pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

pico_add_extra_outputs(${PROJECT})
