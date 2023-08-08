add_library(hal_radio INTERFACE)

target_include_directories(hal_radio INTERFACE ../fm_si470x ../re_Buttons ../bt_rn52)

target_sources(hal_radio INTERFACE 
   hal_main.c
   hal_main.h
   )

target_link_libraries(hal_radio INTERFACE hardware_gpio)
