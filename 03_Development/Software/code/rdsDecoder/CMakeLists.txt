add_library(rdsDecoder INTERFACE)

#target_include_directories(rdsDecoder INTERFACE ./include)

target_sources(rdsDecoder INTERFACE rdsDecoder.c)
