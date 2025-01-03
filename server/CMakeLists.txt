add_executable(server)
target_sources(server PRIVATE server.c)
target_link_libraries(server PRIVATE sll com_protocol)
