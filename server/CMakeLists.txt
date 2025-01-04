add_executable(server)
target_sources(server PRIVATE server.c player.c)
target_link_libraries(server PRIVATE sll com_protocol)
