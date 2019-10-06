/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include "mipi_syst.h"

extern struct mipi_syst_header log_syst_header;
extern struct mipi_syst_handle log_syst_handle;

static int _syst_init()
{
	MIPI_SYST_INIT_STATE(&log_syst_header,
			mipi_syst_platform_init, (void*)0);
	MIPI_SYST_INIT_HANDLE_STATE(&log_syst_header,
			&log_syst_handle, NULL);

	return 0;
}

SYS_INIT(_syst_init, POST_KERNEL, 0);
