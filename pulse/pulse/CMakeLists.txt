# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(PICO_TINYUSB_DEVICE 1)

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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_PLATFORM rp2040 CACHE STRING "Pico platform")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(${PICO_SDK_INIT_CMAKE_FILE})


project(pulse C CXX ASM)


# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1

add_executable( pulse main.c 
                src/actuators/brakes.c
                src/actuators/inverter_rs485.c
                src/actuators/vesc6.c
                src/actuators/blinker.c
                src/actuators/power_cut.c

                #src/cores/core0.c
                src/cores/core1.c  

                src/sensors/mpu9250.c
                #src/sensors/optics.c
                src/sensors/encoder.c
                src/sensors/navigation.c
                src/sensors/ninebot.c

                # src/communication.c
                src/control/communication.c
                src/control/control.c
                src/shared_data.c
                src/control/ring_buffer.c
                src/control/usb_comm.c
                src/control/commands.c )

target_compile_definitions(pulse PRIVATE
        PICO_STDIO_USB=0
        CFG_TUSB_MCU=OPT_MCU_RP2040
        CFG_TUSB_RHPORT0_MODE=OPT_MODE_DEVICE
        CFG_TUD_CDC=1
        CFG_TUD_CDC_RX_BUFSIZE=256
        CFG_TUD_CDC_TX_BUFSIZE=256
)

pico_set_program_name(pulse "pulse")
pico_set_program_version(pulse "0.1")

# Modify the below lines to enable/disable output over UART/USB

pico_enable_stdio_usb(pulse 1)
pico_enable_stdio_uart(pulse 0)

pico_generate_pio_header(pulse ${CMAKE_CURRENT_LIST_DIR}/src/assembly/encoder_read.pio)


# Add the standard library to the build
target_link_libraries(pulse
        pico_stdlib pico_multicore)

# Add the standard include files to the build
target_include_directories(pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_include_directories(pulse PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

# Add any user requested libraries
target_link_libraries(pulse 
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_timer
        hardware_adc
        hardware_sync
        hardware_pwm
        hardware_pio
        hardware_irq
        
        )



pico_add_extra_outputs(pulse)
