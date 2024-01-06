add_executable(spy
        spy.cc
        display/display.cc
        can/can2040.c
        )

# pull in common dependencies
target_link_libraries(spy 
        hardware_i2c
		pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(spy)

# add url via pico_set_program_url
example_auto_set_url(spy)

# Enable stdio over USB
pico_enable_stdio_usb(spy 1)
pico_enable_stdio_uart(spy 0)
