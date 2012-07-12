cmake_minimum_required(VERSION 2.8)

add_executable(rproxy cfg.c downstream.c request.c ssl.c logger.c rproxy.c
               ip_id_map.c redis_servers.c redis_server.c cfg_ip_id_map.c)
target_link_libraries(rproxy ${RPROXY_EXTERNAL_LIBS} ${SYS_LIBS})

install(TARGETS rproxy RUNTIME DESTINATION bin)
