
#include <stdlib.h>
#include <string.h>

#include <hiredis/hiredis.h>
#include <hiredis/adapters/libevent.h>

#include "redis_server.h"
#include "rproxy.h"

redis_server_t *
redis_server_new(rproxy_t * rproxy, redis_server_cfg_t *redis_server)
{
    redis_server_t * redis_server_c;

    (void) rproxy;
    if (!(redis_server_c = calloc(sizeof(redis_server_t), 1))) {
        return NULL;
    }
    redis_server_c->rproxy = rproxy;
    redis_server_c->redis_context =
        redisConnect(redis_server->server, redis_server->port);
    if (redis_server_c->redis_context == NULL ||
        redis_server_c->redis_context->err != REDIS_OK) {
        redis_server_free(redis_server_c);
        return NULL;
    }
    redis_server_c->connected = 1;

    return redis_server_c;
}

void
redis_server_free(redis_server_t * redis_server_c)
{
    if (redis_server_c->redis_context != NULL) {
        redisFree(redis_server_c->redis_context);
    }
    free(redis_server_c);
}

char *
redis_server_send_command_get_str(redis_server_t * redis_server_c,
                                  const char * cmd)
{
    redisReply * reply;
    char       * id;

    if (redis_server_c == NULL ||
        redis_server_c->redis_context == NULL ||
        redis_server_c->connected == 0) {
        return NULL;
    }
    if (strchr(cmd, '%') != NULL) {
        return NULL;
    }
    reply = redisCommand(redis_server_c->redis_context, cmd);
    if (reply == NULL || reply->str == NULL) {
        return NULL;
    }
    id = strdup(reply->str);

    return id;
}

