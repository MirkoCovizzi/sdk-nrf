/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef WORK_Q_H__
#define WORK_Q_H__

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

int work_q_start(void);
struct k_work_q *work_q_get(void);

#ifdef __cplusplus
}
#endif

#endif /* WORK_Q_H__ */
