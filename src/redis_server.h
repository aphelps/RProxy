
#ifndef __REDIS_SERVER_H__
#define __REDIS_SERVER_H__ 1

#include <sys/queue.h>

#include <inttypes.h>

#include "rproxy.h"

typedef struct redis_server_cfg redis_server_cfg_t;
typedef struct redis_server     redis_server_t;

struct redis_server_cfg {
    char     * server;
    uint16_t   port;

    TAILQ_ENTRY(redis_server_cfg) next;
};

struct redis_server {
    struct redisContext * redis_context;
    rproxy_t * rproxy;
    _Bool connected : 1;

    TAILQ_ENTRY(redis_server) next;
};

redis_server_t * redis_server_new(rproxy_t *, redis_server_cfg_t *);
void redis_server_free(redis_server_t *);
char *redis_server_send_command_get_str(redis_server_t * redis_server_c,
                                        const char * cmd);
#endif
