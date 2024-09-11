/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef CEREG_H__
#define CEREG_H__

#include <modem/lte_lc.h>

#ifdef __cplusplus
extern "C" {
#endif

int cereg_status_get(enum lte_lc_nw_reg_status *status);
int cereg_mode_get(enum lte_lc_lte_mode *mode);
int cereg_lte_connect(bool blocking);
int cereg_notifications_enable(void);

#ifdef __cplusplus
}
#endif

#endif /* CEREG_H__ */
