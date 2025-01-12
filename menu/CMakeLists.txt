add_executable(run)
target_sources(run PRIVATE run.c)
target_link_libraries(run PRIVATE l_server l_client sll com_protocol)
