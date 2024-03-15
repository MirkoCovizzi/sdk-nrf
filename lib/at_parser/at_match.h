/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef AT_MATCH_H__
#define AT_MATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

struct at_token at_match(const char *at, const char **remainder);

#ifdef __cplusplus
}
#endif

#endif /* AT_MATCH_H__ */
