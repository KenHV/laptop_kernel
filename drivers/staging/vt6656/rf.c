// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * Purpose: rf function code
 *
 * Author: Jerry Chen
 *
 * Date: Feb. 19, 2004
 *
 * Functions:
 *	vnt_rf_write_embedded	- Embedded write RF register via MAC
 *
 * Revision History:
 *	RF_VT3226: RobertYu:20051111, VT3226C0 and before
 *	RF_VT3226D0: RobertYu:20051228
 *	RF_VT3342A0: RobertYu:20060609
 *
 */

#include <linux/errno.h>
#include "mac.h"
#include "rf.h"
#include "baseband.h"
#include "usbpipe.h"

#define CB_AL2230_INIT_SEQ    15
#define CB_AL7230_INIT_SEQ    16
#define CB_VT3226_INIT_SEQ    11
#define CB_VT3342_INIT_SEQ    13

static u8 al2230_init_table[CB_AL2230_INIT_SEQ][3] = {
	{0x03, 0xf7, 0x90},
	{0x03, 0x33, 0x31},
	{0x01, 0xb8, 0x02},
	{0x00, 0xff, 0xf3},
	{0x00, 0x05, 0xa4},
	{0x0f, 0x4d, 0xc5},
	{0x08, 0x05, 0xb6},
	{0x01, 0x47, 0xc7},
	{0x00, 0x06, 0x88},
	{0x04, 0x03, 0xb9},
	{0x00, 0xdb, 0xba},
	{0x00, 0x09, 0x9b},
	{0x0b, 0xdf, 0xfc},
	{0x00, 0x00, 0x0d},
	{0x00, 0x58, 0x0f}
};

static u8 al2230_channel_table0[CB_MAX_CHANNEL_24G][3] = {
	{0x03, 0xf7, 0x90},
	{0x03, 0xf7, 0x90},
	{0x03, 0xe7, 0x90},
	{0x03, 0xe7, 0x90},
	{0x03, 0xf7, 0xa0},
	{0x03, 0xf7, 0xa0},
	{0x03, 0xe7, 0xa0},
	{0x03, 0xe7, 0xa0},
	{0x03, 0xf7, 0xb0},
	{0x03, 0xf7, 0xb0},
	{0x03, 0xe7, 0xb0},
	{0x03, 0xe7, 0xb0},
	{0x03, 0xf7, 0xc0},
	{0x03, 0xe7, 0xc0}
};

static u8 al2230_channel_table1[CB_MAX_CHANNEL_24G][3] = {
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x06, 0x66, 0x61}
};

static u8 al7230_init_table[CB_AL7230_INIT_SEQ][3] = {
	{0x20, 0x37, 0x90},
	{0x13, 0x33, 0x31},
	{0x84, 0x1f, 0xf2},
	{0x3f, 0xdf, 0xa3},
	{0x7f, 0xd7, 0x84},
	{0x80, 0x2b, 0x55},
	{0x56, 0xaf, 0x36},
	{0xce, 0x02, 0x07},
	{0x6e, 0xbc, 0x98},
	{0x22, 0x1b, 0xb9},
	{0xe0, 0x00, 0x0a},
	{0x08, 0x03, 0x1b},
	{0x00, 0x0a, 0x3c},
	{0xff, 0xff, 0xfd},
	{0x00, 0x00, 0x0e},
	{0x1a, 0xba, 0x8f}
};

static u8 al7230_init_table_amode[CB_AL7230_INIT_SEQ][3] = {
	{0x2f, 0xf5, 0x20},
	{0x00, 0x00, 0x01},
	{0x45, 0x1f, 0xe2},
	{0x5f, 0xdf, 0xa3},
	{0x6f, 0xd7, 0x84},
	{0x85, 0x3f, 0x55},
	{0x56, 0xaf, 0x36},
	{0xce, 0x02, 0x07},
	{0x6e, 0xbc, 0x98},
	{0x22, 0x1b, 0xb9},
	{0xe0, 0x60, 0x0a},
	{0x08, 0x03, 0x1b},
	{0x00, 0x14, 0x7c},
	{0xff, 0xff, 0xfd},
	{0x00, 0x00, 0x0e},
	{0x12, 0xba, 0xcf}
};

static u8 al7230_channel_table0[CB_MAX_CHANNEL][3] = {
	{0x20, 0x37, 0x90},
	{0x20, 0x37, 0x90},
	{0x20, 0x37, 0x90},
	{0x20, 0x37, 0x90},
	{0x20, 0x37, 0xa0},
	{0x20, 0x37, 0xa0},
	{0x20, 0x37, 0xa0},
	{0x20, 0x37, 0xa0},
	{0x20, 0x37, 0xb0},
	{0x20, 0x37, 0xb0},
	{0x20, 0x37, 0xb0},
	{0x20, 0x37, 0xb0},
	{0x20, 0x37, 0xc0},
	{0x20, 0x37, 0xc0},
	{0x0f, 0xf5, 0x20}, /* channel 15 Tf = 4915MHz */
	{0x2f, 0xf5, 0x20},
	{0x0f, 0xf5, 0x20},
	{0x0f, 0xf5, 0x20},
	{0x2f, 0xf5, 0x20},
	{0x0f, 0xf5, 0x20},
	{0x2f, 0xf5, 0x30},
	{0x2f, 0xf5, 0x30},
	{0x0f, 0xf5, 0x40},
	{0x2f, 0xf5, 0x40},
	{0x0f, 0xf5, 0x40},
	{0x0f, 0xf5, 0x40},
	{0x2f, 0xf5, 0x40},
	{0x2f, 0xf5, 0x50},
	{0x2f, 0xf5, 0x60},
	{0x2f, 0xf5, 0x60},
	{0x2f, 0xf5, 0x70},
	{0x2f, 0xf5, 0x70},
	{0x2f, 0xf5, 0x70},
	{0x2f, 0xf5, 0x70},
	{0x2f, 0xf5, 0x70},
	{0x2f, 0xf5, 0x70},
	{0x2f, 0xf5, 0x80},
	{0x2f, 0xf5, 0x80},
	{0x2f, 0xf5, 0x80},
	{0x2f, 0xf5, 0x90},
	{0x2f, 0xf5, 0xc0},
	{0x2f, 0xf5, 0xc0},
	{0x2f, 0xf5, 0xc0},
	{0x2f, 0xf5, 0xd0},
	{0x2f, 0xf5, 0xd0},
	{0x2f, 0xf5, 0xd0},
	{0x2f, 0xf5, 0xe0},
	{0x2f, 0xf5, 0xe0},
	{0x2f, 0xf5, 0xe0},
	{0x2f, 0xf5, 0xf0},
	{0x2f, 0xf5, 0xf0},
	{0x2f, 0xf6, 0x00},
	{0x2f, 0xf6, 0x00},
	{0x2f, 0xf6, 0x00},
	{0x2f, 0xf6, 0x10},
	{0x2f, 0xf6, 0x10}
};

static u8 al7230_channel_table1[CB_MAX_CHANNEL][3] = {
	{0x13, 0x33, 0x31},
	{0x1b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x13, 0x33, 0x31},
	{0x1b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x13, 0x33, 0x31},
	{0x1b, 0x33, 0x31},
	{0x03, 0x33, 0x31},
	{0x0b, 0x33, 0x31},
	{0x13, 0x33, 0x31},
	{0x06, 0x66, 0x61},
	{0x1d, 0x55, 0x51}, /* channel = 15, Tf = 4915MHz */
	{0x00, 0x00, 0x01},
	{0x02, 0xaa, 0xa1},
	{0x08, 0x00, 0x01},
	{0x0a, 0xaa, 0xa1},
	{0x0d, 0x55, 0x51},
	{0x15, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x1d, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x02, 0xaa, 0xa1},
	{0x08, 0x00, 0x01},
	{0x0a, 0xaa, 0xa1},
	{0x15, 0x55, 0x51},
	{0x05, 0x55, 0x51},
	{0x0a, 0xaa, 0xa1},
	{0x10, 0x00, 0x01},
	{0x15, 0x55, 0x51},
	{0x1a, 0xaa, 0xa1},
	{0x00, 0x00, 0x01},
	{0x05, 0x55, 0x51},
	{0x0a, 0xaa, 0xa1},
	{0x15, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x0a, 0xaa, 0xa1},
	{0x15, 0x55, 0x51},
	{0x15, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x0a, 0xaa, 0xa1},
	{0x15, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x0a, 0xaa, 0xa1},
	{0x15, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x0a, 0xaa, 0xa1},
	{0x15, 0x55, 0x51},
	{0x00, 0x00, 0x01},
	{0x18, 0x00, 0x01},
	{0x02, 0xaa, 0xa1},
	{0x0d, 0x55, 0x51},
	{0x18, 0x00, 0x01},
	{0x02, 0xaa, 0xb1}
};

static u8 al7230_channel_table2[CB_MAX_CHANNEL][3] = {
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84}, /* channel = 15 Tf = 4915MHz */
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x6f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84},
	{0x7f, 0xd7, 0x84}
};

static u8 vt3226_init_table[CB_VT3226_INIT_SEQ][3] = {
	{0x03, 0xff, 0x80},
	{0x02, 0x82, 0xa1},
	{0x03, 0xc6, 0xa2},
	{0x01, 0x97, 0x93},
	{0x03, 0x66, 0x64},
	{0x00, 0x61, 0xa5},
	{0x01, 0x7b, 0xd6},
	{0x00, 0x80, 0x17},
	{0x03, 0xf8, 0x08},
	{0x00, 0x02, 0x39},
	{0x02, 0x00, 0x2a}
};

static u8 vt3226d0_init_table[CB_VT3226_INIT_SEQ][3] = {
	{0x03, 0xff, 0x80},
	{0x03, 0x02, 0x21},
	{0x03, 0xc6, 0xa2},
	{0x01, 0x97, 0x93},
	{0x03, 0x66, 0x64},
	{0x00, 0x71, 0xa5},
	{0x01, 0x15, 0xc6},
	{0x01, 0x2e, 0x07},
	{0x00, 0x58, 0x08},
	{0x00, 0x02, 0x79},
	{0x02, 0x01, 0xaa}
};

static u8 vt3226_channel_table0[CB_MAX_CHANNEL_24G][3] = {
	{0x01, 0x97, 0x83},
	{0x01, 0x97, 0x83},
	{0x01, 0x97, 0x93},
	{0x01, 0x97, 0x93},
	{0x01, 0x97, 0x93},
	{0x01, 0x97, 0x93},
	{0x01, 0x97, 0xa3},
	{0x01, 0x97, 0xa3},
	{0x01, 0x97, 0xa3},
	{0x01, 0x97, 0xa3},
	{0x01, 0x97, 0xb3},
	{0x01, 0x97, 0xb3},
	{0x01, 0x97, 0xb3},
	{0x03, 0x37, 0xc3}
};

static u8 vt3226_channel_table1[CB_MAX_CHANNEL_24G][3] = {
	{0x02, 0x66, 0x64},
	{0x03, 0x66, 0x64},
	{0x00, 0x66, 0x64},
	{0x01, 0x66, 0x64},
	{0x02, 0x66, 0x64},
	{0x03, 0x66, 0x64},
	{0x00, 0x66, 0x64},
	{0x01, 0x66, 0x64},
	{0x02, 0x66, 0x64},
	{0x03, 0x66, 0x64},
	{0x00, 0x66, 0x64},
	{0x01, 0x66, 0x64},
	{0x02, 0x66, 0x64},
	{0x00, 0xcc, 0xc4}
};

static const u32 vt3226d0_lo_current_table[CB_MAX_CHANNEL_24G] = {
	0x0135c600,
	0x0135c600,
	0x0235c600,
	0x0235c600,
	0x0235c600,
	0x0335c600,
	0x0335c600,
	0x0335c600,
	0x0335c600,
	0x0335c600,
	0x0335c600,
	0x0335c600,
	0x0335c600,
	0x0135c600
};

static u8 vt3342a0_init_table[CB_VT3342_INIT_SEQ][3] = { /* 11b/g mode */
	{0x03, 0xff, 0x80},
	{0x02, 0x08, 0x81},
	{0x00, 0xc6, 0x02},
	{0x03, 0xc5, 0x13},
	{0x00, 0xee, 0xe4},
	{0x00, 0x71, 0xa5},
	{0x01, 0x75, 0x46},
	{0x01, 0x40, 0x27},
	{0x01, 0x54, 0x08},
	{0x00, 0x01, 0x69},
	{0x02, 0x00, 0xaa},
	{0x00, 0x08, 0xcb},
	{0x01, 0x70, 0x0c}
};

static u8 vt3342_channel_table0[CB_MAX_CHANNEL][3] = {
	{0x02, 0x05, 0x03},
	{0x01, 0x15, 0x03},
	{0x03, 0xc5, 0x03},
	{0x02, 0x65, 0x03},
	{0x01, 0x15, 0x13},
	{0x03, 0xc5, 0x13},
	{0x02, 0x05, 0x13},
	{0x01, 0x15, 0x13},
	{0x03, 0xc5, 0x13},
	{0x02, 0x65, 0x13},
	{0x01, 0x15, 0x23},
	{0x03, 0xc5, 0x23},
	{0x02, 0x05, 0x23},
	{0x00, 0xd5, 0x23},
	{0x01, 0x15, 0x13}, /* channel = 15 Tf = 4915MHz */
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x15, 0x13},
	{0x01, 0x55, 0x63},
	{0x01, 0x55, 0x63},
	{0x02, 0xa5, 0x63},
	{0x02, 0xa5, 0x63},
	{0x00, 0x05, 0x73},
	{0x00, 0x05, 0x73},
	{0x01, 0x55, 0x73},
	{0x02, 0xa5, 0x73},
	{0x00, 0x05, 0x83},
	{0x01, 0x55, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x02, 0xa5, 0x83},
	{0x00, 0x05, 0xF3},
	{0x01, 0x56, 0x03},
	{0x02, 0xa6, 0x03},
	{0x00, 0x06, 0x03},
	{0x00, 0x06, 0x03}
};

static u8 vt3342_channel_table1[CB_MAX_CHANNEL][3] = {
	{0x01, 0x99, 0x94},
	{0x02, 0x44, 0x44},
	{0x02, 0xee, 0xe4},
	{0x03, 0x99, 0x94},
	{0x00, 0x44, 0x44},
	{0x00, 0xee, 0xe4},
	{0x01, 0x99, 0x94},
	{0x02, 0x44, 0x44},
	{0x02, 0xee, 0xe4},
	{0x03, 0x99, 0x94},
	{0x00, 0x44, 0x44},
	{0x00, 0xee, 0xe4},
	{0x01, 0x99, 0x94},
	{0x03, 0x33, 0x34},
	{0x00, 0x44, 0x44}, /* channel = 15 Tf = 4915MHz */
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x00, 0x44, 0x44},
	{0x01, 0x55, 0x54},
	{0x01, 0x55, 0x54},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x00, 0x00, 0x04},
	{0x00, 0x00, 0x04},
	{0x01, 0x55, 0x54},
	{0x02, 0xaa, 0xa4},
	{0x00, 0x00, 0x04},
	{0x01, 0x55, 0x54},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x02, 0xaa, 0xa4},
	{0x03, 0x00, 0x04},
	{0x00, 0x55, 0x54},
	{0x01, 0xaa, 0xa4},
	{0x03, 0x00, 0x04},
	{0x03, 0x00, 0x04}
};

enum {
	VNT_TABLE_INIT = 0,
	VNT_TABLE_INIT_2 = 0,
	VNT_TABLE_0 = 1,
	VNT_TABLE_1 = 2,
	VNT_TABLE_2 = 1
};

struct vnt_table_info {
	u8 *addr;
	int length;
};

static const struct vnt_table_info vnt_table_seq[][3] = {
	{	/* RF_AL2230, RF_AL2230S init table, channel table 0 and 1 */
		{&al2230_init_table[0][0], CB_AL2230_INIT_SEQ * 3},
		{&al2230_channel_table0[0][0], CB_MAX_CHANNEL_24G * 3},
		{&al2230_channel_table1[0][0], CB_MAX_CHANNEL_24G * 3}
	}, {	/* RF_AIROHA7230 init table, channel table 0 and 1 */
		{&al7230_init_table[0][0], CB_AL7230_INIT_SEQ * 3},
		{&al7230_channel_table0[0][0], CB_MAX_CHANNEL * 3},
		{&al7230_channel_table1[0][0], CB_MAX_CHANNEL * 3}
	}, {	/* RF_VT3226 init table, channel table 0 and 1 */
		{&vt3226_init_table[0][0], CB_VT3226_INIT_SEQ * 3},
		{&vt3226_channel_table0[0][0], CB_MAX_CHANNEL_24G * 3},
		{&vt3226_channel_table1[0][0], CB_MAX_CHANNEL_24G * 3}
	}, {	/* RF_VT3226D0 init table, channel table 0 and 1 */
		{&vt3226d0_init_table[0][0], CB_VT3226_INIT_SEQ * 3},
		{&vt3226_channel_table0[0][0], CB_MAX_CHANNEL_24G * 3},
		{&vt3226_channel_table1[0][0], CB_MAX_CHANNEL_24G * 3}
	}, {	/* RF_VT3342A0 init table, channel table 0 and 1 */
		{&vt3342a0_init_table[0][0], CB_VT3342_INIT_SEQ * 3},
		{&vt3342_channel_table0[0][0], CB_MAX_CHANNEL * 3},
		{&vt3342_channel_table1[0][0], CB_MAX_CHANNEL * 3}
	}, {	/* RF_AIROHA7230 init table 2 and channel table 2 */
		{&al7230_init_table_amode[0][0], CB_AL7230_INIT_SEQ * 3},
		{&al7230_channel_table2[0][0], CB_MAX_CHANNEL * 3},
		{NULL, 0}
	}
};

/*
 * Description: Write to IF/RF, by embedded programming
 */
int vnt_rf_write_embedded(struct vnt_private *priv, u32 data)
{
	u8 reg_data[4];

	data |= (VNT_RF_REG_LEN << 3) | IFREGCTL_REGW;

	reg_data[0] = (u8)data;
	reg_data[1] = (u8)(data >> 8);
	reg_data[2] = (u8)(data >> 16);
	reg_data[3] = (u8)(data >> 24);

	return vnt_control_out(priv, MESSAGE_TYPE_WRITE_IFRF, 0, 0,
			       ARRAY_SIZE(reg_data), reg_data);
}

static u8 vnt_rf_addpower(struct vnt_private *priv)
{
	int base;
	s32 rssi = -priv->current_rssi;

	if (!rssi)
		return 7;

	if (priv->rf_type == RF_VT3226D0)
		base = -60;
	else
		base = -70;

	if (rssi < base)
		return ((rssi - base + 1) / -5) * 2 + 5;

	return 0;
}

/* Set Tx power by power level and rate */
static int vnt_rf_set_txpower(struct vnt_private *priv, u8 power,
			      struct ieee80211_channel *ch)
{
	u32 power_setting = 0;
	int ret = 0;

	power += vnt_rf_addpower(priv);
	if (power > VNT_RF_MAX_POWER)
		power = VNT_RF_MAX_POWER;

	if (priv->power == power)
		return 0;

	priv->power = power;

	switch (priv->rf_type) {
	case RF_AL2230:
		power_setting = 0x0404090 | (power << 12);

		ret = vnt_rf_write_embedded(priv, power_setting);
		if (ret)
			return ret;

		if (ch->flags & IEEE80211_CHAN_NO_OFDM)
			ret = vnt_rf_write_embedded(priv, 0x0001b400);
		else
			ret = vnt_rf_write_embedded(priv, 0x0005a400);

		break;
	case RF_AL2230S:
		power_setting = 0x0404090 | (power << 12);

		ret = vnt_rf_write_embedded(priv, power_setting);
		if (ret)
			return ret;

		if (ch->flags & IEEE80211_CHAN_NO_OFDM) {
			ret = vnt_rf_write_embedded(priv, 0x040c1400);
			if (ret)
				return ret;

			ret = vnt_rf_write_embedded(priv, 0x00299b00);
		} else {
			ret = vnt_rf_write_embedded(priv, 0x0005a400);
			if (ret)
				return ret;

			ret = vnt_rf_write_embedded(priv, 0x00099b00);
		}

		break;

	case RF_AIROHA7230:
		if (ch->flags & IEEE80211_CHAN_NO_OFDM)
			ret = vnt_rf_write_embedded(priv, 0x111bb900);
		else
			ret = vnt_rf_write_embedded(priv, 0x221bb900);

		if (ret)
			return ret;

		/*
		 * 0x080F1B00 for 3 wire control TxGain(D10)
		 * and 0x31 as TX Gain value
		 */
		power_setting = 0x080c0b00 | (power << 12);

		ret = vnt_rf_write_embedded(priv, power_setting);
		break;

	case RF_VT3226:
		power_setting = ((0x3f - power) << 20) | (0x17 << 8);

		ret = vnt_rf_write_embedded(priv, power_setting);
		break;
	case RF_VT3226D0:
		if (ch->flags & IEEE80211_CHAN_NO_OFDM) {
			u16 hw_value = ch->hw_value;

			power_setting = ((0x3f - power) << 20) | (0xe07 << 8);

			ret = vnt_rf_write_embedded(priv, power_setting);
			if (ret)
				return ret;

			ret = vnt_rf_write_embedded(priv, 0x03c6a200);
			if (ret)
				return ret;

			dev_dbg(&priv->usb->dev,
				"%s 11b channel [%d]\n", __func__, hw_value);

			hw_value--;

			if (hw_value < ARRAY_SIZE(vt3226d0_lo_current_table)) {
				ret = vnt_rf_write_embedded(priv,
							    vt3226d0_lo_current_table[hw_value]);
				if (ret)
					return ret;
			}

			ret = vnt_rf_write_embedded(priv, 0x015C0800);
		} else {
			dev_dbg(&priv->usb->dev,
				"@@@@ %s> 11G mode\n", __func__);

			power_setting = ((0x3f - power) << 20) | (0x7 << 8);

			ret = vnt_rf_write_embedded(priv, power_setting);
			if (ret)
				return ret;

			ret = vnt_rf_write_embedded(priv, 0x00C6A200);
			if (ret)
				return ret;

			ret = vnt_rf_write_embedded(priv, 0x016BC600);
			if (ret)
				return ret;

			ret = vnt_rf_write_embedded(priv, 0x00900800);
		}

		break;

	case RF_VT3342A0:
		power_setting =  ((0x3f - power) << 20) | (0x27 << 8);

		ret = vnt_rf_write_embedded(priv, power_setting);
		break;
	default:
		break;
	}
	return ret;
}

/* Set Tx power by channel number type */
int vnt_rf_setpower(struct vnt_private *priv,
		    struct ieee80211_channel *ch)
{
	u16 channel;
	u8 power = priv->cck_pwr;

	if (!ch)
		return -EINVAL;

	/* set channel number to array number */
	channel = ch->hw_value - 1;

	if (ch->flags & IEEE80211_CHAN_NO_OFDM) {
		if (channel < ARRAY_SIZE(priv->cck_pwr_tbl))
			power = priv->cck_pwr_tbl[channel];
	} else if (ch->band == NL80211_BAND_5GHZ) {
		/* remove 14 channels to array size */
		channel -= 14;

		if (channel < ARRAY_SIZE(priv->ofdm_a_pwr_tbl))
			power = priv->ofdm_a_pwr_tbl[channel];
	} else {
		if (channel < ARRAY_SIZE(priv->ofdm_pwr_tbl))
			power = priv->ofdm_pwr_tbl[channel];
	}

	return vnt_rf_set_txpower(priv, power, ch);
}

/* Convert rssi to dbm */
void vnt_rf_rssi_to_dbm(struct vnt_private *priv, u8 rssi, long *dbm)
{
	u8 idx = ((rssi & 0xc0) >> 6) & 0x03;
	long b = rssi & 0x3f;
	long a = 0;
	u8 airoharf[4] = {0, 18, 0, 40};

	switch (priv->rf_type) {
	case RF_AL2230:
	case RF_AL2230S:
	case RF_AIROHA7230:
	case RF_VT3226:
	case RF_VT3226D0:
	case RF_VT3342A0:
		a = airoharf[idx];
		break;
	default:
		break;
	}

	*dbm = -1 * (a + b * 2);
}

int vnt_rf_table_download(struct vnt_private *priv)
{
	int ret;
	int idx = -1;
	const struct vnt_table_info *table_seq;

	switch (priv->rf_type) {
	case RF_AL2230:
	case RF_AL2230S:
		idx = 0;
		break;
	case RF_AIROHA7230:
		idx = 1;
		break;
	case RF_VT3226:
		idx = 2;
		break;
	case RF_VT3226D0:
		idx = 3;
		break;
	case RF_VT3342A0:
		idx = 4;
		break;
	}

	if (idx < 0)
		return 0;

	table_seq = &vnt_table_seq[idx][0];

	/* Init Table */
	ret = vnt_control_out(priv, MESSAGE_TYPE_WRITE, 0,
			      MESSAGE_REQUEST_RF_INIT,
			      table_seq[VNT_TABLE_INIT].length,
			      table_seq[VNT_TABLE_INIT].addr);
	if (ret)
		return ret;

	/* Channel Table 0 */
	ret = vnt_control_out_blocks(priv, VNT_REG_BLOCK_SIZE,
				     MESSAGE_REQUEST_RF_CH0,
				     table_seq[VNT_TABLE_0].length,
				     table_seq[VNT_TABLE_0].addr);
	if (ret)
		return ret;

	/* Channel Table 1 */
	ret = vnt_control_out_blocks(priv, VNT_REG_BLOCK_SIZE,
				     MESSAGE_REQUEST_RF_CH1,
				     table_seq[VNT_TABLE_1].length,
				     table_seq[VNT_TABLE_1].addr);

	if (priv->rf_type == RF_AIROHA7230) {
		table_seq = &vnt_table_seq[5][0];

		/* Init Table 2 */
		ret = vnt_control_out(priv, MESSAGE_TYPE_WRITE, 0,
				      MESSAGE_REQUEST_RF_INIT2,
				      table_seq[VNT_TABLE_INIT_2].length,
				      table_seq[VNT_TABLE_INIT_2].addr);
		if (ret)
			return ret;

		/* Channel Table 2 */
		ret = vnt_control_out_blocks(priv, VNT_REG_BLOCK_SIZE,
					     MESSAGE_REQUEST_RF_CH2,
					     table_seq[VNT_TABLE_2].length,
					     table_seq[VNT_TABLE_2].addr);
	}

	return ret;
}
