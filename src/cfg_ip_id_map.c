
#define DEFINE_GLOBALS_FOR_CFG_IP_ID_MAP 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "rproxy.h"
#include "cfg_ip_id_map.h"
#include "ip_id_map.h"

static cfg_opt_t redis_server_opts[] = {
    CFG_BOOL("enabled", cfg_true, CFGF_NONE),
    CFG_STR("server", "localhost", CFGF_NONE),
    CFG_INT("port", 6379, CFGF_NONE),
    CFG_END()
};

static cfg_opt_t redis_servers_opts[] = {
    CFG_STR("get-cmd-tpl", "GET rproxy-ip-id:%s", CFGF_NONE),
    CFG_SEC("redis-server", redis_server_opts, CFGF_MULTI),
    CFG_END()
};

cfg_opt_t ip_id_map_opts[] = {
    CFG_BOOL("enabled", cfg_false, CFGF_NONE),
    CFG_STR("header-name", "X-User-ID", CFGF_NONE),
    CFG_STR("type", "redis", CFGF_NONE),
    CFG_SEC("redis-servers", redis_servers_opts, CFGF_NONE),
    CFG_END()
};

redis_server_cfg_t *
redis_server_cfg_new(void) {
    return calloc(sizeof(redis_server_cfg_t), 1);
}

redis_server_cfg_t *
redis_server_cfg_parse(cfg_t * cfg) {
    redis_server_cfg_t * rcfg;

    if (cfg == NULL) {
        return NULL;
    }

    if (cfg_getbool(cfg, "enabled") == cfg_false) {
        return NULL;
    }

    if (!(rcfg = redis_server_cfg_new())) {
        return NULL;
    }

    if (cfg_getstr(cfg, "server")) {
        rcfg->server = strdup(cfg_getstr(cfg, "server"));
    }

    if (cfg_getint(cfg, "port")) {
        rcfg->port = cfg_getint(cfg, "port");
    }

    return rcfg;
}

void
redis_server_cfg_free(redis_server_cfg_t * rcfg) {
    if (rcfg == NULL) {
        return;
    }
    free(rcfg->server);
    free(rcfg);
}

redis_servers_cfg_t *
redis_servers_cfg_new(void) {
    redis_servers_cfg_t * redis_servers;

    if (!(redis_servers = calloc(sizeof(redis_servers_cfg_t), 1))) {
        return NULL;
    }
    redis_servers->get_cmd_tpl = NULL;
    TAILQ_INIT(&redis_servers->redis_servers_q);

    return redis_servers;
}

redis_servers_cfg_t *
redis_servers_cfg_parse(cfg_t * cfg) {
    redis_servers_cfg_t * rcfg;
    int                   i;
    int                   n_redis_server;

    if (cfg == NULL) {
        return NULL;
    }

    if (!(rcfg = redis_servers_cfg_new())) {
        return NULL;
    }
    if (cfg_getstr(cfg, "get-cmd-tpl")) {
        rcfg->get_cmd_tpl = strdup(cfg_getstr(cfg, "get-cmd-tpl"));
    }
    n_redis_server = cfg_size(cfg, "redis-server");
    for (i = 0; i < n_redis_server; i++) {
        redis_server_cfg_t * server;
        if (!(server = redis_server_cfg_parse(cfg_getnsec(cfg, "redis-server", i)))) {
            continue;
        }
        TAILQ_INSERT_TAIL(&rcfg->redis_servers_q, server, next);
    }

    return rcfg;
}

void
redis_servers_cfg_free(redis_servers_cfg_t * rcfg) {
    redis_server_cfg_t * server;
    redis_server_cfg_t * server_save;

    if (rcfg == NULL) {
        return;
    }
    for (server = TAILQ_FIRST(&rcfg->redis_servers_q); server;
         server = server_save) {
        server_save = TAILQ_NEXT(server, next);
        redis_server_cfg_free(server);
    }
    free(rcfg);
}

ip_id_map_cfg_t *
ip_id_map_cfg_new(void) {
    ip_id_map_cfg_t *icfg;

    if (!(icfg = calloc(sizeof(ip_id_map_cfg_t), 1))) {
        return NULL;
    }
    icfg->redis_servers = NULL;

    return icfg;
}

ip_id_map_cfg_t *
ip_id_map_cfg_parse(cfg_t * cfg) {
    ip_id_map_cfg_t * icfg;

    if (cfg == NULL) {
        return NULL;
    }

    if (cfg_getbool(cfg, "enabled") == cfg_false) {
        return NULL;
    }

    if (!(icfg = ip_id_map_cfg_new())) {
        return NULL;
    }

    if (cfg_getstr(cfg, "header-name")) {
        icfg->header_name = strdup(cfg_getstr(cfg, "header-name"));
    }

    if (cfg_getstr(cfg, "type")) {
        if (strcmp(cfg_getstr(cfg, "type"), "redis") != 0) {
            return NULL;
        }
    }

    icfg->redis_servers =
        redis_servers_cfg_parse(cfg_getsec(cfg, "redis-servers"));

    return icfg;
}

void
ip_id_map_cfg_free(ip_id_map_cfg_t * icfg) {
    redis_server_cfg_t * rcfg;
    redis_server_cfg_t * rcfg_save;

    if (icfg == NULL) {
        return;
    }
    free(icfg->header_name);
    redis_servers_cfg_free(icfg->redis_servers);
    free(icfg);
}
