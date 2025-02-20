# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(smart_lighting C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Habilitar suporte a Wi-Fi
set(PICO_CYW43_ARCH_THREADSAFE_BACKGROUND 1)

# Add executable. Default name is the project name, version 0.1
add_executable(smart_lighting smart_lighting.c ssd1306_i2c.c)

pico_set_program_name(smart_lighting "smart_lighting")
pico_set_program_version(smart_lighting "0.1")

# Generate PIO header
pico_generate_pio_header(smart_lighting ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(smart_lighting 0)
pico_enable_stdio_usb(smart_lighting 1)

# Add the standard library to the build
target_link_libraries(smart_lighting
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        hardware_spi
        hardware_i2c
        hardware_adc
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        )

# Add the standard include files to the build
target_include_directories(smart_lighting PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Generate additional output files
pico_add_extra_outputs(smart_lighting)