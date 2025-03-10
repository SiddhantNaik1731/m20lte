/*
 *  Copyright (C) 2017, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef FINGERPRINT_H_
#define FINGERPRINT_H_

#include <linux/clk.h>
#include "fingerprint_sysfs.h"

/* fingerprint debug timer */
#define FPSENSOR_DEBUG_TIMER_SEC (10 * HZ)

/* For Sensor Type Check */
enum {
	SENSOR_OOO = -2,
	SENSOR_UNKNOWN,
	SENSOR_FAILED,
	SENSOR_VIPER,
	SENSOR_RAPTOR,
	SENSOR_EGIS,
	SENSOR_VIPER_WOG,
	SENSOR_NAMSAN,
	SENSOR_GOODIX,
	SENSOR_MAXIMUM,
};

#define SENSOR_STATUS_SIZE 9
static char sensor_status[SENSOR_STATUS_SIZE][10] = {"ooo", "unknown", "failed",
	"viper", "raptor", "egis", "viper_wog", "namsan", "goodix"};

/* For Finger Detect Mode */
enum {
	DETECT_NORMAL = 0,
	DETECT_ADM,			/* Always on Detect Mode */
};

#if defined(CONFIG_SOC_EXYNOS7570) || defined(CONFIG_SOC_EXYNOS7870) \
	|| defined(CONFIG_SOC_EXYNOS7880) || defined(CONFIG_SOC_EXYNOS8890) \
	|| defined(CONFIG_SOC_EXYNOS8895) || defined(CONFIG_SOC_EXYNOS9810)
#define FP_ENABLE_AP_CONFIG
#endif

#ifdef ENABLE_SENSORS_FPRINT_SECURE
#define MC_FC_FP_PM_SUSPEND ((uint32_t)(0x83000021))
#define MC_FC_FP_PM_RESUME ((uint32_t)(0x83000022))
#define MC_FC_FP_PM_SUSPEND_RETAIN ((uint32_t)(0x83000026))
#define MC_FC_FP_CS_SET ((uint32_t)(0x83000027))
#define MC_FC_FP_PM_SUSPEND_CS_HIGH ((uint32_t)(0x83000028))

extern int fpsensor_goto_suspend;
EXPORT_SYMBOL(fpsensor_goto_suspend);
#endif

#endif
