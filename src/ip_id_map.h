
#ifndef __IP_ID_MAP_H__
#define __IP_ID_MAP_H__ 1

#include "redis_servers.h"
#include "rproxy.h"

typedef struct ip_id_map_cfg     ip_id_map_cfg_t;
typedef struct ip_id_map         ip_id_map_t;

struct ip_id_map_cfg {
    char                * header_name;
    redis_servers_cfg_t * redis_servers;
};

struct ip_id_map {
    char            * header_name;
    redis_servers_t * redis_servers_c;
};

ip_id_map_t * ip_id_map_new(rproxy_t *, ip_id_map_cfg_t *);
void ip_id_map_free(ip_id_map_t *);

int ip_id_map_append_x_headers(rproxy_t * rproxy,
                               evhtp_request_t * upstream_req);

#endif
