#add_executable(client)
#target_sources(client PRIVATE client.c)
#target_link_libraries(client PRIVATE com_protocol)

add_library(l_client)
target_sources(l_client PRIVATE client.c)
#target_link_libraries(l_client PRIVATE com_protocol)
