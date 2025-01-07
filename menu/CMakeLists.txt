add_executable(run)
target_sources(run PRIVATE run.c)
target_link_libraries(run PRIVATE sll com_protocol l_server l_client)
