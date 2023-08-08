cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(fm_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Add FM modules subdirectory
add_subdirectory(fm_si470x)
add_subdirectory(rdsDecoder)
include_directories(fm_si470x)
include_directories(rdsDecoder)

# Add RN52 modules subdirectory
add_subdirectory(bt_rn52)
include_directories(bt_rn52)

# add HMI subdirectory
add_subdirectory(hmi_ePaper)
include_directories(hmi_ePaper)

# add RE subdirectory
add_subdirectory(re_Buttons)
include_directories(re_Buttons)

# add HAL subdirectory
add_subdirectory(hal_radio)
include_directories(hal_radio)

# Generate executable file
add_executable(fmRadioTest main.c)

# Compilation flags
target_compile_options(fmRadioTest PRIVATE -Wall -O0 -Wextra -Wuninitialized)

# Add libraries to project
target_link_libraries(fmRadioTest fm_si470x rdsDecoder bt_rn52 hmi_ePaper re_Buttons hal_radio pico_stdlib hardware_spi pico_multicore)
#target_link_libraries(epd examples ePaper GUI Fonts Config pico_stdlib hardware_spi)

# Disable usb output, enable uart output
pico_enable_stdio_uart(fmRadioTest 1)
pico_enable_stdio_usb(fmRadioTest 0)

pico_add_extra_outputs(fmRadioTest)
