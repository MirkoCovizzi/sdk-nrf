/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef XFACTORYRESET_H__
#define XFACTORYRESET_H__

#include <modem/lte_lc/lte_lc_xfactoryreset.h>

#ifdef __cplusplus
extern "C" {
#endif

int xfactoryreset_reset(enum lte_lc_factory_reset_type type);

#ifdef __cplusplus
}
#endif

#endif /* XFACTORYRESET_H__ */
