/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef PERIODICSEARCHCONF_H__
#define PERIODICSEARCHCONF_H__

#include <modem/lte_lc/lte_lc_periodicsearchconf.h>

#ifdef __cplusplus
extern "C" {
#endif

int periodicsearchconf_set(const struct lte_lc_periodic_search_cfg *const cfg);
int periodicsearchconf_get(struct lte_lc_periodic_search_cfg *const cfg);
int periodicsearchconf_clear(void);
int periodicsearchconf_request(void);

#ifdef __cplusplus
}
#endif

#endif /* PERIODICSEARCHCONF_H__ */
