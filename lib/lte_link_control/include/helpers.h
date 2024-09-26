/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef HELPERS_H__
#define HELPERS_H__

#include <string.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

static int string_to_int(const char *str_buf, int base, int *output)
{
	int temp;
	char *end_ptr;

	__ASSERT_NO_MSG(str_buf != NULL);

	errno = 0;
	temp = strtol(str_buf, &end_ptr, base);

	if (end_ptr == str_buf || *end_ptr != '\0' ||
	    ((temp == LONG_MAX || temp == LONG_MIN) && errno == ERANGE)) {
		return -ENODATA;
	}

	*output = temp;

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H__ */
