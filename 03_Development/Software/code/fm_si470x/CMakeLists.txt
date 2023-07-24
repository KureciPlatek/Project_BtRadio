add_library(fm_si470x INTERFACE)

target_include_directories(fm_si470x INTERFACE ../rdsDecoder ../bt_rn52)

target_sources(fm_si470x INTERFACE 
   si470x_application.c
   si470x_application.h
   si470x_comm.c
   si470x_comm.h
   si470x_driver.c
   si470x_driver.h
   si470x_driver_regs.h
   )

target_link_libraries(fm_si470x INTERFACE hardware_i2c)
