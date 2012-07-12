
#ifndef __REDIS_SERVERS_H__
#define __REDIS_SERVERS_H__ 1

#include <sys/queue.h>

#include "redis_server.h"
#include "rproxy.h"

typedef struct redis_servers_cfg redis_servers_cfg_t;
typedef struct redis_servers     redis_servers_t;

struct redis_servers_cfg {
    char * get_cmd_tpl;
    TAILQ_HEAD(, redis_server_cfg) redis_servers_q;
};

struct redis_servers {
    char * get_cmd_tpl;
    TAILQ_HEAD(, redis_server) redis_servers_c_q;
};

redis_servers_t * redis_servers_new(rproxy_t *, redis_servers_cfg_t *);
void redis_servers_free(redis_servers_t *);
char * redis_servers_get_id_from_ip(const char * ip,
                                    redis_servers_t * redis_servers_c);
#endif
