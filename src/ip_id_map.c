
#include <stdio.h>
#include <stdlib.h>

#include "ip_id_map.h"
#include "redis_servers.h"
#include "rproxy.h"

ip_id_map_t *
ip_id_map_new(rproxy_t * rproxy, ip_id_map_cfg_t * ip_id_map)
{
    ip_id_map_t * ip_id_map_c;

    if (ip_id_map == NULL || ip_id_map->header_name == NULL) {
        return NULL;
    }
    if (!(ip_id_map_c = calloc(sizeof(ip_id_map_t), 1))) {
        return NULL;
    }

    if (!(ip_id_map_c->redis_servers_c =
          redis_servers_new(rproxy, ip_id_map->redis_servers))) {
        return NULL;
    }

    if (!(ip_id_map_c->header_name = strdup(ip_id_map->header_name))) {
        return NULL;
    }

    return ip_id_map_c;
}

void
ip_id_map_free(ip_id_map_t * ip_id_map_c)
{
    redis_servers_free(ip_id_map_c->redis_servers_c);
    free(ip_id_map_c->header_name);
    free(ip_id_map_c);
}

static char *
ip_id_map_get_ip_from_request(char hbuf[NI_MAXHOST],
                              evhtp_request_t * upstream_req)
{
    evhtp_connection_t * conn = upstream_req->conn;
    struct sockaddr * saddr_ = conn->saddr;
    struct sockaddr_in * saddr = (struct sockaddr_in *) saddr_;

    switch (((struct sockaddr_storage *) saddr_)->ss_family) {
    case AF_INET: {
        struct sockaddr_in * saddr = (struct sockaddr_in *) saddr_;
        if (evutil_inet_ntop(AF_INET, &saddr->sin_addr,
                             hbuf, sizeof saddr) != 0) {
            return NULL;
        }
        break;
    }
    case AF_INET6: {
        struct sockaddr_in6 * saddr = (struct sockaddr_in6 *) saddr_;
        if (evutil_inet_ntop(AF_INET6,
                             &saddr->sin6_addr, hbuf, sizeof saddr) != 0) {
            return NULL;
        }
        break;
    }
    }
    return hbuf;
}

static char *
ip_id_map_get_id_from_ip(rproxy_t * rproxy,
                         evhtp_request_t * upstream_req)
{
    char ip[NI_MAXHOST];
    struct ip_id_map * ip_id_map_c = rproxy->ip_id_map_c;
    char * id = NULL;

    if (ip_id_map_c == NULL) {
        return NULL;
    }
    if (ip_id_map_get_ip_from_request(ip, upstream_req) == NULL) {
        return NULL;
    }
    id = redis_servers_get_id_from_ip(ip, ip_id_map_c->redis_servers_c);

    return id;
}

int ip_id_map_append_x_headers(rproxy_t * rproxy,
                               evhtp_request_t * upstream_req)
{
    ip_id_map_t     * ip_id_map_c;
    evhtp_headers_t * headers;
    headers_cfg_t   * headers_cfg;
    char            * id;

    if (!rproxy || !upstream_req) {
        return -1;
    }

    if (!(headers = upstream_req->headers_in)) {
        return -1;
    }

    if (!(headers_cfg = rproxy->server_cfg->headers)) {
        return 0;
    }

    ip_id_map_c = rproxy->ip_id_map_c;
    id = ip_id_map_get_id_from_ip(rproxy, upstream_req);
    if (id == NULL) {
        return 0;
    }
    evhtp_kv_rm_and_free(headers, evhtp_kvs_find_kv(headers,
                                                    ip_id_map_c->header_name));
    evhtp_headers_add_header
        (headers, evhtp_header_new(ip_id_map_c->header_name, id, 0, 1));
    free(id);

    return 0;
}
