/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef PSM_H__
#define PSM_H__

#include <modem/lte_lc.h>

#ifdef __cplusplus
extern "C" {
#endif

int psm_param_set(const char *rptau, const char *rat);
int psm_param_set_seconds(int rptau, int rat);
int psm_req(bool enable);
int psm_proprietary_req(bool enable);
int psm_get(int *tau, int *active_time);
void psm_evt_update_send(struct lte_lc_psm_cfg *psm_cfg);
int psm_parse(const char *active_time_str, const char *tau_ext_str,
	      const char *tau_legacy_str, struct lte_lc_psm_cfg *psm_cfg);
struct k_work *psm_work_get(void);

#ifdef __cplusplus
}
#endif

#endif /* PSM_H__ */
