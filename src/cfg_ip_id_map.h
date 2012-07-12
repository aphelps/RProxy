
#ifndef __CFG_IP_ID_MAP_H__
#define __CFG_IP_ID_MAP_H__ 1

#include "ip_id_map.h"

ip_id_map_cfg_t * ip_id_map_cfg_parse(cfg_t * cfg);

#ifndef DEFINE_GLOBALS_FOR_CFG_IP_ID_MAP
extern
#endif
cfg_opt_t ip_id_map_opts[];

#endif
