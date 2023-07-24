# Find all source files in a single current directory
# Save the name to DIR_examples_SRCS

# aux_source_directory(. DIR_examples_SRCS)
# 
# include_directories(./hmi_ePaper/lib/Config)
# include_directories(./hmi_ePaper/lib/GUI)
# include_directories(./hmi_ePaper/lib/Fonts)
# include_directories(./hmi_ePaper/lib/e-Paper)
# 
# # Generate the link library
# add_library(hmi_ePaper ${DIR_examples_SRCS})
# target_link_libraries(hmi_ePaper PUBLIC Config GUI Fonts ePaper)
# 
# 
# 
# aux_source_directory(. DIR_Config_SRCS)
# 
# # Generate the link library
# add_library(Config ${DIR_Config_SRCS})
# target_link_libraries(Config PUBLIC pico_stdlib hardware_spi)

#######################################################################
add_library(hmi_ePaper INTERFACE)

aux_source_directory(. DIR_hmi_ePaper_SRCS)
include_directories(hmi_ePaper)
#target_include_directories(bt_rn52 INTERFACE ../rdsDecoder)

target_sources(hmi_ePaper INTERFACE 
   ${DIR_hmi_ePaper_SRCS}
   )

#   DEV_Config.c
#   DEV_Config.h
#   ep_application.c
#   ep_application.h
#   EPD_5in83_V2.c
#   EPD_5in83_V2.h
#   font12.c
#   font16.c
#   font20.c
#   font24.c
#   font8.c
#   fonts.h
#   ImageData.c
#   ImageData.h

target_link_libraries(hmi_ePaper INTERFACE pico_stdlib hardware_spi)
