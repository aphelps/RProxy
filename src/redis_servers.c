
#include <stdlib.h>
#include <string.h>

#include "redis_server.h"
#include "redis_servers.h"
#include "rproxy.h"

static int
redis_servers_validates_get_cmd_tpl(const char * const tpl)
{
    const char *h;

    if ((h = strchr(tpl, '%')) == NULL) {
        return -1;
    }
    h++;
    if (*h != 's') {
        return -1;
    }
    h++;
    if (strchr(h, '%') != NULL) {
        return -1;
    }
    return 0;
}

redis_servers_t *
redis_servers_new(rproxy_t * rproxy, redis_servers_cfg_t * redis_servers)
{
    redis_servers_t    * redis_servers_c;
    redis_server_cfg_t * redis_server;
    redis_server_t     * redis_server_c;

    if (redis_servers_validates_get_cmd_tpl(redis_servers->get_cmd_tpl) != 0) {
        return NULL;
    }
    if (!(redis_servers_c = calloc(sizeof(redis_servers_t), 1))) {
        return NULL;
    }
    redis_servers_c->get_cmd_tpl = strdup(redis_servers->get_cmd_tpl);
    TAILQ_INIT(&redis_servers_c->redis_servers_c_q);
    TAILQ_FOREACH(redis_server, &redis_servers->redis_servers_q, next) {
        redis_server_c = redis_server_new(rproxy, redis_server);
        TAILQ_INSERT_TAIL(&redis_servers_c->redis_servers_c_q,
                          redis_server_c, next);
    }
    return redis_servers_c;
}


void
redis_servers_free(redis_servers_t *redis_servers_c)
{
    redis_server_t * redis_server_c;
    redis_server_t * redis_server_c_save;

    for (redis_server_c = TAILQ_FIRST(&redis_servers_c->redis_servers_c_q);
         redis_server_c; redis_server_c = redis_server_c_save) {
        redis_server_c_save = TAILQ_NEXT(redis_server_c, next);
        redis_server_free(redis_server_c);
    }
    free(redis_servers_c->get_cmd_tpl);
    free(redis_servers_c);
}

static char *
redis_servers_get_cmd_for_ip(const char * ip,
                             redis_servers_t * redis_servers_c)
{
    char * cmd_for_ip;

    asprintf(&cmd_for_ip, redis_servers_c->get_cmd_tpl, ip);

    return cmd_for_ip;
}

char *
redis_servers_get_id_from_ip(const char * ip,
                             redis_servers_t * redis_servers_c)
{
    redis_server_t * redis_server_c;
    char           * cmd_for_ip;
    char           * id;

    if (!(cmd_for_ip = redis_servers_get_cmd_for_ip(ip, redis_servers_c))) {
        return NULL;
    }
    TAILQ_FOREACH(redis_server_c, &redis_servers_c->redis_servers_c_q, next) {
        id = redis_server_send_command_get_str(redis_server_c, cmd_for_ip);
        if (id != NULL) {
            break;
        }
    }
    free(cmd_for_ip);

    return id;
}

