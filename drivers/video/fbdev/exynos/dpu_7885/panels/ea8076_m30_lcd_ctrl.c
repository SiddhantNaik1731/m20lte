/*
 * Copyright (c) Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/lcd.h>
#include <linux/backlight.h>
#include <linux/of_device.h>
#include <linux/ctype.h>
#include <video/mipi_display.h>
#include "../dsim.h"
#include "dsim_panel.h"
#include "../decon.h"
#include "../decon_notify.h"

#include "ea8076_m30_param.h"
#include "dd.h"

#ifdef CONFIG_DISPLAY_USE_INFO
#include "dpui.h"

#define	DPUI_VENDOR_NAME	"SDC"
#define DPUI_MODEL_NAME		"AMS638WZ01"
#endif

#define PANEL_STATE_SUSPENED	0
#define PANEL_STATE_RESUMED	1

#define LEVEL_IS_HBM(brightness)		(brightness > UI_MAX_BRIGHTNESS)

#define DSI_WRITE(cmd, size)		do {				\
	ret = dsim_write_hl_data(lcd, cmd, size);			\
	if (ret < 0)							\
		dev_err(&lcd->ld->dev, "%s: failed to write %s\n", __func__, #cmd);	\
} while (0)

#define get_bit(value, shift, width)	((value >> shift) & (GENMASK(width - 1, 0)))

union wrctrld_info {
	u32 value;
	struct {
		u8 bl_reg2;
		u8 bl_reg1;
		u8 hbm;
		u8 reserved;
	};
};

struct lcd_info {
	unsigned int			connected;
	unsigned int			brightness;
	unsigned int			current_elvss;
	unsigned int			current_acl;
	union wrctrld_info		current_wrctrld;
	unsigned int			state;

	struct lcd_device		*ld;
	struct backlight_device		*bd;
	struct device			svc_dev;

	unsigned char			**acl_table;
	unsigned char			**hbm_table;

	union {
		struct {
			u8		reserved;
			u8		id[LDI_LEN_ID];
		};
		u32			value;
	} id_info;
	unsigned char			date[LDI_LEN_DATE];
	unsigned int			coordinate[2];
	unsigned char			coordinates[20];
	unsigned char			manufacture_info[LDI_LEN_MANUFACTURE_INFO + LDI_LEN_MANUFACTURE_INFO_CELL_ID];
	unsigned char			rdnumpe;
	unsigned char			esderr;
	unsigned char			rddsdr;
	unsigned char			rddpm;
	unsigned char			rddsm;

	unsigned int			adaptive_control;
	int				lux;

	struct dsim_device		*dsim;
	struct mutex			lock;

	struct notifier_block		fb_notifier;

#ifdef CONFIG_DISPLAY_USE_INFO
	struct notifier_block		dpui_notif;
#endif

#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
	unsigned int			alpm;
	unsigned int			current_alpm;
	unsigned int			doze_state;

#if defined(CONFIG_SEC_FACTORY)
	unsigned int			prev_brightness;
	unsigned int			prev_alpm;
#endif
#endif
};

static int dsim_write_hl_data(struct lcd_info *lcd, const u8 *cmd, u32 cmdsize)
{
	int ret = 0;
	int retry = 2;

	if (!lcd->connected)
		return ret;

try_write:
	if (cmdsize == 1)
		ret = dsim_write_data(lcd->dsim, MIPI_DSI_DCS_SHORT_WRITE, cmd[0], 0);
	else if (cmdsize == 2)
		ret = dsim_write_data(lcd->dsim, MIPI_DSI_DCS_SHORT_WRITE_PARAM, cmd[0], cmd[1]);
	else
		ret = dsim_write_data(lcd->dsim, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)cmd, cmdsize);

	if (ret < 0) {
		if (--retry)
			goto try_write;
		else
			dev_err(&lcd->ld->dev, "%s: fail. %02x, ret: %d\n", __func__, cmd[0], ret);
	}

	return ret;
}

static int dsim_read_hl_data(struct lcd_info *lcd, u8 addr, u32 size, u8 *buf)
{
	int ret = 0, rx_size = 0;
	int retry = 2;

	if (!lcd->connected)
		return ret;

try_read:
	rx_size = dsim_read_data(lcd->dsim, MIPI_DSI_DCS_READ, (u32)addr, size, buf);
	dev_info(&lcd->ld->dev, "%s: %02x, %d, %d\n", __func__, addr, size, rx_size);
	if (rx_size != size) {
		if (--retry)
			goto try_read;
		else {
			dev_err(&lcd->ld->dev, "%s: fail. %02x, %d\n", __func__, addr, rx_size);
			ret = -EPERM;
		}
	}

	return ret;
}

static int dsim_panel_set_elvss(struct lcd_info *lcd, u8 force)
{
	int ret = 0;
	u8 elvss_value;

	elvss_value = elvss_table[lcd->brightness];

	SEQ_ELVSS_SET[LDI_OFFSET_ELVSS] = elvss_value;

	if (force)
		goto update;
	else if (lcd->current_elvss != elvss_value)
		goto update;
	else
		goto exit;

update:
	DSI_WRITE(SEQ_ELVSS_SET, ELVSS_CMD_CNT);
	lcd->current_elvss = elvss_value;
	dev_info(&lcd->ld->dev, "elvss: %x\n", lcd->current_elvss);

exit:
	return ret;
}

static int dsim_panel_set_wrctrld(struct lcd_info *lcd, u8 force)
{
	int ret = 0;
	unsigned char bl_reg[3] = {0, };
	union wrctrld_info wrctrld = {0, };
	unsigned char hbm_level = 0;

	hbm_level = LEVEL_IS_HBM(lcd->brightness);

	bl_reg[0] = LDI_REG_BRIGHTNESS;
	wrctrld.bl_reg1 = bl_reg[1] = get_bit(brightness_table[lcd->brightness], 8, 2);
	wrctrld.bl_reg2 = bl_reg[2] = get_bit(brightness_table[lcd->brightness], 0, 8);
	wrctrld.hbm = lcd->hbm_table[hbm_level][LDI_OFFSET_HBM];

	if (force || lcd->current_wrctrld.value != wrctrld.value) {
		DSI_WRITE(lcd->hbm_table[hbm_level], HBM_CMD_CNT);
		DSI_WRITE(bl_reg, ARRAY_SIZE(bl_reg));
		lcd->current_wrctrld.value = wrctrld.value;
	}

	return ret;
}

static int dsim_panel_set_acl(struct lcd_info *lcd, int force)
{
	int ret = 0, opr_status = ACL_STATUS_15P;
	unsigned int acl_value = 0;

	opr_status = brightness_opr_table[!!lcd->adaptive_control][lcd->brightness];
	acl_value = lcd->acl_table[opr_status][LDI_OFFSET_ACL];

	if (force)
		goto update;
	else if (lcd->current_acl != acl_value)
		goto update;
	else
		goto exit;

update:
	DSI_WRITE(lcd->acl_table[opr_status], ACL_CMD_CNT);
	lcd->current_acl = acl_value;
	dev_info(&lcd->ld->dev, "acl: %x, brightness: %d, adaptive_control: %d\n", lcd->current_acl, lcd->brightness, lcd->adaptive_control);

exit:
	return ret;
}

static int low_level_set_brightness(struct lcd_info *lcd, int force)
{
	int ret = 0;

	DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));

	dsim_panel_set_elvss(lcd, force);

	dsim_panel_set_wrctrld(lcd, force);

	dsim_panel_set_acl(lcd, force);

	DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

	return 0;
}

static int dsim_panel_set_brightness(struct lcd_info *lcd, int force)
{
	int ret = 0;

	mutex_lock(&lcd->lock);

#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
	if (lcd->doze_state) {
		dev_info(&lcd->ld->dev, "%s: brightness: %d, doze_state: %d, %d, %d\n", __func__, lcd->bd->props.brightness, lcd->doze_state, lcd->current_alpm, lcd->alpm);
		goto exit;
	}
#endif

 	lcd->brightness = lcd->bd->props.brightness;

	if (!force && lcd->state != PANEL_STATE_RESUMED) {
		dev_info(&lcd->ld->dev, "%s: brightness: %d, panel_state: %d\n", __func__, lcd->brightness, lcd->state);
		goto exit;
	}

	low_level_set_brightness(lcd, force);

	dev_info(&lcd->ld->dev, "brightness: %3d, %4d, %6x, lx: %d\n", lcd->brightness,
		brightness_table[lcd->brightness], lcd->current_wrctrld.value, lcd->lux);

exit:
	mutex_unlock(&lcd->lock);

	return ret;
}

static int panel_get_brightness(struct backlight_device *bd)
{
	struct lcd_info *lcd = bl_get_data(bd);

	return brightness_table[lcd->brightness];
}

static int panel_set_brightness(struct backlight_device *bd)
{
	int ret = 0;
	struct lcd_info *lcd = bl_get_data(bd);

	if (lcd->state == PANEL_STATE_RESUMED) {
		ret = dsim_panel_set_brightness(lcd, 0);
		if (ret < 0)
			dev_err(&lcd->ld->dev, "%s: failed to set brightness\n", __func__);
	}

	return ret;
}

static const struct backlight_ops panel_backlight_ops = {
	.get_brightness = panel_get_brightness,
	.update_status = panel_set_brightness,
};

static int ea8076_read_info(struct lcd_info *lcd, u8 reg, u32 len, u8 *buf)
{
	int ret = 0, i;

	ret = dsim_read_hl_data(lcd, reg, len, buf);
	if (ret < 0) {
		dev_err(&lcd->ld->dev, "%s: fail. %02x, ret: %d\n", __func__, reg, ret);
		goto exit;
	}

	dev_dbg(&lcd->ld->dev, "%s: %02xh\n", __func__, reg);
	for (i = 0; i < len; i++)
		dev_dbg(&lcd->ld->dev, "%02dth value is %02x, %3d\n", i + 1, buf[i], buf[i]);

exit:
	return ret;
}

static int ea8076_read_id(struct lcd_info *lcd)
{
	struct panel_private *priv = &lcd->dsim->priv;
	int ret = 0;

	lcd->id_info.value = 0;
	priv->lcdconnected = lcd->connected = lcdtype ? 1 : 0;

	ret = ea8076_read_info(lcd, LDI_REG_ID, LDI_LEN_ID, lcd->id_info.id);
	if (ret < 0 || !lcd->id_info.value) {
		priv->lcdconnected = lcd->connected = 0;
		dev_err(&lcd->ld->dev, "%s: connected lcd is invalid\n", __func__);
	}

	dev_info(&lcd->ld->dev, "%s: %x\n", __func__, cpu_to_be32(lcd->id_info.value));

	return ret;
}

static int ea8076_read_coordinate(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_GPARA_MANUFACTURE_INFO + LDI_LEN_MANUFACTURE_INFO] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_COORDINATE, ARRAY_SIZE(buf), buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);

	lcd->coordinate[0] = buf[LDI_GPARA_COORDINATE + 0] << 8 | buf[LDI_GPARA_COORDINATE + 1];	/* X */
	lcd->coordinate[1] = buf[LDI_GPARA_COORDINATE + 2] << 8 | buf[LDI_GPARA_COORDINATE + 4];	/* Y */

	scnprintf(lcd->coordinates, sizeof(lcd->coordinates), "%d %d\n", lcd->coordinate[0], lcd->coordinate[1]);

	memcpy(lcd->date, &buf[LDI_GPARA_DATE], LDI_LEN_DATE);

	memcpy(lcd->manufacture_info, &buf[LDI_GPARA_MANUFACTURE_INFO], LDI_LEN_MANUFACTURE_INFO);

	return ret;
}

static int ea8076_read_manufacture_info(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_GPARA_MANUFACTURE_INFO_CELL_ID + LDI_LEN_MANUFACTURE_INFO_CELL_ID] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_MANUFACTURE_INFO_CELL_ID, ARRAY_SIZE(buf), buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);

	memcpy(&lcd->manufacture_info[LDI_LEN_MANUFACTURE_INFO], &buf[LDI_GPARA_MANUFACTURE_INFO_CELL_ID], LDI_LEN_MANUFACTURE_INFO_CELL_ID);

	return ret;
}

#ifdef CONFIG_DISPLAY_USE_INFO
static int ea8076_inc_dpui_u32_field(struct lcd_info *lcd, enum dpui_key key, u32 value)
{
	if (lcd->connected)
		inc_dpui_u32_field(key, value);

	return 0;
}

static int ea8076_read_rdnumpe(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_LEN_RDNUMPE] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_RDNUMPE, LDI_LEN_RDNUMPE, buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);
	else
		ea8076_inc_dpui_u32_field(lcd, DPUI_KEY_PNDSIE, (buf[0] & LDI_PNDSIE_MASK));

	memcpy(&lcd->rdnumpe, buf, LDI_LEN_RDNUMPE);

	dev_info(&lcd->ld->dev, "%s: %x\n", __func__, lcd->rdnumpe);

	return ret;
}

static int ea8076_read_esderr(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_LEN_ESDERR] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_ESDERR, LDI_LEN_ESDERR, buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);
	else {
		/*ea8076_inc_dpui_u32_field(lcd, DPUI_KEY_PNELVDE, !!(buf[0] & LDI_PNELVDE_MASK));
		ea8076_inc_dpui_u32_field(lcd, DPUI_KEY_PNVLI1E, !!(buf[0] & LDI_PNVLI1E_MASK));
		ea8076_inc_dpui_u32_field(lcd, DPUI_KEY_PNVLO3E, !!(buf[0] & LDI_PNVLO3E_MASK));
		ea8076_inc_dpui_u32_field(lcd, DPUI_KEY_PNESDE, !!(buf[0] & LDI_PNESDE_MASK));*/
	}

	memcpy(&lcd->esderr, buf, LDI_LEN_ESDERR);

	dev_info(&lcd->ld->dev, "%s: %x\n", __func__, lcd->esderr);

	return ret;
}

static int ea8076_read_rddsdr(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_LEN_RDDSDR] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_RDDSDR, LDI_LEN_RDDSDR, buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);
	else
		ea8076_inc_dpui_u32_field(lcd, DPUI_KEY_PNSDRE, !(buf[0] & LDI_PNSDRE_MASK));

	memcpy(&lcd->rddsdr, buf, LDI_LEN_RDDSDR);

	dev_info(&lcd->ld->dev, "%s: %x\n", __func__, lcd->rddsdr);

	return ret;
}
#endif

static int ea8076_read_rddpm(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_LEN_RDDPM] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_RDDPM, LDI_LEN_RDDPM, buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);
	else
		memcpy(&lcd->rddpm, buf, LDI_LEN_RDDPM);

	dev_info(&lcd->ld->dev, "%s: %x\n", __func__, lcd->rddpm);

	return ret;
}

static int ea8076_read_rddsm(struct lcd_info *lcd)
{
	int ret = 0;
	unsigned char buf[LDI_LEN_RDDSM] = {0, };

	ret = ea8076_read_info(lcd, LDI_REG_RDDSM, LDI_LEN_RDDSM, buf);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: fail\n", __func__);
	else
		memcpy(&lcd->rddsm, buf, LDI_LEN_RDDSM);

	dev_info(&lcd->ld->dev, "%s: %x\n", __func__, lcd->rddsm);

	return ret;
}

#ifdef CONFIG_LOGGING_BIGDATA_BUG
unsigned int get_panel_bigdata(struct dsim_device *dsim)
{
	struct lcd_info *lcd = dsim->priv.par;
	unsigned int val = 0;

	lcd->rddpm = 0xff;
	lcd->rddsm = 0xff;

#ifdef CONFIG_DISPLAY_USE_INFO
	ea8076_read_rddpm(lcd);
	ea8076_read_rddsm(lcd);
#endif

	val = (lcd->rddpm  << 8) | lcd->rddsm;

	return val;
}
#endif

static int ea8076_read_init_info(struct lcd_info *lcd)
{
	int ret = 0;

	ea8076_read_id(lcd);

	DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));

	ea8076_read_coordinate(lcd);
	ea8076_read_manufacture_info(lcd);

	DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

	return ret;
}

static int ea8076_exit(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	ea8076_read_rddpm(lcd);
	ea8076_read_rddsm(lcd);

#ifdef CONFIG_DISPLAY_USE_INFO
	ea8076_read_rdnumpe(lcd);

	DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	ea8076_read_esderr(lcd);
	DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
#endif

	/* 2. Display Off (28h) */
	DSI_WRITE(SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));

	/* 3. Wait 20ms */
	msleep(20);

	/* 4. Sleep In (10h) */
	DSI_WRITE(SEQ_SLEEP_IN, ARRAY_SIZE(SEQ_SLEEP_IN));

	/* 5. Wait 120ms */
	msleep(120);

#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
	mutex_lock(&lcd->lock);
	lcd->current_alpm = ALPM_OFF;
	lcd->doze_state = 0;
	mutex_unlock(&lcd->lock);
#endif

	return ret;
}

static int ea8076_displayon(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	/* 12. Display On(29h) */
	DSI_WRITE(SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));

	return ret;
}

static int ea8076_init(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	/* 6. Sleep Out(11h) */
	DSI_WRITE(SEQ_SLEEP_OUT, ARRAY_SIZE(SEQ_SLEEP_OUT));

	/* 7. Wait 10ms */
	usleep_range(15000, 16000);

#if defined(CONFIG_SEC_FACTORY)
	ea8076_read_init_info(lcd);
#else
	ea8076_read_id(lcd);
#endif

#ifdef CONFIG_DISPLAY_USE_INFO
	ea8076_read_rddsdr(lcd);
#endif

	/* 8. Common Setting */
	/* 8.2 PAGE ADDRESS SET */
	DSI_WRITE(SEQ_PAGE_ADDR_SET, ARRAY_SIZE(SEQ_PAGE_ADDR_SET));

	/* Testkey Enable */
	DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	DSI_WRITE(SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));

	/* 8.1.3 FFC SET & ERR_FG SET */
	DSI_WRITE(SEQ_FFC_SET, ARRAY_SIZE(SEQ_FFC_SET));
	DSI_WRITE(SEQ_ERR_FG_SET, ARRAY_SIZE(SEQ_ERR_FG_SET));

	/* 8.4.2.3 ACL SET for fault MTP values */
	DSI_WRITE(SEQ_ACL_SETTING_1, ARRAY_SIZE(SEQ_ACL_SETTING_1));
	DSI_WRITE(SEQ_ACL_SETTING_2, ARRAY_SIZE(SEQ_ACL_SETTING_2));

	/* Testkey Disable */
	DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	DSI_WRITE(SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));

	/* 9. Brightness Setting */
	dsim_panel_set_brightness(lcd, 1);

	/* 8.1 TE(Vsync) ON/OFF */
	DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	DSI_WRITE(SEQ_TE_ON, ARRAY_SIZE(SEQ_TE_ON));
	DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

	/* 10. Wait 110ms */
	msleep(110);

	return ret;
}

#ifdef CONFIG_DISPLAY_USE_INFO
static int panel_dpui_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct lcd_info *lcd = NULL;
	struct dpui_info *dpui = data;
	char tbuf[MAX_DPUI_VAL_LEN];
	int size;
	unsigned int site, rework, poc, i, invalid = 0;
	unsigned char *m_info;

	struct seq_file m = {
		.buf = tbuf,
		.size = sizeof(tbuf) - 1,
	};

	if (dpui == NULL) {
		pr_err("%s: dpui is null\n", __func__);
		return NOTIFY_DONE;
	}

	lcd = container_of(self, struct lcd_info, dpui_notif);

	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%04d%02d%02d %02d%02d%02d",
			((lcd->date[0] & 0xF0) >> 4) + 2011, lcd->date[0] & 0xF, lcd->date[1] & 0x1F,
			lcd->date[2] & 0x1F, lcd->date[3] & 0x3F, lcd->date[4] & 0x3F);
	set_dpui_field(DPUI_KEY_MAID_DATE, tbuf, size);

	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", lcd->id_info.id[0]);
	set_dpui_field(DPUI_KEY_LCDID1, tbuf, size);
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", lcd->id_info.id[1]);
	set_dpui_field(DPUI_KEY_LCDID2, tbuf, size);
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", lcd->id_info.id[2]);
	set_dpui_field(DPUI_KEY_LCDID3, tbuf, size);
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%s_%s", DPUI_VENDOR_NAME, DPUI_MODEL_NAME);
	set_dpui_field(DPUI_KEY_DISP_MODEL, tbuf, size);

	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		lcd->date[0], lcd->date[1], lcd->date[2], lcd->date[3], lcd->date[4],
		lcd->date[5], lcd->date[6], (lcd->coordinate[0] & 0xFF00) >> 8, lcd->coordinate[0] & 0x00FF,
		(lcd->coordinate[1] & 0xFF00) >> 8, lcd->coordinate[1] & 0x00FF);
	set_dpui_field(DPUI_KEY_CELLID, tbuf, size);

	m_info = lcd->manufacture_info;
	site = get_bit(m_info[0], 4, 4);
	rework = get_bit(m_info[0], 0, 4);
	poc = get_bit(m_info[1], 0, 4);
	seq_printf(&m, "%d%d%d%02x%02x", site, rework, poc, m_info[2], m_info[3]);

	for (i = 4; i < LDI_LEN_MANUFACTURE_INFO + LDI_LEN_MANUFACTURE_INFO_CELL_ID; i++) {
		if (!isdigit(m_info[i]) && !isupper(m_info[i])) {
			invalid = 1;
			break;
		}
	}
	for (i = 4; !invalid && i < LDI_LEN_MANUFACTURE_INFO + LDI_LEN_MANUFACTURE_INFO_CELL_ID; i++)
		seq_printf(&m, "%c", m_info[i]);

	set_dpui_field(DPUI_KEY_OCTAID, tbuf, m.count);

	return NOTIFY_DONE;
}
#endif /* CONFIG_DISPLAY_USE_INFO */

static int fb_notifier_callback(struct notifier_block *self,
			unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	struct lcd_info *lcd = NULL;
	int fb_blank;

	switch (event) {
	case FB_EVENT_BLANK:
	case FB_EARLY_EVENT_BLANK:
		break;
	default:
		return NOTIFY_DONE;
	}

	lcd = container_of(self, struct lcd_info, fb_notifier);

	fb_blank = *(int *)evdata->data;

	dev_info(&lcd->ld->dev, "%s: %02lx, %d\n", __func__, event, fb_blank);

	if (evdata->info->node)
		return NOTIFY_DONE;

	if (event == FB_EVENT_BLANK && fb_blank == FB_BLANK_UNBLANK)
		ea8076_displayon(lcd);

	return NOTIFY_DONE;
}

static int ea8076_register_notifier(struct lcd_info *lcd)
{
	lcd->fb_notifier.notifier_call = fb_notifier_callback;
	decon_register_notifier(&lcd->fb_notifier);

#ifdef CONFIG_DISPLAY_USE_INFO
	lcd->dpui_notif.notifier_call = panel_dpui_notifier_callback;
	if (lcd->connected)
		dpui_logging_register(&lcd->dpui_notif, DPUI_TYPE_PANEL);
#endif

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	return 0;
}

static int ea8076_probe(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	lcd->bd->props.max_brightness = EXTEND_BRIGHTNESS;
	lcd->bd->props.brightness = UI_DEFAULT_BRIGHTNESS;

	lcd->state = PANEL_STATE_RESUMED;

	lcd->adaptive_control = ACL_STATUS_15P;
	lcd->lux = -1;

	lcd->acl_table = ACL_TABLE;
	lcd->hbm_table = HBM_TABLE;

	ret = ea8076_read_init_info(lcd);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: failed to init information\n", __func__);

	dsim_panel_set_brightness(lcd, 1);

	dev_info(&lcd->ld->dev, "- %s\n", __func__);

	return 0;
}

#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
int ea8076_setalpm(struct lcd_info *lcd, int mode)
{
	int ret = 0;

	/* 5. HLPM On setting Go to Page */
	/* 5.2.3 HLPM On Setting */
	switch (mode) {
	case HLPM_ON_LOW:
	case ALPM_ON_LOW:
		DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
		DSI_WRITE(SEQ_HLPM_ON_02, ARRAY_SIZE(SEQ_HLPM_ON_02));
		usleep_range(1, 2);
		DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
		dev_info(&lcd->ld->dev, "%s: HLPM_ON_02, %d\n", __func__, mode);
		break;
	case HLPM_ON_HIGH:
	case ALPM_ON_HIGH:
		DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
		DSI_WRITE(SEQ_HLPM_ON_60, ARRAY_SIZE(SEQ_HLPM_ON_60));
		usleep_range(1, 2);
		DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
		dev_info(&lcd->ld->dev, "%s: HLPM_ON_60, %d\n", __func__, mode);
		break;
	default:
		dev_info(&lcd->ld->dev, "%s: input is out of range: %d\n", __func__, mode);
		break;
	}

	return ret;
}

static int ea8076_enteralpm(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s: %d, %d, %d\n", __func__, lcd->current_alpm, lcd->alpm, lcd->lux);

	mutex_lock(&lcd->lock);

	if (lcd->state == PANEL_STATE_SUSPENED) {
		dev_info(&lcd->ld->dev, "%s: panel state is %d\n", __func__, lcd->state);
		goto exit;
	}

	if (lcd->current_alpm == lcd->alpm)
		goto exit;

	/* 2. Display Off(28h) */
	/* DSI_WRITE(SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF)); */

	/* 3. Image Write for HLPM Mode */
	/* 4. HLPM/ALPM On Setting */
	ret = ea8076_setalpm(lcd, lcd->alpm);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: failed to set alpm\n", __func__);

	/* 5. Wait 16.7ms */
	msleep(20);

	/* 6. Display On(29h) */
	/* DSI_WRITE(SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON)); */

	lcd->current_alpm = lcd->alpm;
exit:
	mutex_unlock(&lcd->lock);
	return ret;
}

static int ea8076_exitalpm(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s: %d, %d\n", __func__, lcd->current_alpm, lcd->alpm);

	mutex_lock(&lcd->lock);

	if (lcd->state == PANEL_STATE_SUSPENED) {
		dev_info(&lcd->ld->dev, "%s: panel state is %d\n", __func__, lcd->state);
		goto exit;
	}

	/* 2. Display Off(28h) */
	/* DSI_WRITE(SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF)); */

	/* 3. Wait 16.7ms */
	msleep(20);
	/* 4. Image Write for Normal Mode */
	/* 5. Wait 16.7ms */
	/* msleep(20); */

	/* 6. HLPM/ALPM Off Setting */
	DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	DSI_WRITE(SEQ_HLPM_OFF, ARRAY_SIZE(SEQ_HLPM_OFF));
	usleep_range(1, 2);
	DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

	dev_info(&lcd->ld->dev, "%s: HLPM_OFF\n", __func__);

	/* 7. Display On(29h) */
	/* DSI_WRITE(SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON)); */

	lcd->current_alpm = ALPM_OFF;
exit:
	mutex_unlock(&lcd->lock);
	return ret;
}
#endif

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "SDC_%02X%02X%02X\n", lcd->id_info.id[0], lcd->id_info.id[1], lcd->id_info.id[2]);

	return strlen(buf);
}

static ssize_t window_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%x %x %x\n", lcd->id_info.id[0], lcd->id_info.id[1], lcd->id_info.id[2]);

	return strlen(buf);
}

static ssize_t brightness_table_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	char *pos = buf;

	for (i = 0; i <= EXTEND_BRIGHTNESS; i++)
		pos += sprintf(pos, "%3d %4d\n", i, brightness_table[i]);

	return pos - buf;
}

static ssize_t color_coordinate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%u, %u\n", lcd->coordinate[0], lcd->coordinate[1]);

	return strlen(buf);
}

static ssize_t manufacture_date_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	u16 year;
	u8 month, day, hour, min, sec;
	u16 ms;

	year = ((lcd->date[0] & 0xF0) >> 4) + 2011;
	month = lcd->date[0] & 0xF;
	day = lcd->date[1] & 0x1F;
	hour = lcd->date[2] & 0x1F;
	min = lcd->date[3] & 0x3F;
	sec = lcd->date[4];
	ms = (lcd->date[5] << 8) | lcd->date[6];

	sprintf(buf, "%04d, %02d, %02d, %02d:%02d:%02d.%04d\n", year, month, day, hour, min, sec, ms);

	return strlen(buf);
}

static ssize_t cell_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
		lcd->date[0], lcd->date[1], lcd->date[2], lcd->date[3], lcd->date[4],
		lcd->date[5], lcd->date[6], (lcd->coordinate[0] & 0xFF00) >> 8, lcd->coordinate[0] & 0x00FF,
		(lcd->coordinate[1] & 0xFF00) >> 8, lcd->coordinate[1] & 0x00FF);

	return strlen(buf);
}

static ssize_t adaptive_control_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", lcd->adaptive_control);

	return strlen(buf);
}

static ssize_t adaptive_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int rc;
	unsigned int value;

	rc = kstrtouint(buf, 0, &value);
	if (rc < 0)
		return rc;

	if (lcd->adaptive_control != value) {
		dev_info(&lcd->ld->dev, "%s: %d, %d\n", __func__, lcd->adaptive_control, value);
		mutex_lock(&lcd->lock);
		lcd->adaptive_control = value;
		mutex_unlock(&lcd->lock);
		if (lcd->state == PANEL_STATE_RESUMED)
			dsim_panel_set_brightness(lcd, 1);
	}

	return size;
}

static ssize_t lux_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", lcd->lux);

	return strlen(buf);
}

static ssize_t lux_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = kstrtoint(buf, 0, &value);
	if (rc < 0)
		return rc;

	if (lcd->lux != value) {
		mutex_lock(&lcd->lock);
		lcd->lux = value;
		mutex_unlock(&lcd->lock);
	}

	return size;
}

static ssize_t octa_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	unsigned int site, rework, poc, i, invalid = 0;
	unsigned char *m_info;

	struct seq_file m = {
		.buf = buf,
		.size = PAGE_SIZE - 1,
	};

	m_info = lcd->manufacture_info;
	site = get_bit(m_info[0], 4, 4);
	rework = get_bit(m_info[0], 0, 4);
	poc = get_bit(m_info[1], 0, 4);
	seq_printf(&m, "%d%d%d%02x%02x", site, rework, poc, m_info[2], m_info[3]);

	for (i = 4; i < LDI_LEN_MANUFACTURE_INFO + LDI_LEN_MANUFACTURE_INFO_CELL_ID; i++) {
		if (!isdigit(m_info[i]) && !isupper(m_info[i])) {
			invalid = 1;
			break;
		}
	}
	for (i = 4; !invalid && i < LDI_LEN_MANUFACTURE_INFO + LDI_LEN_MANUFACTURE_INFO_CELL_ID; i++)
		seq_printf(&m, "%c", m_info[i]);

	seq_puts(&m, "\n");

	return strlen(buf);
}

#if defined(CONFIG_SEC_FACTORY)
static ssize_t xtalk_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	if (lcd->state != PANEL_STATE_RESUMED)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &value);
	if (ret < 0)
		return ret;

	if (value == 1) {
		DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
		DSI_WRITE(SEQ_XTALK_B0, ARRAY_SIZE(SEQ_XTALK_B0));
		DSI_WRITE(SEQ_XTALK_ON, ARRAY_SIZE(SEQ_XTALK_ON));
		DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	} else {
		DSI_WRITE(SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
		DSI_WRITE(SEQ_XTALK_B0, ARRAY_SIZE(SEQ_XTALK_B0));
		DSI_WRITE(SEQ_XTALK_OFF, ARRAY_SIZE(SEQ_XTALK_OFF));
		DSI_WRITE(SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	}

	dev_info(&lcd->ld->dev, "%s: %d\n", __func__, value);

	return size;
}
#endif

#ifdef CONFIG_DISPLAY_USE_INFO
/*
 * HW PARAM LOGGING SYSFS NODE
 */
static ssize_t dpui_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	update_dpui_log(DPUI_LOG_LEVEL_INFO, DPUI_TYPE_PANEL);
	ret = get_dpui_log(buf, DPUI_LOG_LEVEL_INFO, DPUI_TYPE_PANEL);
	if (ret < 0) {
		pr_err("%s failed to get log %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s\n", buf);
	return ret;
}

static ssize_t dpui_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == 'C' || buf[0] == 'c')
		clear_dpui_log(DPUI_LOG_LEVEL_INFO, DPUI_TYPE_PANEL);

	return size;
}

/*
 * [DEV ONLY]
 * HW PARAM LOGGING SYSFS NODE
 */
static ssize_t dpui_dbg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	update_dpui_log(DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_PANEL);
	ret = get_dpui_log(buf, DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_PANEL);
	if (ret < 0) {
		pr_err("%s failed to get log %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s\n", buf);
	return ret;
}

static ssize_t dpui_dbg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == 'C' || buf[0] == 'c')
		clear_dpui_log(DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_PANEL);

	return size;
}

static DEVICE_ATTR(dpui, 0660, dpui_show, dpui_store);
static DEVICE_ATTR(dpui_dbg, 0660, dpui_dbg_show, dpui_dbg_store);
#endif

#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
#if defined(CONFIG_SEC_FACTORY)
static ssize_t alpm_doze_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	struct dsim_device *dsim = lcd->dsim;
	struct decon_device *decon = get_decon_drvdata(0);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 0, &value);
	if (ret < 0)
		return ret;

	dev_info(&lcd->ld->dev, "%s: %d, %d, %d, %d\n", __func__, value, lcd->doze_state, lcd->current_alpm, lcd->alpm);

	if (value >= ALPM_MODE_MAX) {
		dev_err(&lcd->ld->dev, "%s: undefined alpm mode: %d\n", __func__, value);
		return -EINVAL;
	}

	mutex_lock(&lcd->lock);
	lcd->prev_alpm = lcd->alpm;
	lcd->alpm = value;
	mutex_unlock(&lcd->lock);

	switch (value) {
	case ALPM_OFF:
		if (lcd->prev_brightness) {
			mutex_lock(&lcd->lock);
			lcd->bd->props.brightness = lcd->prev_brightness;
			lcd->prev_brightness = 0;
			mutex_unlock(&lcd->lock);
		}
		mutex_lock(&decon->lock);
		call_panel_ops(dsim, exitalpm, dsim);
		mutex_unlock(&decon->lock);
		usleep_range(17000, 18000);
		call_panel_ops(dsim, displayon, dsim);
		ea8076_displayon(lcd);
		break;
	case ALPM_ON_LOW:
	case HLPM_ON_LOW:
	case ALPM_ON_HIGH:
	case HLPM_ON_HIGH:
		if (lcd->prev_alpm == ALPM_OFF) {
			mutex_lock(&lcd->lock);
			lcd->prev_brightness = lcd->bd->props.brightness;
			lcd->bd->props.brightness = 0;
			mutex_unlock(&lcd->lock);
		}
		mutex_lock(&decon->lock);
		call_panel_ops(dsim, enteralpm, dsim);
		mutex_unlock(&decon->lock);
		usleep_range(17000, 18000);
		ea8076_displayon(lcd);
		break;
	}

	return size;
}
#else
static ssize_t alpm_doze_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	struct dsim_device *dsim = lcd->dsim;
	struct decon_device *decon = get_decon_drvdata(0);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 0, &value);
	if (ret < 0)
		return ret;

	dev_info(&lcd->ld->dev, "%s: %d, %d, %d, %d\n", __func__, value, lcd->doze_state, lcd->current_alpm, lcd->alpm);

	if (value >= ALPM_MODE_MAX) {
		dev_err(&lcd->ld->dev, "%s: undefined alpm mode: %d\n", __func__, value);
		return -EINVAL;
	}

	mutex_lock(&decon->lock);

	if (lcd->alpm != value) {
		mutex_lock(&lcd->lock);
		lcd->alpm = value;
		mutex_unlock(&lcd->lock);

		if (lcd->doze_state)
			call_panel_ops(dsim, enteralpm, dsim);
	}

	mutex_unlock(&decon->lock);

	return size;
}
#endif

static ssize_t alpm_doze_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%d, %d\n", lcd->current_alpm, lcd->alpm);

	return strlen(buf);
}

static DEVICE_ATTR(alpm, 0664, alpm_doze_show, alpm_doze_store);
#endif

static DEVICE_ATTR(lcd_type, 0444, lcd_type_show, NULL);
static DEVICE_ATTR(window_type, 0444, window_type_show, NULL);
static DEVICE_ATTR(cell_id, 0444, cell_id_show, NULL);
static DEVICE_ATTR(brightness_table, 0444, brightness_table_show, NULL);
static DEVICE_ATTR(color_coordinate, 0444, color_coordinate_show, NULL);
static DEVICE_ATTR(manufacture_date, 0444, manufacture_date_show, NULL);
static DEVICE_ATTR(adaptive_control, 0664, adaptive_control_show, adaptive_control_store);
static DEVICE_ATTR(lux, 0644, lux_show, lux_store);
static DEVICE_ATTR(octa_id, 0444, octa_id_show, NULL);
static DEVICE_ATTR(SVC_OCTA, 0444, cell_id_show, NULL);
static DEVICE_ATTR(SVC_OCTA_CHIPID, 0444, octa_id_show, NULL);
#if defined(CONFIG_SEC_FACTORY)
static DEVICE_ATTR(xtalk_mode, 0644, NULL, xtalk_mode_store);
#endif

static struct attribute *lcd_sysfs_attributes[] = {
	&dev_attr_lcd_type.attr,
	&dev_attr_window_type.attr,
	&dev_attr_cell_id.attr,
	&dev_attr_color_coordinate.attr,
	&dev_attr_manufacture_date.attr,
	&dev_attr_brightness_table.attr,
	&dev_attr_adaptive_control.attr,
	&dev_attr_lux.attr,
	&dev_attr_octa_id.attr,
#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
	&dev_attr_alpm.attr,
#endif
#if defined(CONFIG_SEC_FACTORY)
	&dev_attr_xtalk_mode.attr,
#endif
#ifdef CONFIG_DISPLAY_USE_INFO
	&dev_attr_dpui.attr,
	&dev_attr_dpui_dbg.attr,
#endif
	NULL,
};

static const struct attribute_group lcd_sysfs_attr_group = {
	.attrs = lcd_sysfs_attributes,
};

static void lcd_init_svc(struct lcd_info *lcd)
{
	struct device *dev = &lcd->svc_dev;
	struct kobject *top_kobj = &lcd->ld->dev.kobj.kset->kobj;
	struct kernfs_node *kn = kernfs_find_and_get(top_kobj->sd, "svc");
	struct kobject *svc_kobj = NULL;
	char *buf = NULL, *path = NULL;
	int ret = 0;

	svc_kobj = kn ? kn->priv : kobject_create_and_add("svc", top_kobj);
	if (!svc_kobj)
		return;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (buf) {
		path = kernfs_path(svc_kobj->sd, buf, PATH_MAX);
		dev_info(&lcd->ld->dev, "%s: %s %s\n", __func__, path, !kn ? "create" : "");
		kfree(buf);
	}

	dev->kobj.parent = svc_kobj;
	dev_set_name(dev, "OCTA");
	dev_set_drvdata(dev, lcd);
	ret = device_register(dev);
	if (ret < 0) {
		dev_info(&lcd->ld->dev, "%s: device_register fail\n", __func__);
		return;
	}

	device_create_file(dev, &dev_attr_SVC_OCTA);
	device_create_file(dev, &dev_attr_SVC_OCTA_CHIPID);

	if (kn)
		kernfs_put(kn);
}

static void lcd_init_sysfs(struct lcd_info *lcd)
{
	int ret = 0;

	ret = sysfs_create_group(&lcd->ld->dev.kobj, &lcd_sysfs_attr_group);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add lcd sysfs\n");

	lcd_init_svc(lcd);

	init_debugfs_backlight(lcd->bd, brightness_table, NULL);
}

static int dsim_panel_probe(struct dsim_device *dsim)
{
	int ret = 0;
	struct lcd_info *lcd;

	dsim->priv.par = lcd = kzalloc(sizeof(struct lcd_info), GFP_KERNEL);
	if (!lcd) {
		pr_err("%s: failed to allocate for lcd\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	lcd->ld = lcd_device_register("panel", dsim->dev, lcd, NULL);
	if (IS_ERR(lcd->ld)) {
		pr_err("%s: failed to register lcd device\n", __func__);
		ret = PTR_ERR(lcd->ld);
		goto exit;
	}

	lcd->bd = backlight_device_register("panel", dsim->dev, lcd, &panel_backlight_ops, NULL);
	if (IS_ERR(lcd->bd)) {
		pr_err("%s: failed to register backlight device\n", __func__);
		ret = PTR_ERR(lcd->bd);
		goto exit;
	}

	mutex_init(&lcd->lock);

	lcd->dsim = dsim;
	ret = ea8076_probe(lcd);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "%s: failed to probe panel\n", __func__);

	lcd_init_sysfs(lcd);

	ea8076_register_notifier(lcd);

	dev_info(&lcd->ld->dev, "%s: %s: done\n", kbasename(__FILE__), __func__);

exit:
	return ret;
}

static int dsim_panel_displayon(struct dsim_device *dsim)
{
	struct lcd_info *lcd = dsim->priv.par;

	dev_info(&lcd->ld->dev, "+ %s: %d\n", __func__, lcd->state);

	if (lcd->state == PANEL_STATE_SUSPENED) {
		ea8076_init(lcd);

		mutex_lock(&lcd->lock);
		lcd->state = PANEL_STATE_RESUMED;
		mutex_unlock(&lcd->lock);
	}

	dev_info(&lcd->ld->dev, "- %s: %d, %d\n", __func__, lcd->state, lcd->connected);

	return 0;
}

static int dsim_panel_suspend(struct dsim_device *dsim)
{
	struct lcd_info *lcd = dsim->priv.par;

	dev_info(&lcd->ld->dev, "+ %s: %d\n", __func__, lcd->state);

	if (lcd->state == PANEL_STATE_SUSPENED)
		goto exit;

	ea8076_exit(lcd);

	mutex_lock(&lcd->lock);
	lcd->state = PANEL_STATE_SUSPENED;
	mutex_unlock(&lcd->lock);

	dev_info(&lcd->ld->dev, "- %s: %d, %d\n", __func__, lcd->state, lcd->connected);

exit:
	return 0;
}

#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
static int dsim_panel_enteralpm(struct dsim_device *dsim)
{
	struct lcd_info *lcd = dsim->priv.par;

	dev_info(&lcd->ld->dev, "+ %s: %d\n", __func__, lcd->state);

	if (lcd->state == PANEL_STATE_SUSPENED) {
		ea8076_init(lcd);

		mutex_lock(&lcd->lock);
		lcd->state = PANEL_STATE_RESUMED;
		mutex_unlock(&lcd->lock);
	}

	ea8076_enteralpm(lcd);

	mutex_lock(&lcd->lock);
	lcd->doze_state = 1;
	mutex_unlock(&lcd->lock);

	dev_info(&lcd->ld->dev, "- %s: %d, %d\n", __func__, lcd->state, lcd->connected);

	return 0;
}

static int dsim_panel_exitalpm(struct dsim_device *dsim)
{
	struct lcd_info *lcd = dsim->priv.par;

	dev_info(&lcd->ld->dev, "+ %s: %d\n", __func__, lcd->state);

	if (lcd->state == PANEL_STATE_SUSPENED) {
		ea8076_init(lcd);

		mutex_lock(&lcd->lock);
		lcd->state = PANEL_STATE_RESUMED;
		mutex_unlock(&lcd->lock);
	}

	ea8076_exitalpm(lcd);

	mutex_lock(&lcd->lock);
	lcd->doze_state = 0;
	mutex_unlock(&lcd->lock);

	dsim_panel_set_brightness(lcd, 1);

	dev_info(&lcd->ld->dev, "- %s: %d, %d\n", __func__, lcd->state, lcd->connected);

	return 0;
}
#endif

struct dsim_lcd_driver ea8076_mipi_lcd_driver = {
	.probe		= dsim_panel_probe,
	.displayon	= dsim_panel_displayon,
	.suspend	= dsim_panel_suspend,
#if defined(CONFIG_EXYNOS_SUPPORT_DOZE)
	.enteralpm	= dsim_panel_enteralpm,
	.exitalpm	= dsim_panel_exitalpm,
#endif
};
