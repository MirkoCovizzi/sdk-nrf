/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include "common/work_q.h"

K_THREAD_STACK_DEFINE(work_q_stack, CONFIG_LTE_LC_WORKQUEUE_STACK_SIZE);

struct k_work_q work_q;

int work_q_start(void)
{
	struct k_work_queue_config cfg = {
		.name = "work_q",
	};

	k_work_queue_start(&work_q, work_q_stack, K_THREAD_STACK_SIZEOF(work_q_stack),
			   K_LOWEST_APPLICATION_THREAD_PRIO, &cfg);

	return 0;
}

struct k_work_q *work_q_get(void)
{
	return &work_q;
}
