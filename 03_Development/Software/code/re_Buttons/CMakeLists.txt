add_library(re_Buttons INTERFACE)

aux_source_directory(. DIR_reButtons_SRCS)
include_directories(re_Buttons)

target_sources(re_Buttons INTERFACE 
   ${DIR_reButtons_SRCS}
   )


target_link_libraries(re_Buttons INTERFACE pico_stdlib hardware_spi)
