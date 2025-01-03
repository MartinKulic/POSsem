# libsll/CMakeLists.txt
add_library(sll SHARED)
target_sources(sll PRIVATE sll.c)
target_compile_options(sll PUBLIC "-std=c11")
target_compile_options(sll PRIVATE "-Wall" "-Wextra" "-Wpedantic")

