/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef REDMOB_H__
#define REDMOB_H__

#include <modem/lte_lc/lte_lc_redmob.h>

#ifdef __cplusplus
extern "C" {
#endif

int redmob_get(enum lte_lc_reduced_mobility_mode *mode);
int redmob_set(enum lte_lc_reduced_mobility_mode mode);

#ifdef __cplusplus
}
#endif

#endif /* REDMOB_H__ */
