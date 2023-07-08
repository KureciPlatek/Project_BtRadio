add_library(bt_rn52 INTERFACE)

#target_include_directories(bt_rn52 INTERFACE ../rdsDecoder)

target_sources(bt_rn52 INTERFACE 
   bt_rn52_application.c
   bt_rn52_application.h
   )

target_link_libraries(bt_rn52 INTERFACE pico_stdlib hardware_uart hardware_gpio)