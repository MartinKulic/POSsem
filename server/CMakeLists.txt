#add_executable(server)
#target_sources(server PRIVATE server.c map.c)
#target_link_libraries(server PRIVATE sll com_protocol)

add_library(l_server)
target_sources(l_server PRIVATE server.c map.c)
#target_link_libraries\(l_server PRIVATE sll com_protocol)
