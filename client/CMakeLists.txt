add_executable(client)
target_sources(client PRIVATE client.c)
target_link_libraries(client PRIVATE com_protocol)
