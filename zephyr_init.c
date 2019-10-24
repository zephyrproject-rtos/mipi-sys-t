/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <kernel.h>
#include <mipi_syst.h>
#include <logging/log_output.h>

static struct mipi_syst_header log_syst_header;
struct mipi_syst_handle log_syst_handle;

#if defined(MIPI_SYST_PCFG_ENABLE_PLATFORM_STATE_DATA)
static const char pattern[] = "SYS-T RAW DATA: ";
static const char valToHex[] = "0123456789ABCDEF";

static int out_func(int c, void *ctx)
{
	const struct log_output *out_ctx =
		(const struct log_output *)ctx;

	out_ctx->buf[out_ctx->control_block->offset] = (u8_t)c;
	out_ctx->control_block->offset++;

	__ASSERT_NO_MSG(out_ctx->control_block->offset <= out_ctx->size);

	if (out_ctx->control_block->offset == out_ctx->size) {
		log_output_flush(out_ctx);
	}

	return 0;
}

/*
 * dump contents for raw message bytes
 */
static void write_raw(
	struct mipi_syst_handle *systh, const void *p, int n)
{
	int i;
	mipi_syst_u8 c;

#if defined(MIPI_SYST_BIG_ENDIAN)
	for (i = n - 1; i >= 0; --i) {
#else
	for (i = 0; i < n; ++i) {
#endif
		c = ((const mipi_syst_u8 *)p)[i];
		out_func(valToHex[c >> 0x4],
			systh->systh_platform.log_output);
		out_func(valToHex[c & 0xF],
			systh->systh_platform.log_output);
	}
}

static void write_d8(struct mipi_syst_handle *systh, mipi_syst_u8 v)
{
	write_raw(systh, &v, sizeof(v));
}

static void write_d16(struct mipi_syst_handle *systh, mipi_syst_u16 v)
{
	write_raw(systh, &v, sizeof(v));
}

static void write_d32(struct mipi_syst_handle *systh, mipi_syst_u32 v)
{
	write_raw(systh, &v, sizeof(v));
}

#if defined(MIPI_SYST_PCFG_ENABLE_64BIT_IO)
static void write_d64(struct mipi_syst_handle *systh, mipi_syst_u64 v)
{
	write_raw(systh, &v, sizeof(v));
}
#endif

static void write_d32ts(struct mipi_syst_handle *systh, mipi_syst_u32 v)
{
	for (int i = 0; i < strlen(pattern); i++) {
		out_func(pattern[i],
			systh->systh_platform.log_output);
	}

	write_raw(systh, &v, sizeof(v));
}

static void write_flag(struct mipi_syst_handle *systh)
{
	u32_t flag = systh->systh_platform.flag;

	if ((flag & LOG_OUTPUT_FLAG_CRLF_NONE) != 0U) {
		return;
	}

	if ((flag & LOG_OUTPUT_FLAG_CRLF_LFONLY) != 0U) {
		out_func('\n', systh->systh_platform.log_output);
	} else {
		out_func('\r', systh->systh_platform.log_output);
		out_func('\n', systh->systh_platform.log_output);
	}
}
#endif

#if defined(MIPI_SYST_PCFG_ENABLE_PLATFORM_HANDLE_DATA)
/*
 * Platform specific SyS-T handle initialization hook function
 *
 * @param systh pointer to the SyS-T handle structure
 */
static void platform_handle_init(struct mipi_syst_handle *systh)
{
	ARG_UNUSED(systh);
}

static void platform_handle_release(struct mipi_syst_handle *systh)
{
	ARG_UNUSED(systh);
}
#endif

#if defined(MIPI_SYST_PCFG_ENABLE_TIMESTAMP)
mipi_syst_u64 mipi_syst_get_epoch(void)
{
	return k_uptime_get();
}
#endif

/*
 * Platform specific global state initialization hook function
 *
 * @param systh pointer to the new SyS-T handle structure
 * @param platform_data user defined data for the init function.
 */
static void mipi_syst_platform_init(struct mipi_syst_header *systh,
		const void *platform_data)
{
#if defined(MIPI_SYST_PCFG_ENABLE_PLATFORM_HANDLE_DATA)
	/*
	 * Set handle init hook that performs per SyS-T handle initialization
	 * and destruction
	 */
	systh->systh_inith = platform_handle_init;
	systh->systh_releaseh = platform_handle_release;
#endif

#if defined(MIPI_SYST_PCFG_ENABLE_PLATFORM_STATE_DATA)
	/*
	 * Initialize platform specific data in global state
	 */
	systh->systh_platform.write_d8 = write_d8;
	systh->systh_platform.write_d16 = write_d16;
	systh->systh_platform.write_d32 = write_d32;
#if defined(MIPI_SYST_PCFG_ENABLE_64BIT_IO)
	systh->systh_platform.write_d64 = write_d64;
#endif
	systh->systh_platform.write_d32ts = write_d32ts;
	systh->systh_platform.write_flag = write_flag;
#endif
}

void update_systh_platform_data(struct mipi_syst_handle *handle,
		const struct log_output *log_output, u32_t flag)
{
#if defined(MIPI_SYST_PCFG_ENABLE_PLATFORM_STATE_DATA)
	handle->systh_platform.flag = (mipi_syst_u32)flag;
	handle->systh_platform.log_output = (struct log_output *)log_output;
#endif
}

static int _syst_init()
{
	MIPI_SYST_INIT_STATE(&log_syst_header,
			mipi_syst_platform_init, (void*)0);
	MIPI_SYST_INIT_HANDLE_STATE(&log_syst_header,
			&log_syst_handle, NULL);

	return 0;
}

SYS_INIT(_syst_init, POST_KERNEL, 0);
