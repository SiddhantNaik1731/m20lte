#ifndef __EA8061S_J7DUO_PARAM_H__
#define __EA8061S_J7DUO_PARAM_H__

#include <linux/types.h>
#include <linux/kernel.h>
#include "dynamic_aid_ea8061s_j7duo.h"

#define EXTEND_BRIGHTNESS	355
#define UI_MAX_BRIGHTNESS	255
#define UI_DEFAULT_BRIGHTNESS	134
#define NORMAL_TEMPERATURE	25	/* 25 degrees Celsius */

#define GAMMA_CMD_CNT				((u16)ARRAY_SIZE(SEQ_GAMMA_CONDITION_SET))
#define ACL_CMD_CNT				((u16)ARRAY_SIZE(SEQ_ACL_OFF))
#define OPR_CMD_CNT				((u16)ARRAY_SIZE(SEQ_ACL_OPR_OFF))
#define ELVSS_CMD_CNT				((u16)ARRAY_SIZE(SEQ_ELVSS_SET))
#define AID_CMD_CNT				((u16)ARRAY_SIZE(SEQ_AID_SET))
#define TSET_CMD_CNT				((u16)ARRAY_SIZE(SEQ_TSET))
#define HBM_CMD_CNT				((u16)ARRAY_SIZE(SEQ_HBM_OFF))


#define LDI_REG_ELVSS				0xB6
#define LDI_REG_COORDINATE			0xA1
#define LDI_REG_DATE				LDI_REG_COORDINATE
#define LDI_REG_ID				0x04
#define LDI_REG_MTP				0xC8

/* len is read length */
#define LDI_LEN_ELVSS				(ELVSS_CMD_CNT - 1)
#define LDI_LEN_COORDINATE			4
#define LDI_LEN_DATE				7
#define LDI_LEN_ID				3
#define LDI_LEN_MTP				33

/* offset is position including addr, not only para */
#define LDI_OFFSET_AOR_1	3
#define LDI_OFFSET_AOR_2	4

#define LDI_OFFSET_ELVSS_1	1	/* write B6h 1st Para: MPS_CON */
#define LDI_OFFSET_ELVSS_2	2	/* write B6h 2nd Para: ELVSS_Dim_offset */
#define LDI_OFFSET_ELVSS_3	8	/* write B6h 8th Para: ELVSS Temp Compensation */

#define LDI_OFFSET_OPR_1	4	/* B4h 22nd Para: ACL Percent */

#define LDI_OFFSET_ACL		1
#define LDI_OFFSET_HBM		1

#define LDI_OFFSET_TSET		1


struct lcd_seq_info {
	unsigned char	*cmd;
	unsigned int	len;
	unsigned int	sleep;
};

static unsigned char SEQ_SLEEP_OUT[] = {
	0x11
};

static unsigned char SEQ_SLEEP_IN[] = {
	0x10
};

static unsigned char SEQ_DISPLAY_ON[] = {
	0x29
};

static unsigned char SEQ_DISPLAY_OFF[] = {
	0x28
};

static unsigned char SEQ_TEST_KEY_ON_F0[] = {
	0xF0,
	0x5A, 0x5A
};

static unsigned char SEQ_TEST_KEY_OFF_F0[] = {
	0xF0,
	0xA5, 0xA5
};

static unsigned char SEQ_TEST_KEY_ON_FC[] = {
	0xFC,
	0x5A, 0x5A
};

static unsigned char SEQ_TEST_KEY_OFF_FC[] = {
	0xFC,
	0xA5, 0xA5
};

static unsigned char SEQ_GAMMA_CONDITION_SET[] = {
	0xCA,
	0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x00, 0x00, 0x00
};

static unsigned char SEQ_AID_SET[] = {
	0xB2,
	0x00, 0x00, 0x00, 0x0C,
};

static unsigned char SEQ_ELVSS_SET[] = {
	0xB6,
	0xDC, 0x84,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, /* 8th: elvss temp compensation */
};

static unsigned char SEQ_GAMMA_UPDATE[] = {
	0xF7,
	0x03
};

static unsigned char SEQ_ACL_OPR_OFF[] = {
	0xB5,
	0x0A,
	0x9F,
	0x22,
	0x00	/* ACL 0% */
};

static unsigned char SEQ_ACL_OPR_08P[] = {
	0xB5,
	0x0A,
	0x9F,
	0x22,
	0x09	/* ACL 8% */
};

static unsigned char SEQ_ACL_OPR_15P[] = {
	0xB5,
	0x0A,
	0x9F,
	0x22,
	0x12	/* ACL 15% */
};

static unsigned char SEQ_ACL_OFF[] = {
	0x55,
	0x00
};

static unsigned char SEQ_ACL_ON[] = {
	0x55,
	0x02	/* 0x02 : ACL 15% (Default) */
};

static unsigned char SEQ_HBM_OFF[] = {
	0x53,
	0x00,
};

static unsigned char SEQ_HBM_ON[] = {
	0x53,
	0xC0,
};

static unsigned char SEQ_HSYNC_GEN_ON[] = {
	0xCF,
	0x30, 0x09,
};

static unsigned char SEQ_SOURCE_SLEW[] = {
	0xBA,
	0x77,
};

static unsigned char SEQ_S_WIRE[] = {
	0xB8,
	0x19, 0x00,
};

static unsigned char SEQ_TSET[] = {
	0xB8,
	0x19,
};

static unsigned char SEQ_POWER[] = {
	0xB1,
	0x3C, 0x89, 0x00, 0x05, 0x33, 0x31, 0x14,
};

static unsigned char SEQ_AID_SETTING_MAX[] = {
	0xB2,
	0x00, 0x00, 0x05, 0x10,
};

enum {
	ACL_STATUS_OFF,
	ACL_STATUS_ON,
	ACL_STATUS_MAX
};

enum {
	OPR_STATUS_OFF,
	OPR_STATUS_08P,
	OPR_STATUS_15P,
	OPR_STATUS_MAX
};

enum {
	TEMP_ABOVE_MINUS_00_DEGREE,	/* T > 0 */
	TEMP_ABOVE_MINUS_15_DEGREE,	/* -15 < T <= 0 */
	TEMP_BELOW_MINUS_15_DEGREE,	/* T <= -15 */
	TEMP_MAX
};

enum {
	HBM_STATUS_OFF,
	HBM_STATUS_ON,
	HBM_STATUS_MAX
};

static unsigned char *HBM_TABLE[HBM_STATUS_MAX] = {SEQ_HBM_OFF, SEQ_HBM_ON};
static unsigned char *ACL_TABLE[ACL_STATUS_MAX] = {SEQ_ACL_OFF, SEQ_ACL_ON};
static unsigned char *OPR_TABLE[OPR_STATUS_MAX] = {SEQ_ACL_OPR_OFF, SEQ_ACL_OPR_08P, SEQ_ACL_OPR_15P};

static unsigned char elvss_mpscon_offset_data[IBRIGHTNESS_MAX][4] = {
	[IBRIGHTNESS_005NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_006NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_007NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_008NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_009NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_010NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_011NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_012NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_013NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_014NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_015NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_016NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_017NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_019NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_020NIT] = {0xB6, 0xCC, 0x84},
	[IBRIGHTNESS_021NIT] = {0xB6, 0xCC, 0x89},
	[IBRIGHTNESS_022NIT] = {0xB6, 0xCC, 0x8A},
	[IBRIGHTNESS_024NIT] = {0xB6, 0xCC, 0x8C},
	[IBRIGHTNESS_025NIT] = {0xB6, 0xCC, 0x8E},
	[IBRIGHTNESS_027NIT] = {0xB6, 0xCC, 0x90},
	[IBRIGHTNESS_029NIT] = {0xB6, 0xCC, 0x92},
	[IBRIGHTNESS_030NIT] = {0xB6, 0xCC, 0x94},
	[IBRIGHTNESS_032NIT] = {0xB6, 0xCC, 0x94},
	[IBRIGHTNESS_034NIT] = {0xB6, 0xCC, 0x94},
	[IBRIGHTNESS_037NIT] = {0xB6, 0xCC, 0x94},
	[IBRIGHTNESS_039NIT] = {0xB6, 0xCC, 0x94},
	[IBRIGHTNESS_041NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_044NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_047NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_050NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_053NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_056NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_060NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_064NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_068NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_072NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_077NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_082NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_087NIT] = {0xB6, 0xDC, 0x94},
	[IBRIGHTNESS_093NIT] = {0xB6, 0xDC, 0x93},
	[IBRIGHTNESS_098NIT] = {0xB6, 0xDC, 0x93},
	[IBRIGHTNESS_105NIT] = {0xB6, 0xDC, 0x93},
	[IBRIGHTNESS_111NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_119NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_126NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_134NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_143NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_152NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_162NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_172NIT] = {0xB6, 0xDC, 0x92},
	[IBRIGHTNESS_183NIT] = {0xB6, 0xDC, 0x91},
	[IBRIGHTNESS_195NIT] = {0xB6, 0xDC, 0x90},
	[IBRIGHTNESS_207NIT] = {0xB6, 0xDC, 0x8F},
	[IBRIGHTNESS_220NIT] = {0xB6, 0xDC, 0x8E},
	[IBRIGHTNESS_234NIT] = {0xB6, 0xDC, 0x8D},
	[IBRIGHTNESS_249NIT] = {0xB6, 0xDC, 0x8C},
	[IBRIGHTNESS_265NIT] = {0xB6, 0xDC, 0x8B},
	[IBRIGHTNESS_282NIT] = {0xB6, 0xDC, 0x8A},
	[IBRIGHTNESS_300NIT] = {0xB6, 0xDC, 0x88},
	[IBRIGHTNESS_316NIT] = {0xB6, 0xDC, 0x87},
	[IBRIGHTNESS_333NIT] = {0xB6, 0xDC, 0x86},
	[IBRIGHTNESS_360NIT] = {0xB6, 0xDC, 0x84}
};

struct elvss_otp_info {
	unsigned int	nit;
	int not_otp[TEMP_MAX];
};

struct elvss_otp_info elvss_otp_data[IBRIGHTNESS_MAX] = {
	[IBRIGHTNESS_005NIT] = {5,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_006NIT] = {6,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_007NIT] = {7,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_008NIT] = {8,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_009NIT] = {9,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_010NIT] = {10,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_011NIT] = {11,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_012NIT] = {12,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_013NIT] = {13,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_014NIT] = {14,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_015NIT] = {15,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_016NIT] = {16,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_017NIT] = {17,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_019NIT] = {19,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_020NIT] = {20,	{0x12, 0x18, 0x1F} },
	[IBRIGHTNESS_021NIT] = {21,	{0x12, 0x14, 0x1B} },
	[IBRIGHTNESS_022NIT] = {22,	{0x12, 0x14, 0x1A} },
	[IBRIGHTNESS_024NIT] = {24,	{0x12, 0x13, 0x18} },
	[IBRIGHTNESS_025NIT] = {25,	{0x12, 0x12, 0x17} },
	[IBRIGHTNESS_027NIT] = {27,	{0x12, 0x12, 0x15} },
	[IBRIGHTNESS_029NIT] = {29,	{0x12, 0x12, 0x13} },
};

static unsigned char AOR_TABLE[IBRIGHTNESS_MAX][AID_CMD_CNT] = {
	{0xB2, 0x00, 0x00, 0x04, 0xE6},
	{0xB2, 0x00, 0x00, 0x04, 0xE1},
	{0xB2, 0x00, 0x00, 0x04, 0xD4},
	{0xB2, 0x00, 0x00, 0x04, 0xC8},
	{0xB2, 0x00, 0x00, 0x04, 0xC4},
	{0xB2, 0x00, 0x00, 0x04, 0xB7},
	{0xB2, 0x00, 0x00, 0x04, 0xB2},
	{0xB2, 0x00, 0x00, 0x04, 0xA4},
	{0xB2, 0x00, 0x00, 0x04, 0xA1},
	{0xB2, 0x00, 0x00, 0x04, 0x95},
	{0xB2, 0x00, 0x00, 0x04, 0x88},
	{0xB2, 0x00, 0x00, 0x04, 0x83},
	{0xB2, 0x00, 0x00, 0x04, 0x77},
	{0xB2, 0x00, 0x00, 0x04, 0x64},
	{0xB2, 0x00, 0x00, 0x04, 0x58},
	{0xB2, 0x00, 0x00, 0x04, 0x52},
	{0xB2, 0x00, 0x00, 0x04, 0x44},
	{0xB2, 0x00, 0x00, 0x04, 0x31},
	{0xB2, 0x00, 0x00, 0x04, 0x24},
	{0xB2, 0x00, 0x00, 0x04, 0x09},
	{0xB2, 0x00, 0x00, 0x03, 0xF5},
	{0xB2, 0x00, 0x00, 0x03, 0xE7},
	{0xB2, 0x00, 0x00, 0x03, 0xD4},
	{0xB2, 0x00, 0x00, 0x03, 0xC2},
	{0xB2, 0x00, 0x00, 0x03, 0xA2},
	{0xB2, 0x00, 0x00, 0x03, 0x8F},
	{0xB2, 0x00, 0x00, 0x03, 0x75},
	{0xB2, 0x00, 0x00, 0x03, 0x54},
	{0xB2, 0x00, 0x00, 0x03, 0x35},
	{0xB2, 0x00, 0x00, 0x03, 0x15},
	{0xB2, 0x00, 0x00, 0x02, 0xF6},
	{0xB2, 0x00, 0x00, 0x02, 0xD6},
	{0xB2, 0x00, 0x00, 0x02, 0xA5},
	{0xB2, 0x00, 0x00, 0x02, 0x79},
	{0xB2, 0x00, 0x00, 0x02, 0x4F},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x02, 0x32},
	{0xB2, 0x00, 0x00, 0x01, 0xEA},
	{0xB2, 0x00, 0x00, 0x01, 0xAE},
	{0xB2, 0x00, 0x00, 0x01, 0x63},
	{0xB2, 0x00, 0x00, 0x01, 0x23},
	{0xB2, 0x00, 0x00, 0x00, 0xE5},
	{0xB2, 0x00, 0x00, 0x00, 0x92},
	{0xB2, 0x00, 0x00, 0x00, 0x44},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C},
	{0xB2, 0x00, 0x00, 0x00, 0x0C}		/* 360 nit */
};

/* platform brightness <-> acl opr and percent */
static unsigned int brightness_opr_table[ACL_STATUS_MAX][EXTEND_BRIGHTNESS + 1] = {
	{
		[0 ... UI_MAX_BRIGHTNESS - 1]				= OPR_STATUS_15P,
		[UI_MAX_BRIGHTNESS ... EXTEND_BRIGHTNESS]		= OPR_STATUS_OFF
	}, {
		[0 ... EXTEND_BRIGHTNESS - 1]				= OPR_STATUS_15P,
		[EXTEND_BRIGHTNESS ... EXTEND_BRIGHTNESS]		= OPR_STATUS_08P
	}
};

/* platform brightness <-> gamma level */
static unsigned int brightness_table[EXTEND_BRIGHTNESS + 1] = {
	[0 ... 5] = IBRIGHTNESS_005NIT,
	[6 ... 6] = IBRIGHTNESS_006NIT,
	[7 ... 7] = IBRIGHTNESS_007NIT,
	[8 ... 8] = IBRIGHTNESS_008NIT,
	[9 ... 9] = IBRIGHTNESS_009NIT,
	[10 ... 10] = IBRIGHTNESS_010NIT,
	[11 ... 11] = IBRIGHTNESS_011NIT,
	[12 ... 12] = IBRIGHTNESS_012NIT,
	[13 ... 13] = IBRIGHTNESS_013NIT,
	[14 ... 14] = IBRIGHTNESS_014NIT,
	[15 ... 15] = IBRIGHTNESS_015NIT,
	[16 ... 16] = IBRIGHTNESS_016NIT,
	[17 ... 17] = IBRIGHTNESS_017NIT,
	[18 ... 18] = IBRIGHTNESS_019NIT,
	[19 ... 19] = IBRIGHTNESS_020NIT,
	[20 ... 20] = IBRIGHTNESS_021NIT,
	[21 ... 21] = IBRIGHTNESS_022NIT,
	[22 ... 22] = IBRIGHTNESS_024NIT,
	[23 ... 23] = IBRIGHTNESS_025NIT,
	[24 ... 24] = IBRIGHTNESS_027NIT,
	[25 ... 25] = IBRIGHTNESS_029NIT,
	[26 ... 26] = IBRIGHTNESS_030NIT,
	[27 ... 27] = IBRIGHTNESS_032NIT,
	[28 ... 28] = IBRIGHTNESS_034NIT,
	[29 ... 29] = IBRIGHTNESS_037NIT,
	[30 ... 30] = IBRIGHTNESS_039NIT,
	[31 ... 32] = IBRIGHTNESS_041NIT,
	[33 ... 34] = IBRIGHTNESS_044NIT,
	[35 ... 36] = IBRIGHTNESS_047NIT,
	[37 ... 38] = IBRIGHTNESS_050NIT,
	[39 ... 40] = IBRIGHTNESS_053NIT,
	[41 ... 43] = IBRIGHTNESS_056NIT,
	[44 ... 46] = IBRIGHTNESS_060NIT,
	[47 ... 49] = IBRIGHTNESS_064NIT,
	[50 ... 52] = IBRIGHTNESS_068NIT,
	[53 ... 56] = IBRIGHTNESS_072NIT,
	[57 ... 59] = IBRIGHTNESS_077NIT,
	[60 ... 63] = IBRIGHTNESS_082NIT,
	[64 ... 67] = IBRIGHTNESS_087NIT,
	[68 ... 71] = IBRIGHTNESS_093NIT,
	[72 ... 76] = IBRIGHTNESS_098NIT,
	[77 ... 80] = IBRIGHTNESS_105NIT,
	[81 ... 86] = IBRIGHTNESS_111NIT,
	[87 ... 91] = IBRIGHTNESS_119NIT,
	[92 ... 97] = IBRIGHTNESS_126NIT,
	[98 ... 104] = IBRIGHTNESS_134NIT,
	[105 ... 110] = IBRIGHTNESS_143NIT,
	[111 ... 118] = IBRIGHTNESS_152NIT,
	[119 ... 125] = IBRIGHTNESS_162NIT,
	[126 ... 133] = IBRIGHTNESS_172NIT,
	[134 ... 142] = IBRIGHTNESS_183NIT,
	[143 ... 150] = IBRIGHTNESS_195NIT,
	[151 ... 160] = IBRIGHTNESS_207NIT,
	[161 ... 170] = IBRIGHTNESS_220NIT,
	[171 ... 181] = IBRIGHTNESS_234NIT,
	[182 ... 193] = IBRIGHTNESS_249NIT,
	[194 ... 205] = IBRIGHTNESS_265NIT,
	[206 ... 218] = IBRIGHTNESS_282NIT,
	[219 ... 229] = IBRIGHTNESS_300NIT,
	[230 ... 241] = IBRIGHTNESS_316NIT,
	[242 ... 254] = IBRIGHTNESS_333NIT,
	[255 ... EXTEND_BRIGHTNESS] = IBRIGHTNESS_360NIT
};

#endif /* __EA8061S_J7NEOPLUS_PARAM_H__ */
