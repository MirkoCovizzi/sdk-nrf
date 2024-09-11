/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef EDRX_H__
#define EDRX_H__

#include <modem/lte_lc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Length for eDRX and PTW values */
#define LTE_LC_EDRX_VALUE_LEN 5

int edrx_cfg_get(struct lte_lc_edrx_cfg *edrx_cfg);
int edrx_ptw_set(enum lte_lc_lte_mode mode, const char *ptw);
int edrx_param_set(enum lte_lc_lte_mode mode, const char *edrx);
int edrx_request(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* EDRX_H__ */
