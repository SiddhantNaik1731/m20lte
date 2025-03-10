/*
 * Samsung EXYNOS FIMC-IS (Imaging Subsystem) driver
 *
 * Copyright (C) 2014 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "fimc-is-hw-mcscaler-v2.h"
#include "api/fimc-is-hw-api-mcscaler-v2.h"
#include "../interface/fimc-is-interface-ischain.h"
#include "fimc-is-param.h"
#include "fimc-is-err.h"

spinlock_t	shared_output_slock;
static ulong hw_mcsc_out_configured = 0xFFFF;
#define HW_MCSC_OUT_CLEARED_ALL (0x3F)

static int fimc_is_hw_mcsc_rdma_cfg(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame);
static void fimc_is_hw_mcsc_wdma_cfg(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame);
static void fimc_is_hw_mcsc_size_dump(struct fimc_is_hw_ip *hw_ip);

static const struct djag_setfile_contents init_djag_cfgs = {
	.xfilter_dejagging_coeff_cfg = {
		.xfilter_dejagging_weight0 = 3,
		.xfilter_dejagging_weight1 = 4,
		.xfilter_hf_boost_weight = 4,
		.center_hf_boost_weight = 2,
		.diagonal_hf_boost_weight = 3,
		.center_weighted_mean_weight = 3,
	},
	.thres_1x5_matching_cfg = {
		.thres_1x5_matching_sad = 128,
		.thres_1x5_abshf = 512,
	},
	.thres_shooting_detect_cfg = {
		.thres_shooting_llcrr = 255,
		.thres_shooting_lcr = 255,
		.thres_shooting_neighbor = 255,
		.thres_shooting_uucdd = 80,
		.thres_shooting_ucd = 80,
		.min_max_weight = 2,
	},
	.lfsr_seed_cfg = {
		.lfsr_seed_0 = 44257,
		.lfsr_seed_1 = 4671,
		.lfsr_seed_2 = 47792,
	},
	.dither_cfg = {
		.dither_value = {0, 0, 1, 2, 3, 4, 6, 7, 8},
		.sat_ctrl = 5,
		.dither_sat_thres = 1000,
		.dither_thres = 5,
	},
	.cp_cfg = {
		.cp_hf_thres = 40,
		.cp_arbi_max_cov_offset = 0,
		.cp_arbi_max_cov_shift = 6,
		.cp_arbi_denom = 7,
		.cp_arbi_mode = 1,
	},
};

static int fimc_is_hw_mcsc_handle_interrupt(u32 id, void *context)
{
	struct fimc_is_hardware *hardware;
	struct fimc_is_hw_ip *hw_ip = NULL;
	struct fimc_is_group *head;
	u32 status, intr_mask, intr_status;
	bool err_intr_flag = false;
	int ret = 0;
	u32 hl = 0, vl = 0;
	u32 instance;
	u32 hw_fcount, index;
	struct mcs_param *param;
	bool flag_clk_gate = false;

	hw_ip = (struct fimc_is_hw_ip *)context;
	hardware = hw_ip->hardware;
	hw_fcount = atomic_read(&hw_ip->fcount);
	instance = atomic_read(&hw_ip->instance);
	param = &hw_ip->region[instance]->parameter.mcs;

	fimc_is_scaler_get_input_status(hw_ip->regs, hw_ip->id, &hl, &vl);
	/* read interrupt status register (sc_intr_status) */
	intr_mask = fimc_is_scaler_get_intr_mask(hw_ip->regs, hw_ip->id);
	intr_status = fimc_is_scaler_get_intr_status(hw_ip->regs, hw_ip->id);
	status = (~intr_mask) & intr_status;

	fimc_is_scaler_clear_intr_src(hw_ip->regs, hw_ip->id, status);

	if (!test_bit(HW_RUN, &hw_ip->state)) {
		mserr_hw("HW disabled!! interrupt(0x%x)", instance, hw_ip, status);
		goto p_err;
	}

	if (status & (1 << INTR_MC_SCALER_OVERFLOW)) {
		mserr_hw("Overflow!! (0x%x)", instance, hw_ip, status);
		err_intr_flag = true;
	}

	if (status & (1 << INTR_MC_SCALER_OUTSTALL)) {
		mserr_hw("Output Block BLOCKING!! (0x%x)", instance, hw_ip, status);
		err_intr_flag = true;
	}

	if (status & (1 << INTR_MC_SCALER_INPUT_VERTICAL_UNF)) {
		mserr_hw("Input OTF Vertical Underflow!! (0x%x)", instance, hw_ip, status);
		err_intr_flag = true;
	}

	if (status & (1 << INTR_MC_SCALER_INPUT_VERTICAL_OVF)) {
		mserr_hw("Input OTF Vertical Overflow!! (0x%x)", instance, hw_ip, status);
		err_intr_flag = true;
	}

	if (status & (1 << INTR_MC_SCALER_INPUT_HORIZONTAL_UNF)) {
		mserr_hw("Input OTF Horizontal Underflow!! (0x%x)", instance, hw_ip, status);
		err_intr_flag = true;
	}

	if (status & (1 << INTR_MC_SCALER_INPUT_HORIZONTAL_OVF)) {
		mserr_hw("Input OTF Horizontal Overflow!! (0x%x)", instance, hw_ip, status);
		err_intr_flag = true;
	}

	if (status & (1 << INTR_MC_SCALER_WDMA_FINISH))
		mserr_hw("Disabeld interrupt occurred! WDAM FINISH!! (0x%x)", instance, hw_ip, status);

	if (status & (1 << INTR_MC_SCALER_FRAME_END)) {
		atomic_inc(&hw_ip->count.fe);
		hw_ip->cur_e_int++;
		if (hw_ip->cur_e_int >= hw_ip->num_buffers) {
			fimc_is_hw_mcsc_frame_done(hw_ip, NULL, IS_SHOT_SUCCESS);

			if (!atomic_read(&hardware->streaming[hardware->sensor_position[instance]]))
				sinfo_hw("[F:%d]F.E\n", hw_ip, hw_fcount);

			atomic_set(&hw_ip->status.Vvalid, V_BLANK);
			if (atomic_read(&hw_ip->count.fs) < atomic_read(&hw_ip->count.fe)) {
				mserr_hw("fs(%d), fe(%d), dma(%d), status(0x%x)", instance, hw_ip,
					atomic_read(&hw_ip->count.fs),
					atomic_read(&hw_ip->count.fe),
					atomic_read(&hw_ip->count.dma), status);
			}

			wake_up(&hw_ip->status.wait_queue);
			head = GET_HEAD_GROUP_IN_DEVICE(FIMC_IS_DEVICE_ISCHAIN, hw_ip->group[instance]);
			if (!test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state))
				up(&hw_ip->smp_resource);
			flag_clk_gate = true;
			hw_ip->mframe = NULL;
		}
	}

	if (status & (1 << INTR_MC_SCALER_FRAME_START)) {
		atomic_inc(&hw_ip->count.fs);
		hw_ip->cur_s_int++;
		if (hw_ip->cur_s_int == 1) {
			hw_ip->debug_index[1] = hw_ip->debug_index[0] % DEBUG_FRAME_COUNT;
			index = hw_ip->debug_index[1];
			hw_ip->debug_info[index].fcount = hw_ip->debug_index[0];
			hw_ip->debug_info[index].cpuid[DEBUG_POINT_FRAME_START] = raw_smp_processor_id();
			hw_ip->debug_info[index].time[DEBUG_POINT_FRAME_START] = cpu_clock(raw_smp_processor_id());
			if (!atomic_read(&hardware->streaming[hardware->sensor_position[instance]]))
				msinfo_hw("[F:%d]F.S\n", instance, hw_ip, hw_fcount);

			if (param->input.dma_cmd == DMA_INPUT_COMMAND_ENABLE) {
				fimc_is_hardware_frame_start(hw_ip, instance);
			} else {
				clear_bit(HW_CONFIG, &hw_ip->state);
				atomic_set(&hw_ip->status.Vvalid, V_VALID);
			}
		}

		/* for set shadow register write start */
		head = GET_HEAD_GROUP_IN_DEVICE(FIMC_IS_DEVICE_ISCHAIN, hw_ip->group[instance]);
		if (test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state))
			fimc_is_scaler_set_shadow_ctrl(hw_ip->regs, hw_ip->id, SHADOW_WRITE_START);

		if (hw_ip->cur_s_int < hw_ip->num_buffers) {
			if (hw_ip->mframe) {
				struct fimc_is_frame *mframe = hw_ip->mframe;
				mframe->cur_buf_index = hw_ip->cur_s_int;
				/* WDMA cfg */
				fimc_is_hw_mcsc_wdma_cfg(hw_ip, mframe);

				/* RDMA cfg */
				if (param->input.dma_cmd == DMA_INPUT_COMMAND_ENABLE) {
					ret = fimc_is_hw_mcsc_rdma_cfg(hw_ip, mframe);
					if (ret) {
						mserr_hw("[F:%d]mcsc rdma_cfg failed\n",
							mframe->instance, hw_ip, mframe->fcount);
						return ret;
					}
				}
			} else {
				serr_hw("mframe is null(s:%d, e:%d, t:%d)", hw_ip,
					hw_ip->cur_s_int, hw_ip->cur_e_int, hw_ip->num_buffers);
			}
		} else {
			struct fimc_is_group *group;
			group = hw_ip->group[instance];
			/*
			 * In case of M2M mcsc, just supports only one buffering.
			 * So, in start irq, "setting to stop mcsc for N + 1" should be assigned.
			 *
			 * TODO: Don't touch global control, but we don't know how to be mapped
			 *       with group-id and scX_ctrl.
			 */
			if (!test_bit(FIMC_IS_GROUP_OTF_INPUT, &group->state))
				fimc_is_scaler_stop(hw_ip->regs, hw_ip->id);
		}
	}

	/* for handle chip dependant intr */
	err_intr_flag |= fimc_is_scaler_handle_extended_intr(status);

	if (err_intr_flag) {
		msinfo_hw("[F:%d] Ocurred error interrupt (%d,%d) status(0x%x)\n",
			instance, hw_ip, hw_fcount, hl, vl, status);
		fimc_is_scaler_dump(hw_ip->regs);
		fimc_is_hardware_size_dump(hw_ip);
	}

	if (flag_clk_gate)
		CALL_HW_OPS(hw_ip, clk_gate, instance, false, false);

p_err:
	return ret;
}

void fimc_is_hw_mcsc_hw_info(struct fimc_is_hw_ip *hw_ip, struct fimc_is_hw_mcsc_cap *cap)
{
	int i = 0;

	if (!(hw_ip && cap))
		return;

	sinfo_hw("==== h/w info(ver:0x%X) ====\n", hw_ip, cap->hw_ver);
	info_hw("[IN] max_out:%d, in(otf/dma):%d/%d, hwfc:%d, tdnr:%d\n",
			cap->max_output, cap->in_otf, cap->in_dma, cap->hwfc, cap->tdnr);
	for (i = MCSC_OUTPUT0; i < cap->max_output; i++)
		info_hw("[OUT%d] out(otf/dma):%d/%d, hwfc:%d\n", i,
			cap->out_otf[i], cap->out_dma[i], cap->out_hwfc[i]);

	sinfo_hw("========================\n", hw_ip);
}

void get_mcsc_hw_ip(struct fimc_is_hardware *hardware, struct fimc_is_hw_ip **hw_ip0, struct fimc_is_hw_ip **hw_ip1)
{
	int hw_slot = -1;

	hw_slot = fimc_is_hw_slot_id(DEV_HW_MCSC0);
	if (valid_hw_slot_id(hw_slot))
		*hw_ip0 = &hardware->hw_ip[hw_slot];

	hw_slot = fimc_is_hw_slot_id(DEV_HW_MCSC1);
	if (valid_hw_slot_id(hw_slot))
		*hw_ip1 = &hardware->hw_ip[hw_slot];
}

static int fimc_is_hw_mcsc_open(struct fimc_is_hw_ip *hw_ip, u32 instance,
	struct fimc_is_group *group)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap;
	struct fimc_is_hardware *hardware;
	struct fimc_is_hw_ip *hw_ip0 = NULL, *hw_ip1 = NULL;
	u32 output_id;
	int i;

	BUG_ON(!hw_ip);

	if (test_bit(HW_OPEN, &hw_ip->state))
		return 0;

	frame_manager_probe(hw_ip->framemgr, FRAMEMGR_ID_HW | (1 << hw_ip->id), "HWMCS");
	frame_manager_probe(hw_ip->framemgr_late, FRAMEMGR_ID_HW | (1 << hw_ip->id) | 0xF000, "HWMCS LATE");
	frame_manager_open(hw_ip->framemgr, FIMC_IS_MAX_HW_FRAME);
	frame_manager_open(hw_ip->framemgr_late, FIMC_IS_MAX_HW_FRAME_LATE);

	hw_ip->priv_info = vzalloc(sizeof(struct fimc_is_hw_mcsc));
	if(!hw_ip->priv_info) {
		mserr_hw("hw_ip->priv_info(null)", instance, hw_ip);
		ret = -ENOMEM;
		goto err_alloc;
	}

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	hw_mcsc->instance = FIMC_IS_STREAM_COUNT;

	cap = GET_MCSC_HW_CAP(hw_ip);

	/* get the mcsc hw info */
	ret = fimc_is_hw_query_cap((void *)cap, hw_ip->id);
	if (ret) {
		mserr_hw("failed to get hw info", instance, hw_ip);
		ret = -EINVAL;
		goto err_query_cap;
	}

	/* print hw info */
	fimc_is_hw_mcsc_hw_info(hw_ip, cap);

	hw_ip->mframe = NULL;
	atomic_set(&hw_ip->status.Vvalid, V_BLANK);
	set_bit(HW_OPEN, &hw_ip->state);
	msdbg_hw(2, "open: [G:0x%x], framemgr[%s]", instance, hw_ip,
		GROUP_ID(group->id), hw_ip->framemgr->name);

	hardware = hw_ip->hardware;
	get_mcsc_hw_ip(hardware, &hw_ip0, &hw_ip1);

	for (i = 0; i < SENSOR_POSITION_MAX; i++) {
		hw_mcsc->applied_setfile[i] = NULL;
	}

	if (cap->enable_shared_output) {
		if (hw_ip0 && test_bit(HW_RUN, &hw_ip0->state))
			return 0;

		if (hw_ip1 && test_bit(HW_RUN, &hw_ip1->state))
			return 0;
	}

	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++)
		set_bit(output_id, &hw_mcsc_out_configured);
	clear_bit(HW_MCSC_OUT_CLEARED_ALL, &hw_mcsc_out_configured);

	msinfo_hw("mcsc_open: done, (0x%lx)\n", instance, hw_ip, hw_mcsc_out_configured);

	return 0;

err_query_cap:
	vfree(hw_ip->priv_info);
err_alloc:
	frame_manager_close(hw_ip->framemgr);
	frame_manager_close(hw_ip->framemgr_late);
	return ret;
}

static int fimc_is_hw_mcsc_init(struct fimc_is_hw_ip *hw_ip, u32 instance,
	struct fimc_is_group *group, bool flag, u32 module_id)
{
	int ret = 0;
	u32 entry, output_id;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap;
	struct fimc_is_subdev *subdev;
	struct fimc_is_video_ctx *vctx;
	struct fimc_is_video *video;

	BUG_ON(!hw_ip);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	hw_mcsc->rep_flag[instance] = flag;

	cap = GET_MCSC_HW_CAP(hw_ip);
	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		if (test_bit(output_id, &hw_mcsc->out_en)) {
			msdbg_hw(2, "already set output(%d)\n", instance, hw_ip, output_id);
			continue;
		}

		entry = GET_ENTRY_FROM_OUTPUT_ID(output_id);
		subdev = group->subdev[entry];
		if (!subdev)
			continue;

		vctx = subdev->vctx;
		if (!vctx) {
			continue;
		}

		video = vctx->video;
		if (!video) {
			mserr_hw("video is NULL. entry(%d)", instance, hw_ip, entry);
			BUG();
		}
		set_bit(output_id, &hw_mcsc->out_en);
	}

	fimc_is_hw_mcsc_djag_init(hw_ip);

	set_bit(HW_INIT, &hw_ip->state);
	msdbg_hw(2, "init: out_en[0x%lx]\n", instance, hw_ip, hw_mcsc->out_en);

	return ret;
}

static int fimc_is_hw_mcsc_close(struct fimc_is_hw_ip *hw_ip, u32 instance)
{
	int ret = 0;
	u32 output_id;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap;

	BUG_ON(!hw_ip);

	if (!test_bit(HW_OPEN, &hw_ip->state))
		return 0;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	cap = GET_MCSC_HW_CAP(hw_ip);

	/* clear out_en bit */
	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		if (test_bit(output_id, &hw_mcsc->out_en))
			clear_bit(output_id, &hw_mcsc->out_en);
	}

	vfree(hw_ip->priv_info);
	frame_manager_close(hw_ip->framemgr);
	frame_manager_close(hw_ip->framemgr_late);

	clear_bit(HW_OPEN, &hw_ip->state);
	msinfo_hw("close (%d)\n", instance, hw_ip, atomic_read(&hw_ip->rsccount));

	return ret;
}

static int fimc_is_hw_mcsc_enable(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	int ret = 0;
	ulong flag = 0;
	struct mcs_param *mcs_param;

	BUG_ON(!hw_ip);
	BUG_ON(!hw_ip->priv_info);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -EINVAL;
	}

	if (test_bit(HW_RUN, &hw_ip->state))
		return ret;

	msdbg_hw(2, "enable: start, (0x%lx)\n", instance, hw_ip, hw_mcsc_out_configured);

	mcs_param = &hw_ip->region[instance]->parameter.mcs;

	spin_lock_irqsave(&shared_output_slock, flag);
	ret = fimc_is_hw_mcsc_reset(hw_ip);
	if (ret != 0) {
		mserr_hw("MCSC sw reset fail", instance, hw_ip);
		spin_unlock_irqrestore(&shared_output_slock, flag);
		return -ENODEV;
	}

	/* input source select 0: otf, 1:rdma */
	if (test_bit(FIMC_IS_GROUP_OTF_INPUT, &hw_ip->group[instance]->state))
		fimc_is_scaler_set_input_source(hw_ip->regs, hw_ip->id, 0);
	else
		fimc_is_scaler_set_input_source(hw_ip->regs, hw_ip->id, 1);

	ret = fimc_is_hw_mcsc_clear_interrupt(hw_ip);
	if (ret != 0) {
		mserr_hw("MCSC sw reset fail", instance, hw_ip);
		spin_unlock_irqrestore(&shared_output_slock, flag);
		return -ENODEV;
	}

	msdbg_hw(2, "enable: done, (0x%lx)\n", instance, hw_ip, hw_mcsc_out_configured);

#if defined(ENABLE_DNR_COMPRESSOR_IN_MCSC)
	fimc_is_hw_mcsc_tdnr_init(hw_ip, mcs_param, instance);
#endif
	set_bit(HW_RUN, &hw_ip->state);
	spin_unlock_irqrestore(&shared_output_slock, flag);

	return ret;
}

static int fimc_is_hw_mcsc_disable(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	int ret = 0;
	u32 input_id, output_id;
	long timetowait;
	bool config = true;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hardware *hardware;
	struct fimc_is_hw_ip *hw_ip0 = NULL, *hw_ip1 = NULL;

	BUG_ON(!hw_ip);
	BUG_ON(!cap);
	BUG_ON(!hw_ip->priv_info);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (atomic_read(&hw_ip->rsccount) > 1)
		return 0;

	msinfo_hw("mcsc_disable: Vvalid(%d)\n", instance, hw_ip,
		atomic_read(&hw_ip->status.Vvalid));

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (test_bit(HW_RUN, &hw_ip->state)) {
		timetowait = wait_event_timeout(hw_ip->status.wait_queue,
			!atomic_read(&hw_ip->status.Vvalid),
			FIMC_IS_HW_STOP_TIMEOUT);

		if (!timetowait)
			mserr_hw("wait FRAME_END timeout (%ld)", instance,
				hw_ip, timetowait);

		ret = fimc_is_hw_mcsc_clear_interrupt(hw_ip);
		if (ret != 0) {
			mserr_hw("MCSC sw clear_interrupt fail", instance, hw_ip);
			return -ENODEV;
		}

		clear_bit(HW_RUN, &hw_ip->state);
		clear_bit(HW_CONFIG, &hw_ip->state);
	} else {
		msdbg_hw(2, "already disabled\n", instance, hw_ip);
	}

	hw_ip->mframe = NULL;
	hardware = hw_ip->hardware;
	get_mcsc_hw_ip(hardware, &hw_ip0, &hw_ip1);

	if (hw_ip0 && test_bit(HW_RUN, &hw_ip0->state))
		return 0;

	if (hw_ip1 && test_bit(HW_RUN, &hw_ip1->state))
		return 0;


	if (hw_ip0)
		fimc_is_scaler_stop(hw_ip0->regs, hw_ip0->id);

	if (hw_ip1)
		fimc_is_scaler_stop(hw_ip1->regs, hw_ip1->id);

	/* disable MCSC */
	if (cap->in_dma == MCSC_CAP_SUPPORT)
		fimc_is_scaler_clear_rdma_addr(hw_ip->regs);

	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
		config = (input_id == hw_ip->id ? true: false);
		if (cap->enable_shared_output == false || !test_bit(output_id, &hw_mcsc_out_configured)) {
			msinfo_hw("[OUT:%d]hw_mcsc_disable: clear_wdma_addr\n", instance, hw_ip, output_id);
			fimc_is_scaler_clear_wdma_addr(hw_ip->regs, output_id);
		}
	}

	fimc_is_scaler_clear_shadow_ctrl(hw_ip->regs, hw_ip->id);

	/* disable TDNR */
	if (cap->tdnr == MCSC_CAP_SUPPORT) {
		fimc_is_scaler_set_tdnr_mode_select(hw_ip->regs, TDNR_MODE_BYPASS);

		fimc_is_scaler_clear_tdnr_rdma_addr(hw_ip->regs, TDNR_IMAGE);
		fimc_is_scaler_clear_tdnr_rdma_addr(hw_ip->regs, TDNR_WEIGHT);

		fimc_is_scaler_set_tdnr_wdma_enable(hw_ip->regs, TDNR_WEIGHT, false);
		fimc_is_scaler_clear_tdnr_wdma_addr(hw_ip->regs, TDNR_WEIGHT);

		hw_mcsc->cur_tdnr_mode = TDNR_MODE_BYPASS;
	}

	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++)
		set_bit(output_id, &hw_mcsc_out_configured);
	clear_bit(HW_MCSC_OUT_CLEARED_ALL, &hw_mcsc_out_configured);

	msinfo_hw("mcsc_disable: done, (0x%lx)\n", instance, hw_ip, hw_mcsc_out_configured);

	return ret;
}

static int fimc_is_hw_mcsc_rdma_cfg(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame)
{
	int ret = 0;
	int i;
	u32 rdma_addr[4] = {0};
	struct mcs_param *param;
	u32 plane;

	param = &hw_ip->region[frame->instance]->parameter.mcs;

	plane = param->input.plane;
	for (i = 0; i < plane; i++)
		rdma_addr[i] = frame->sourceAddress[plane * frame->cur_buf_index + i];

	/* DMA in */
	msdbg_hw(2, "[F:%d]rdma_cfg [addr: %x]\n",
		frame->instance, hw_ip, frame->fcount, rdma_addr[0]);

	if ((rdma_addr[0] == 0)
		&& (param->input.dma_cmd == DMA_INPUT_COMMAND_ENABLE)) {
		mserr_hw("Wrong rdma_addr(%x)\n", frame->instance, hw_ip, rdma_addr[0]);
		fimc_is_scaler_clear_rdma_addr(hw_ip->regs);
		ret = -EINVAL;
		return ret;
	}

	/* use only one buffer (per-frame) */
	fimc_is_scaler_set_rdma_frame_seq(hw_ip->regs, 0x1 << USE_DMA_BUFFER_INDEX);

	if (param->input.plane == DMA_INPUT_PLANE_4) {
		/* 8+2(10bit) format */
		rdma_addr[2] = rdma_addr[0] + ALIGN(param->input.dma_crop_width, 16);
		rdma_addr[3] = rdma_addr[1] + ALIGN(param->input.dma_crop_width, 16);
		fimc_is_scaler_set_rdma_addr(hw_ip->regs,
			rdma_addr[0], rdma_addr[1], 0, USE_DMA_BUFFER_INDEX);
		fimc_is_scaler_set_rdma_2bit_addr(hw_ip->regs,
			rdma_addr[2], rdma_addr[3], USE_DMA_BUFFER_INDEX);
	} else {
		fimc_is_scaler_set_rdma_addr(hw_ip->regs,
			rdma_addr[0], rdma_addr[1], rdma_addr[2],
			USE_DMA_BUFFER_INDEX);
	}

	return ret;
}

static void fimc_is_hw_mcsc_wdma_cfg(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame)
{
	int i;
	struct mcs_param *param;
	u32 wdma_addr[MCSC_OUTPUT_MAX][4] = {{0}};
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);
	struct fimc_is_hw_mcsc *hw_mcsc;
	u32 plane;
	ulong flag;

	BUG_ON(!cap);
	BUG_ON(!hw_ip->priv_info);

	param = &hw_ip->region[frame->instance]->parameter.mcs;
	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (frame->type == SHOT_TYPE_INTERNAL)
		goto skip_addr;

	plane = param->output[MCSC_OUTPUT0].plane;
	for (i = 0; i < plane; i++) {
		wdma_addr[MCSC_OUTPUT0][i] =
			frame->sc0TargetAddress[plane * frame->cur_buf_index + i];
		dbg_hw(2, "M0P(P:%d)(A:0x%X)\n", i, wdma_addr[MCSC_OUTPUT0][i]);
	}

	plane = param->output[MCSC_OUTPUT1].plane;
	for (i = 0; i < plane; i++) {
		wdma_addr[MCSC_OUTPUT1][i] =
			frame->sc1TargetAddress[plane * frame->cur_buf_index + i];
		dbg_hw(2, "M1P(P:%d)(A:0x%X)\n", i, wdma_addr[MCSC_OUTPUT1][i]);
	}

	plane = param->output[MCSC_OUTPUT2].plane;
	for (i = 0; i < plane; i++) {
		wdma_addr[MCSC_OUTPUT2][i] =
			frame->sc2TargetAddress[plane * frame->cur_buf_index + i];
		dbg_hw(2, "M2P(P:%d)(A:0x%X)\n", i, wdma_addr[MCSC_OUTPUT2][i]);
	}

	plane = param->output[MCSC_OUTPUT3].plane;
	for (i = 0; i < plane; i++) {
		wdma_addr[MCSC_OUTPUT3][i] =
			frame->sc3TargetAddress[plane * frame->cur_buf_index + i];
		dbg_hw(2, "M3P(P:%d)(A:0x%X)\n", i, wdma_addr[MCSC_OUTPUT3][i]);
	}

	plane = param->output[MCSC_OUTPUT4].plane;
	for (i = 0; i < plane; i++) {
		wdma_addr[MCSC_OUTPUT4][i] =
			frame->sc4TargetAddress[plane * frame->cur_buf_index + i];
		dbg_hw(2, "M4P(P:%d)(A:0x%X)\n", i, wdma_addr[MCSC_OUTPUT4][i]);
	}

	plane = param->output[MCSC_OUTPUT5].plane;
	for (i = 0; i < plane; i++) {
		wdma_addr[MCSC_OUTPUT5][i] =
			frame->sc5TargetAddress[plane * frame->cur_buf_index + i];
		dbg_hw(2, "M5P(P:%d)(A:0x%X)\n", i, wdma_addr[MCSC_OUTPUT5][i]);
	}
skip_addr:

	/* DMA out */
	for (i = MCSC_OUTPUT0; i < cap->max_output; i++) {
		if ((cap->out_dma[i] != MCSC_CAP_SUPPORT) || !test_bit(i, &hw_mcsc->out_en))
			continue;

		msdbg_hw(2, "[F:%d]wdma_cfg [T:%d][addr%d: %x]\n", frame->instance, hw_ip,
			frame->fcount, frame->type, i, wdma_addr[i][0]);

		if (param->output[i].dma_cmd != DMA_OUTPUT_COMMAND_DISABLE
			&& wdma_addr[i][0] != 0
			&& frame->type != SHOT_TYPE_INTERNAL) {

			spin_lock_irqsave(&shared_output_slock, flag);
			if (cap->enable_shared_output && test_bit(i, &hw_mcsc_out_configured)
				&& frame->type != SHOT_TYPE_MULTI) {
				mswarn_hw("[OUT:%d]DMA_OUTPUT in running state[F:%d]",
					frame->instance, hw_ip, i, frame->fcount);
				spin_unlock_irqrestore(&shared_output_slock, flag);
				return;
			}
			set_bit(i, &hw_mcsc_out_configured);
			spin_unlock_irqrestore(&shared_output_slock, flag);

			msdbg_hw(2, "[OUT:%d]dma_out enabled\n", frame->instance, hw_ip, i);
			fimc_is_scaler_set_dma_out_enable(hw_ip->regs, i, true);

			/* use only one buffer (per-frame) */
			fimc_is_scaler_set_wdma_frame_seq(hw_ip->regs, i,
				0x1 << USE_DMA_BUFFER_INDEX);
			fimc_is_scaler_set_wdma_addr(hw_ip->regs, i,
				wdma_addr[i][0], wdma_addr[i][1], wdma_addr[i][2],
				USE_DMA_BUFFER_INDEX);
			/* TODO : 10bit format */
/*
 *			wdma_addr[i][3] = 0;
 *			wdma_addr[i][4] = 0;
 *			fimc_is_scaler_set_wdma_2bit_addr(hw_ip->regs, i,
 *				wdma_addr[i][3], wdma_addr[i][4], USE_DMA_BUFFER_INDEX);
 */
		} else {
			u32 wdma_enable = 0;

			wdma_enable = fimc_is_scaler_get_dma_out_enable(hw_ip->regs, i);
			spin_lock_irqsave(&shared_output_slock, flag);
			if (wdma_enable && (cap->enable_shared_output == false || !test_bit(i, &hw_mcsc_out_configured))) {
				fimc_is_scaler_set_dma_out_enable(hw_ip->regs, i, false);
				fimc_is_scaler_clear_wdma_addr(hw_ip->regs, i);
				msdbg_hw(2, "[OUT:%d]shot: dma_out disabled\n",
						frame->instance, hw_ip, i);

				if (i == MCSC_OUTPUT_DS) {
					fimc_is_scaler_set_ds_enable(hw_ip->regs, false);
					msinfo_hw(" DS off\n", frame->instance, hw_ip);
				}
			}
			spin_unlock_irqrestore(&shared_output_slock, flag);
			msdbg_hw(2, "[OUT:%d]mcsc_wdma_cfg:wmda_enable(%d)[F:%d][T:%d][cmd:%d][addr:0x%x]\n",
				frame->instance, hw_ip, i, wdma_enable, frame->fcount, frame->type,
				param->output[i].dma_cmd, wdma_addr[i][0]);
		}
	}
}

static int fimc_is_hw_mcsc_shot(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame,
	ulong hw_map)
{
	int ret = 0;
	struct fimc_is_group *head;
	struct fimc_is_hardware *hardware;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct is_param_region *param;
	struct mcs_param *mcs_param;
	bool start_flag = true;
	u32 lindex, hindex, instance;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!frame);
	BUG_ON(!cap);

	hardware = hw_ip->hardware;
	instance = frame->instance;

	msdbgs_hw(2, "[F:%d]shot\n", instance, hw_ip, frame->fcount);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		msdbg_hw(2, "not initialized\n", instance, hw_ip);
		return -EINVAL;
	}

	head = GET_HEAD_GROUP_IN_DEVICE(FIMC_IS_DEVICE_ISCHAIN, hw_ip->group[frame->instance]);
	if (!test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state)) {
		ret = down_interruptible(&hw_ip->smp_resource);
		if (ret) {
			mserr_hw(" down fail(%d)", frame->instance, hw_ip, ret);
			return -EINVAL;
		}
	}

	if ((!test_bit(ENTRY_M0P, &frame->out_flag))
		&& (!test_bit(ENTRY_M1P, &frame->out_flag))
		&& (!test_bit(ENTRY_M2P, &frame->out_flag))
		&& (!test_bit(ENTRY_M3P, &frame->out_flag))
		&& (!test_bit(ENTRY_M4P, &frame->out_flag)))
		set_bit(hw_ip->id, &frame->core_flag);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	param = &hw_ip->region[instance]->parameter;
	mcs_param = &param->mcs;

	if (test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state)) {
		if (!test_bit(HW_CONFIG, &hw_ip->state)
			&& !atomic_read(&hardware->streaming[hardware->sensor_position[instance]]))
			start_flag = true;
		else
			start_flag = false;
	} else {
		start_flag = true;
	}

	if (frame->type == SHOT_TYPE_INTERNAL) {
		msdbg_hw(2, "request not exist\n", instance, hw_ip);
		goto config;
	}

	lindex = frame->shot->ctl.vendor_entry.lowIndexParam;
	hindex = frame->shot->ctl.vendor_entry.highIndexParam;

	fimc_is_hw_mcsc_update_param(hw_ip, mcs_param,
		lindex, hindex, instance);

	msdbg_hw(2, "[F:%d]shot [T:%d]\n", instance, hw_ip, frame->fcount, frame->type);

config:
	/* multi-buffer */
	hw_ip->cur_s_int = 0;
	hw_ip->cur_e_int = 0;
	if (frame->num_buffers)
		hw_ip->num_buffers = frame->num_buffers;
	if (frame->num_buffers > 1)
		hw_ip->mframe = frame;

	/* RDMA cfg */
	if (mcs_param->input.dma_cmd == DMA_INPUT_COMMAND_ENABLE
		&& cap->in_dma == MCSC_CAP_SUPPORT) {
		ret = fimc_is_hw_mcsc_rdma_cfg(hw_ip, frame);
		if (ret) {
			mserr_hw("[F:%d]mcsc rdma_cfg failed\n",
				instance, hw_ip, frame->fcount);
			return ret;
		}
	}

	/* WDMA cfg */
	fimc_is_hw_mcsc_wdma_cfg(hw_ip, frame);

	/* setting for DS */
	if (cap->ds_vra == MCSC_CAP_SUPPORT) {
		ret = fimc_is_hw_mcsc_update_dsvra_register(hw_ip, param, instance,
			frame->shot ? frame->shot->uctl.scalerUd.mcsc_sub_blk_port[INTERFACE_TYPE_DS] : MCSC_PORT_NONE);
		if (ret) {
			msdbg_hw(2, "dsvra cfg is failed\n", instance, hw_ip);
			fimc_is_scaler_set_ds_enable(hw_ip->regs, false);
		}
	}

	/* setting for TDNR */
	if (cap->tdnr == MCSC_CAP_SUPPORT)
		ret = fimc_is_hw_mcsc_update_tdnr_register(hw_ip, frame, param, start_flag);

	/* setting for YSUM */
	if (cap->ysum == MCSC_CAP_SUPPORT)
		ret = fimc_is_hw_mcsc_update_ysum_register(hw_ip, param,
			frame->shot ? frame->shot->uctl.scalerUd.mcsc_sub_blk_port[INTERFACE_TYPE_YSUM] : MCSC_PORT_NONE);

	/* for set shadow register write start
	 * only group input is OTF && not async shot
	 */
	if (test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state) && !start_flag)
		fimc_is_scaler_set_shadow_ctrl(hw_ip->regs, hw_ip->id, SHADOW_WRITE_FINISH);

	if (start_flag) {
		msdbg_hw(2, "[F:%d]start\n", instance, hw_ip, frame->fcount);
		fimc_is_scaler_start(hw_ip->regs, hw_ip->id);
		fimc_is_scaler_set_tdnr_rdma_start(hw_ip->regs, hw_mcsc->cur_tdnr_mode);
		if (mcs_param->input.dma_cmd == DMA_INPUT_COMMAND_ENABLE && cap->in_dma == MCSC_CAP_SUPPORT)
			fimc_is_scaler_rdma_start(hw_ip->regs, hw_ip->id);
	}

	msdbg_hw(2, "shot: hw_mcsc_out_configured[0x%lx]\n", instance, hw_ip,
		hw_mcsc_out_configured);

	hw_mcsc->instance = instance;
	clear_bit(HW_MCSC_OUT_CLEARED_ALL, &hw_mcsc_out_configured);
	set_bit(HW_CONFIG, &hw_ip->state);

	return ret;
}

static int fimc_is_hw_mcsc_set_param(struct fimc_is_hw_ip *hw_ip, struct is_region *region,
	u32 lindex, u32 hindex, u32 instance, ulong hw_map)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc;

	BUG_ON(!hw_ip);
	BUG_ON(!region);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -EINVAL;
	}

	hw_ip->region[instance] = region;
	hw_ip->lindex[instance] = lindex;
	hw_ip->hindex[instance] = hindex;

	/* To execute update_register function in hw_mcsc_shot(),
	 * the value of hw_mcsc->instance is set as default.
	 */
	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	hw_mcsc->instance = FIMC_IS_STREAM_COUNT;

	return ret;
}

void fimc_is_hw_mcsc_check_size(struct fimc_is_hw_ip *hw_ip, struct mcs_param *param,
	u32 instance, u32 output_id)
{
	struct param_mcs_input *input = &param->input;
	struct param_mcs_output *output = &param->output[output_id];

	minfo_hw("[OUT:%d]>>> hw_mcsc_check_size >>>\n", instance, output_id);
	info_hw("otf_input: format(%d),size(%dx%d)\n",
		input->otf_format, input->width, input->height);
	info_hw("dma_input: format(%d),crop_size(%dx%d)\n",
		input->dma_format, input->dma_crop_width, input->dma_crop_height);
	info_hw("output: otf_cmd(%d),dma_cmd(%d),format(%d),stride(y:%d,c:%d)\n",
		output->otf_cmd, output->dma_cmd, output->dma_format,
		output->dma_stride_y, output->dma_stride_c);
	info_hw("output: pos(%d,%d),crop%dx%d),size(%dx%d)\n",
		output->crop_offset_x, output->crop_offset_y,
		output->crop_width, output->crop_height,
		output->width, output->height);
	info_hw("[%d]<<< hw_mcsc_check_size <<<\n", instance);
}

int fimc_is_hw_mcsc_update_register(struct fimc_is_hw_ip *hw_ip,
	struct mcs_param *param, u32 output_id, u32 instance)
{
	int ret = 0;

	if (output_id == MCSC_OUTPUT_DS)
		return ret;

	hw_mcsc_check_size(hw_ip, param, instance, output_id);
	ret = fimc_is_hw_mcsc_poly_phase(hw_ip, &param->input,
			&param->output[output_id], output_id, instance);
	ret = fimc_is_hw_mcsc_post_chain(hw_ip, &param->input,
			&param->output[output_id], output_id, instance);
	ret = fimc_is_hw_mcsc_flip(hw_ip, &param->output[output_id], output_id, instance);
	ret = fimc_is_hw_mcsc_otf_output(hw_ip, &param->output[output_id], output_id, instance);
	ret = fimc_is_hw_mcsc_dma_output(hw_ip, &param->output[output_id], output_id, instance);
	ret = fimc_is_hw_mcsc_output_yuvrange(hw_ip, &param->output[output_id], output_id, instance);
	ret = fimc_is_hw_mcsc_hwfc_output(hw_ip, &param->output[output_id], output_id, instance);

	return ret;
}

int fimc_is_hw_mcsc_update_param(struct fimc_is_hw_ip *hw_ip,
	struct mcs_param *param, u32 lindex, u32 hindex, u32 instance)
{
	int i = 0;
	int ret = 0;
	bool control_cmd = false;
	struct fimc_is_hw_mcsc *hw_mcsc;
	u32 hwfc_output_ids = 0;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!param);
	BUG_ON(!cap);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (hw_mcsc->instance != instance) {
		control_cmd = true;
		msdbg_hw(2, "update_param: hw_ip->instance(%d), control_cmd(%d)\n",
			instance, hw_ip, hw_mcsc->instance, control_cmd);
	}

	if (control_cmd || (lindex & LOWBIT_OF(PARAM_MCS_INPUT))
		|| (hindex & HIGHBIT_OF(PARAM_MCS_INPUT))
		|| (test_bit(HW_MCSC_OUT_CLEARED_ALL, &hw_mcsc_out_configured))) {
		ret = fimc_is_hw_mcsc_otf_input(hw_ip, &param->input, instance);
		ret = fimc_is_hw_mcsc_dma_input(hw_ip, &param->input, instance);
	}

	if (cap->djag == MCSC_CAP_SUPPORT)
		fimc_is_hw_mcsc_update_djag_register(hw_ip, param, instance);	/* for DZoom */

	for (i = MCSC_OUTPUT0; i < cap->max_output; i++) {
		if (control_cmd || (lindex & LOWBIT_OF((i + PARAM_MCS_OUTPUT0)))
				|| (hindex & HIGHBIT_OF((i + PARAM_MCS_OUTPUT0)))
				|| (test_bit(HW_MCSC_OUT_CLEARED_ALL, &hw_mcsc_out_configured))) {
			ret = fimc_is_hw_mcsc_update_register(hw_ip, param, i, instance);
			fimc_is_scaler_set_wdma_pri(hw_ip->regs, i, param->output[i].plane);	/* FIXME: */

		}
			/* check the hwfc enable in all output */
			if (param->output[i].hwfc)
				hwfc_output_ids |= (1 << i);
		}

	/* setting for hwfc */
	ret = fimc_is_hw_mcsc_hwfc_mode(hw_ip, &param->input, hwfc_output_ids, instance);

	if (ret)
		fimc_is_hw_mcsc_size_dump(hw_ip);

	return ret;
}

int fimc_is_hw_mcsc_reset(struct fimc_is_hw_ip *hw_ip)
{
	int ret = 0;
	u32 output_id;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);
	struct fimc_is_hardware *hardware;
	struct fimc_is_hw_ip *hw_ip0 = NULL, *hw_ip1 = NULL;

	BUG_ON(!hw_ip);
	BUG_ON(!cap);

	hardware = hw_ip->hardware;
	get_mcsc_hw_ip(hardware, &hw_ip0, &hw_ip1);

	if (cap->enable_shared_output) {
		if (hw_ip0 && test_bit(HW_RUN, &hw_ip0->state))
			return 0;

		if (hw_ip1 && test_bit(HW_RUN, &hw_ip1->state))
			return 0;
	}

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (hw_ip0) {
		sinfo_hw("hw_mcsc_reset: out_en[0x%lx]\n", hw_ip0, hw_mcsc->out_en);
		ret = fimc_is_scaler_sw_reset(hw_ip0->regs, hw_ip0->id, 0, 0);
		if (ret != 0) {
			serr_hw("sw reset fail", hw_ip0);
			return -ENODEV;
		}

		/* shadow ctrl register clear */
		fimc_is_scaler_clear_shadow_ctrl(hw_ip0->regs, hw_ip0->id);
	}

	if (hw_ip1) {
		sinfo_hw("hw_mcsc_reset: out_en[0x%lx]\n", hw_ip1, hw_mcsc->out_en);
		ret = fimc_is_scaler_sw_reset(hw_ip1->regs, hw_ip1->id, 0, 0);
		if (ret != 0) {
			serr_hw("sw reset fail", hw_ip1);
			return -ENODEV;
		}

		/* shadow ctrl register clear */
		fimc_is_scaler_clear_shadow_ctrl(hw_ip1->regs, hw_ip1->id);
	}

	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		if (cap->enable_shared_output == false || test_bit(output_id, &hw_mcsc_out_configured)) {
			sinfo_hw("[OUT:%d]set output clear\n", hw_ip, output_id);
			fimc_is_scaler_set_poly_scaler_enable(hw_ip->regs, hw_ip->id, output_id, 0);
			fimc_is_scaler_set_post_scaler_enable(hw_ip->regs, output_id, 0);
			fimc_is_scaler_set_otf_out_enable(hw_ip->regs, output_id, false);
			fimc_is_scaler_set_dma_out_enable(hw_ip->regs, output_id, false);

			fimc_is_scaler_set_wdma_pri(hw_ip->regs, output_id, 0);	/* FIXME: */
			fimc_is_scaler_set_wdma_axi_pri(hw_ip->regs);		/* FIXME: */
			fimc_is_scaler_set_wdma_sram_base(hw_ip->regs, output_id);
			clear_bit(output_id, &hw_mcsc_out_configured);
		}
	}
	fimc_is_scaler_set_wdma_sram_base(hw_ip->regs, MCSC_OUTPUT_SSB);	/* FIXME: */

	/* for tdnr */
	if (cap->tdnr == MCSC_CAP_SUPPORT)
		fimc_is_scaler_set_tdnr_wdma_sram_base(hw_ip->regs, TDNR_WEIGHT);

	set_bit(HW_MCSC_OUT_CLEARED_ALL, &hw_mcsc_out_configured);
	clear_bit(ALL_BLOCK_SET_DONE, &hw_mcsc->blk_set_ctrl);

	if (cap->in_otf == MCSC_CAP_SUPPORT) {
		if (hw_ip0)
			fimc_is_scaler_set_stop_req_post_en_ctrl(hw_ip0->regs, hw_ip0->id, 0);

		if (hw_ip1)
			fimc_is_scaler_set_stop_req_post_en_ctrl(hw_ip1->regs, hw_ip1->id, 0);
	}

	return ret;
}

int fimc_is_hw_mcsc_clear_interrupt(struct fimc_is_hw_ip *hw_ip)
{
	int ret = 0;

	BUG_ON(!hw_ip);

	sinfo_hw("hw_mcsc_clear_interrupt\n", hw_ip);

	fimc_is_scaler_clear_intr_all(hw_ip->regs, hw_ip->id);
	fimc_is_scaler_disable_intr(hw_ip->regs, hw_ip->id);
	fimc_is_scaler_mask_intr(hw_ip->regs, hw_ip->id, MCSC_INTR_MASK);

	return ret;
}

static int fimc_is_hw_mcsc_load_setfile(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	int ret = 0;
	struct fimc_is_hw_ip_setfile *setfile;
	struct hw_api_scaler_setfile *setfile_addr;
	enum exynos_sensor_position sensor_position;
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
	int setfile_index = 0;

	BUG_ON(!hw_ip);

	if (!test_bit_variables(hw_ip->id, &hw_map)) {
		msdbg_hw(2, "%s: hw_map(0x%lx)\n", instance, hw_ip, __func__, hw_map);
		return 0;
	}

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -ESRCH;
	}

	if (!unlikely(hw_ip->priv_info)) {
		mserr_hw("priv_info is NULL", instance, hw_ip);
		return -EINVAL;
	}
	sensor_position = hw_ip->hardware->sensor_position[instance];
	setfile = &hw_ip->setfile[sensor_position];

	switch (setfile->version) {
	case SETFILE_V2:
		break;
	case SETFILE_V3:
		break;
	default:
		mserr_hw("invalid version (%d)", instance, hw_ip,
			setfile->version);
		return -EINVAL;
	}

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (setfile->table[0].size != sizeof(struct hw_api_scaler_setfile))
		mswarn_hw("tuneset size(%x) is not matched to setfile structure size(%lx)",
			instance, hw_ip, setfile->table[0].size,
			sizeof(struct hw_api_scaler_setfile));

	/* copy MCSC setfile set */
	setfile_addr = (struct hw_api_scaler_setfile *)setfile->table[0].addr;
	memcpy(hw_mcsc->setfile[sensor_position], setfile_addr,
			sizeof(struct hw_api_scaler_setfile) * setfile->using_count);

	/* check each setfile Magic numbers */
	for (setfile_index = 0; setfile_index < setfile->using_count; setfile_index++) {
		setfile_addr = &hw_mcsc->setfile[sensor_position][setfile_index];

		if (setfile_addr->setfile_version != MCSC_SETFILE_VERSION) {
			mserr_hw("setfile[%d] version(0x%x) is incorrect",
				instance, hw_ip, setfile_index,
				setfile_addr->setfile_version);
			return -EINVAL;
		}
	}

	set_bit(HW_TUNESET, &hw_ip->state);

	return ret;
}

static int fimc_is_hw_mcsc_apply_setfile(struct fimc_is_hw_ip *hw_ip, u32 scenario,
	u32 instance, ulong hw_map)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
	struct fimc_is_hw_ip_setfile *setfile;
	enum exynos_sensor_position sensor_position;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);
	u32 setfile_index = 0;

	BUG_ON(!hw_ip);
	BUG_ON(!cap);

	if (!test_bit_variables(hw_ip->id, &hw_map)) {
		msdbg_hw(2, "%s: hw_map(0x%lx)\n", instance, hw_ip, __func__, hw_map);
		return 0;
	}

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -ESRCH;
	}

	if (!unlikely(hw_ip->priv_info)) {
		mserr_hw("priv info is NULL", instance, hw_ip);
		return -EINVAL;
	}

	sensor_position = hw_ip->hardware->sensor_position[instance];
	setfile = &hw_ip->setfile[sensor_position];

	setfile_index = setfile->index[scenario];
	if (setfile_index >= setfile->using_count) {
		mserr_hw("setfile index is out-of-range, [%d:%d]",
				instance, hw_ip, scenario, setfile_index);
		return -EINVAL;
	}

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!hw_mcsc->setfile[sensor_position]) {
		mserr_hw("hw_mcsc->setfile(null)", instance, hw_ip);
		return -ENOMEM;
	}

	hw_mcsc->applied_setfile[sensor_position] =
		&hw_mcsc->setfile[sensor_position][setfile_index];

	if (cap->djag) {
		if ((setfile_index == FIMC_IS_SN_FRONT_CAMCORDING)
			|| (setfile_index == FIMC_IS_SN_FRONT_CAMCORDING_WHD)
			|| (setfile_index == FIMC_IS_SN_FRONT_CAMCORDING_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_FRONT_CAMCORDING_WHD_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_FRONT2_CAMCORDING)
			|| (setfile_index == FIMC_IS_SN_FRONT2_CAMCORDING_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_REAR2_CAMCORDING_FHD)
			|| (setfile_index == FIMC_IS_SN_REAR2_CAMCORDING_FHD_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_FHD)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_WHD)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_UHD)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_UHD_60FPS)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_FHD_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_WHD_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_REAR_CAMCORDING_UHD_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_DUAL_CAMCORDING)
			|| (setfile_index == FIMC_IS_SN_DUAL_CAMCORDING_CAPTURE)
			|| (setfile_index == FIMC_IS_SN_PIP_CAMCORDING)
			|| (setfile_index == FIMC_IS_SN_PIP_CAMCORDING_CAPTURE))
			hw_mcsc->djag_input_source = 0;
		else
			hw_mcsc->djag_input_source = 1;
	}

	msinfo_hw("setfile (%d) scenario (%d)\n", instance, hw_ip,
		setfile_index, scenario);

	return ret;
}

static int fimc_is_hw_mcsc_delete_setfile(struct fimc_is_hw_ip *hw_ip, u32 instance,
	ulong hw_map)
{
	BUG_ON(!hw_ip);

	if (!test_bit_variables(hw_ip->id, &hw_map)) {
		msdbg_hw(2, "%s: hw_map(0x%lx)\n", instance, hw_ip, __func__, hw_map);
		return 0;
	}

	if (!test_bit(HW_TUNESET, &hw_ip->state)) {
		msdbg_hw(2, "setfile already deleted", instance, hw_ip);
		return 0;
	}

	clear_bit(HW_TUNESET, &hw_ip->state);

	return 0;
}

void fimc_is_hw_mcsc_frame_done(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame,
	int done_type)
{
	int ret = 0;
	bool fdone_flag = false;
	struct fimc_is_framemgr *framemgr;
	u32 index;
	int instance = atomic_read(&hw_ip->instance);
	ulong flags = 0;
	bool flag_get_meta = true;

	switch (done_type) {
	case IS_SHOT_SUCCESS:
		framemgr = hw_ip->framemgr;
		framemgr_e_barrier_common(framemgr, 0, flags);
		frame = peek_frame(framemgr, FS_HW_WAIT_DONE);
		framemgr_x_barrier_common(framemgr, 0, flags);
		if (frame == NULL) {
			mserr_hw("[F:%d] frame(null) @FS_HW_WAIT_DONE!!", instance,
				hw_ip, atomic_read(&hw_ip->fcount));
			framemgr_e_barrier_common(framemgr, 0, flags);
			print_all_hw_frame_count(hw_ip->hardware);
			framemgr_x_barrier_common(framemgr, 0, flags);
			return;
		}
		break;
	case IS_SHOT_UNPROCESSED:
	case IS_SHOT_LATE_FRAME:
	case IS_SHOT_OVERFLOW:
	case IS_SHOT_INVALID_FRAMENUMBER:
	case IS_SHOT_TIMEOUT:
		if (frame == NULL) {
			mserr_hw("[F:%d] frame(null) type(%d)", instance,
				hw_ip, atomic_read(&hw_ip->fcount), done_type);
			return;
		}
		break;
	default:
		mserr_hw("[F:%d] invalid done type(%d)\n", instance, hw_ip,
			atomic_read(&hw_ip->fcount), done_type);
		return;
	}

	msdbgs_hw(2, "frame done[F:%d][O:0x%lx]\n", instance, hw_ip,
		frame->fcount, frame->out_flag);

	if (test_bit(ENTRY_M0P, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame,
			WORK_M0P_FDONE, ENTRY_M0P, done_type, flag_get_meta);
		fdone_flag = true;
		flag_get_meta = false;
		clear_bit(MCSC_OUTPUT0, &hw_mcsc_out_configured);
	}

	if (test_bit(ENTRY_M1P, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame,
			WORK_M1P_FDONE, ENTRY_M1P, done_type, flag_get_meta);
		fdone_flag = true;
		flag_get_meta = false;
		clear_bit(MCSC_OUTPUT1, &hw_mcsc_out_configured);
	}

	if (test_bit(ENTRY_M2P, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame,
			WORK_M2P_FDONE, ENTRY_M2P, done_type, flag_get_meta);
		fdone_flag = true;
		flag_get_meta = false;
		clear_bit(MCSC_OUTPUT2, &hw_mcsc_out_configured);
	}

	if (test_bit(ENTRY_M3P, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame,
			WORK_M3P_FDONE, ENTRY_M3P, done_type, flag_get_meta);
		fdone_flag = true;
		flag_get_meta = false;
		clear_bit(MCSC_OUTPUT3, &hw_mcsc_out_configured);
	}

	if (test_bit(ENTRY_M4P, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame,
			WORK_M4P_FDONE, ENTRY_M4P, done_type, flag_get_meta);
		fdone_flag = true;
		flag_get_meta = false;
		clear_bit(MCSC_OUTPUT4, &hw_mcsc_out_configured);
	}

	if (test_bit(ENTRY_M5P, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame,
			WORK_M5P_FDONE, ENTRY_M5P, done_type, flag_get_meta);
		fdone_flag = true;
		flag_get_meta = false;
		clear_bit(MCSC_OUTPUT5, &hw_mcsc_out_configured);
	}

	if (fdone_flag) {
		index = hw_ip->debug_index[1];
		hw_ip->debug_info[index].cpuid[DEBUG_POINT_FRAME_DMA_END] = raw_smp_processor_id();
		hw_ip->debug_info[index].time[DEBUG_POINT_FRAME_DMA_END] = cpu_clock(raw_smp_processor_id());
		atomic_inc(&hw_ip->count.dma);
	} else {
		index = hw_ip->debug_index[1];
		hw_ip->debug_info[index].cpuid[DEBUG_POINT_FRAME_END] = raw_smp_processor_id();
		hw_ip->debug_info[index].time[DEBUG_POINT_FRAME_END] = cpu_clock(raw_smp_processor_id());

		fimc_is_hardware_frame_done(hw_ip, NULL, -1, FIMC_IS_HW_CORE_END,
				IS_SHOT_SUCCESS, flag_get_meta);
	}

	return;
}

static int fimc_is_hw_mcsc_frame_ndone(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame,
	u32 instance, enum ShotErrorType done_type)
{
	int ret = 0;
	struct fimc_is_group *head;

	fimc_is_hw_mcsc_frame_done(hw_ip, frame, done_type);

	if (test_bit_variables(hw_ip->id, &frame->core_flag))
		ret = fimc_is_hardware_frame_done(hw_ip, frame, -1, FIMC_IS_HW_CORE_END,
				done_type, true);

	head = GET_HEAD_GROUP_IN_DEVICE(FIMC_IS_DEVICE_ISCHAIN, hw_ip->group[instance]);
	if (!test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state))
		up(&hw_ip->smp_resource);

	return ret;
}

int fimc_is_hw_mcsc_otf_input(struct fimc_is_hw_ip *hw_ip, struct param_mcs_input *input,
	u32 instance)
{
	int ret = 0;
	u32 width, height;
	u32 format, bit_width;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!input);
	BUG_ON(!cap);

	/* can't support this function */
	if (cap->in_otf != MCSC_CAP_SUPPORT)
		return ret;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	msdbg_hw(2, "otf_input_setting cmd(%d)\n", instance, hw_ip, input->otf_cmd);
	width  = input->width;
	height = input->height;
	format = input->otf_format;
	bit_width = input->otf_bitwidth;

	if (input->otf_cmd == OTF_INPUT_COMMAND_DISABLE)
		return ret;

	ret = fimc_is_hw_mcsc_check_format(HW_MCSC_OTF_INPUT,
		format, bit_width, width, height);
	if (ret) {
		mserr_hw("Invalid OTF Input format: fmt(%d),bit(%d),size(%dx%d)",
			instance, hw_ip, format, bit_width, width, height);
		return ret;
	}

	fimc_is_scaler_set_input_img_size(hw_ip->regs, hw_ip->id, width, height);
	fimc_is_scaler_set_dither(hw_ip->regs, hw_ip->id, 0);

	return ret;
}

int fimc_is_hw_mcsc_dma_input(struct fimc_is_hw_ip *hw_ip, struct param_mcs_input *input,
	u32 instance)
{
	int ret = 0;
	u32 width, height;
	u32 format, bit_width, plane, order;
	u32 y_stride, uv_stride;
	u32 y_2bit_stride, uv_2bit_stride;
	u32 img_format;
	u32 img_10bit_type;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!input);
	BUG_ON(!cap);

	/* can't support this function */
	if (cap->in_dma != MCSC_CAP_SUPPORT)
		return ret;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	msdbg_hw(2, "dma_input_setting cmd(%d)\n", instance, hw_ip, input->dma_cmd);
	width  = input->dma_crop_width;
	height = input->dma_crop_height;
	format = input->dma_format;
	bit_width = input->dma_bitwidth;
	plane = input->plane;
	order = input->dma_order;
	y_stride = input->dma_stride_y;
	uv_stride = input->dma_stride_c;
	y_2bit_stride = 0;
	uv_2bit_stride = 0;
	img_10bit_type = 0;

	if (input->dma_cmd == DMA_INPUT_COMMAND_DISABLE)
		return ret;

	ret = fimc_is_hw_mcsc_check_format(HW_MCSC_DMA_INPUT,
		format, bit_width, width, height);
	if (ret) {
		mserr_hw("Invalid DMA Input format: fmt(%d),bit(%d),size(%dx%d)",
			instance, hw_ip, format, bit_width, width, height);
		return ret;
	}

	fimc_is_scaler_set_input_img_size(hw_ip->regs, hw_ip->id, width, height);
	fimc_is_scaler_set_dither(hw_ip->regs, hw_ip->id, 0);

	fimc_is_scaler_set_rdma_stride(hw_ip->regs, y_stride, uv_stride);
	fimc_is_scaler_set_rdma_2bit_stride(hw_ip->regs, y_2bit_stride, uv_2bit_stride);

	ret = fimc_is_hw_mcsc_adjust_input_img_fmt(format, plane, order, &img_format);
	if (ret < 0) {
		mswarn_hw("Invalid rdma image format", instance, hw_ip);
		img_format = hw_mcsc->in_img_format;
	} else {
		hw_mcsc->in_img_format = img_format;
	}

	fimc_is_scaler_set_rdma_size(hw_ip->regs, width, height);
	fimc_is_scaler_set_rdma_format(hw_ip->regs, img_format);
	fimc_is_scaler_set_rdma_10bit_type(hw_ip->regs, img_10bit_type);

	return ret;
}


int fimc_is_hw_mcsc_poly_phase(struct fimc_is_hw_ip *hw_ip, struct param_mcs_input *input,
	struct param_mcs_output *output, u32 output_id, u32 instance)
{
	int ret = 0;
	u32 src_pos_x, src_pos_y, src_width, src_height;
	u32 poly_dst_width, poly_dst_height;
	u32 out_width, out_height;
	ulong temp_width, temp_height;
	u32 hratio, vratio;
	u32 input_id = 0;
	bool config = true;
	bool post_en = false;
	bool round_mode_en = true;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!input);
	BUG_ON(!output);
	BUG_ON(!hw_ip->priv_info);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	msdbg_hw(2, "[OUT:%d]poly_phase_setting cmd(O:%d,D:%d)\n",
		instance, hw_ip, output_id, output->otf_cmd, output->dma_cmd);

	input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
	config = (input_id == hw_ip->id ? true: false);

	if (output->otf_cmd == OTF_OUTPUT_COMMAND_DISABLE
		&& output->dma_cmd == DMA_OUTPUT_COMMAND_DISABLE) {
		if (cap->enable_shared_output == false
			|| (config && !test_bit(output_id, &hw_mcsc_out_configured)))
			fimc_is_scaler_set_poly_scaler_enable(hw_ip->regs, hw_ip->id, output_id, 0);
		return ret;
	}

	fimc_is_scaler_set_poly_scaler_enable(hw_ip->regs, hw_ip->id, output_id, 1);

	src_pos_x = output->crop_offset_x;
	src_pos_y = output->crop_offset_y;
	src_width = output->crop_width;
	src_height = output->crop_height;

	out_width = output->width;
	out_height = output->height;

	fimc_is_scaler_set_poly_src_size(hw_ip->regs, output_id, src_pos_x, src_pos_y,
		src_width, src_height);

	if ((src_width <= (out_width * MCSC_POLY_RATIO_DOWN))
		&& (out_width <= (src_width * MCSC_POLY_RATIO_UP))) {
		poly_dst_width = out_width;
		post_en = false;
	} else if ((src_width <= (out_width * MCSC_POLY_RATIO_DOWN * MCSC_POST_RATIO_DOWN))
		&& ((out_width * MCSC_POLY_RATIO_DOWN) < src_width)) {
		poly_dst_width = MCSC_ROUND_UP(src_width / MCSC_POLY_RATIO_DOWN, 2);
		post_en = true;
	} else {
		mserr_hw("hw_mcsc_poly_phase: Unsupported H ratio, (%dx%d)->(%dx%d)\n",
			instance, hw_ip, src_width, src_height, out_width, out_height);
		poly_dst_width = MCSC_ROUND_UP(src_width / MCSC_POLY_RATIO_DOWN, 2);
		post_en = true;
	}

	if ((src_height <= (out_height * MCSC_POLY_RATIO_DOWN))
		&& (out_height <= (src_height * MCSC_POLY_RATIO_UP))) {
		poly_dst_height = out_height;
		post_en = false;
	} else if ((src_height <= (out_height * MCSC_POLY_RATIO_DOWN * MCSC_POST_RATIO_DOWN))
		&& ((out_height * MCSC_POLY_RATIO_DOWN) < src_height)) {
		poly_dst_height = (src_height / MCSC_POLY_RATIO_DOWN);
		post_en = true;
	} else {
		mserr_hw("hw_mcsc_poly_phase: Unsupported H ratio, (%dx%d)->(%dx%d)\n",
			instance, hw_ip, src_width, src_height, out_width, out_height);
		poly_dst_height = (src_height / MCSC_POLY_RATIO_DOWN);
		post_en = true;
	}
#if defined(MCSC_POST_WA)
	/* The post scaler guarantee the quality of image          */
	/*  in case the scaling ratio equals to multiple of x1/256 */
	if ((post_en && ((poly_dst_width << MCSC_POST_WA_SHIFT) % out_width))
		|| (post_en && ((poly_dst_height << MCSC_POST_WA_SHIFT) % out_height))) {
		u32 multiple_hratio = 1;
		u32 multiple_vratio = 1;
		u32 shift = 0;
		for (shift = 0; shift <= MCSC_POST_WA_SHIFT; shift++) {
			if (((out_width % (1 << (MCSC_POST_WA_SHIFT-shift))) == 0)
				&& (out_height % (1 << (MCSC_POST_WA_SHIFT-shift)) == 0)) {
				multiple_hratio = out_width  / (1 << (MCSC_POST_WA_SHIFT-shift));
				multiple_vratio = out_height / (1 << (MCSC_POST_WA_SHIFT-shift));
				break;
			}
		}
		msdbg_hw(2, "[OUT:%d]poly_phase: shift(%d), ratio(%d,%d), "
			"size(%dx%d) before calculation\n",
			instance, hw_ip, output_id, shift, multiple_hratio, multiple_hratio,
			poly_dst_width, poly_dst_height);
		poly_dst_width  = MCSC_ROUND_UP(poly_dst_width, multiple_hratio);
		poly_dst_height = MCSC_ROUND_UP(poly_dst_height, multiple_vratio);
		if (poly_dst_width % 2) {
			poly_dst_width  = poly_dst_width  + multiple_hratio;
			poly_dst_height = poly_dst_height + multiple_vratio;
		}
		msdbg_hw(2, "[OUT:%d]poly_phase: size(%dx%d) after  calculation\n",
			instance, hw_ip, output_id, poly_dst_width, poly_dst_height);
	}
#endif

	fimc_is_scaler_set_poly_dst_size(hw_ip->regs, output_id,
		poly_dst_width, poly_dst_height);

	temp_width  = (ulong)src_width;
	temp_height = (ulong)src_height;
	hratio = (u32)((temp_width  << MCSC_PRECISION) / poly_dst_width);
	vratio = (u32)((temp_height << MCSC_PRECISION) / poly_dst_height);

	fimc_is_scaler_set_poly_scaling_ratio(hw_ip->regs, output_id, hratio, vratio);
	fimc_is_scaler_set_poly_scaler_coef(hw_ip->regs, output_id, hratio, vratio);
	fimc_is_scaler_set_poly_round_mode(hw_ip->regs, output_id, round_mode_en);

	return ret;
}

int fimc_is_hw_mcsc_post_chain(struct fimc_is_hw_ip *hw_ip, struct param_mcs_input *input,
	struct param_mcs_output *output, u32 output_id, u32 instance)
{
	int ret = 0;
	u32 img_width, img_height;
	u32 dst_width, dst_height;
	ulong temp_width, temp_height;
	u32 hratio, vratio;
	u32 input_id = 0;
	bool config = true;
	bool round_mode_en = true;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!input);
	BUG_ON(!output);
	BUG_ON(!hw_ip->priv_info);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	msdbg_hw(2, "[OUT:%d]post_chain_setting cmd(O:%d,D:%d)\n",
		instance, hw_ip, output_id, output->otf_cmd, output->dma_cmd);

	input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
	config = (input_id == hw_ip->id ? true: false);

	if (output->otf_cmd == OTF_OUTPUT_COMMAND_DISABLE
		&& output->dma_cmd == DMA_OUTPUT_COMMAND_DISABLE) {
		if (cap->enable_shared_output == false
			|| (config && !test_bit(output_id, &hw_mcsc_out_configured)))
			fimc_is_scaler_set_post_scaler_enable(hw_ip->regs, output_id, 0);
		return ret;
	}

	fimc_is_scaler_get_poly_dst_size(hw_ip->regs, output_id, &img_width, &img_height);

	dst_width = output->width;
	dst_height = output->height;

	/* if x1 ~ x1/4 scaling, post scaler bypassed */
	if ((img_width == dst_width) && (img_height == dst_height)) {
		fimc_is_scaler_set_post_scaler_enable(hw_ip->regs, output_id, 0);
	} else {
		fimc_is_scaler_set_post_scaler_enable(hw_ip->regs, output_id, 1);
	}

	fimc_is_scaler_set_post_img_size(hw_ip->regs, output_id, img_width, img_height);
	fimc_is_scaler_set_post_dst_size(hw_ip->regs, output_id, dst_width, dst_height);

	temp_width  = (ulong)img_width;
	temp_height = (ulong)img_height;
	hratio = (u32)((temp_width  << MCSC_PRECISION) / dst_width);
	vratio = (u32)((temp_height << MCSC_PRECISION) / dst_height);

	fimc_is_scaler_set_post_scaling_ratio(hw_ip->regs, output_id, hratio, vratio);
	fimc_is_scaler_set_post_scaler_coef(hw_ip->regs, output_id, hratio, vratio);
	fimc_is_scaler_set_post_round_mode(hw_ip->regs, output_id, round_mode_en);

	return ret;
}

int fimc_is_hw_mcsc_flip(struct fimc_is_hw_ip *hw_ip, struct param_mcs_output *output,
	u32 output_id, u32 instance)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc;

	BUG_ON(!hw_ip);
	BUG_ON(!output);
	BUG_ON(!hw_ip->priv_info);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	msdbg_hw(2, "[OUT:%d]flip_setting flip(%d),cmd(O:%d,D:%d)\n",
		instance, hw_ip, output_id, output->flip, output->otf_cmd, output->dma_cmd);

	if (output->dma_cmd == DMA_OUTPUT_COMMAND_DISABLE)
		return ret;

	fimc_is_scaler_set_flip_mode(hw_ip->regs, output_id, output->flip);

	return ret;
}

int fimc_is_hw_mcsc_otf_output(struct fimc_is_hw_ip *hw_ip, struct param_mcs_output *output,
	u32 output_id, u32 instance)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc;
	u32 out_width, out_height;
	u32 format, bitwidth;
	u32 input_id = 0;
	bool config = true;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!output);
	BUG_ON(!cap);
	BUG_ON(!hw_ip->priv_info);

	/* can't support this function */
	if (cap->out_otf[output_id] != MCSC_CAP_SUPPORT)
		return ret;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	msdbg_hw(2, "[OUT:%d]otf_output_setting cmd(O:%d,D:%d)\n",
		instance, hw_ip, output_id, output->otf_cmd, output->dma_cmd);

	input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
	config = (input_id == hw_ip->id ? true: false);

	if (output->otf_cmd == OTF_OUTPUT_COMMAND_DISABLE) {
		if (cap->enable_shared_output == false
			|| (config && !test_bit(output_id, &hw_mcsc_out_configured)))
			fimc_is_scaler_set_otf_out_enable(hw_ip->regs, output_id, false);
		return ret;
	}

	out_width  = output->width;
	out_height = output->height;
	format     = output->otf_format;
	bitwidth  = output->otf_bitwidth;

	ret = fimc_is_hw_mcsc_check_format(HW_MCSC_OTF_OUTPUT,
		format, bitwidth, out_width, out_height);
	if (ret) {
		mserr_hw("[OUT:%d]Invalid MCSC OTF Output format: fmt(%d),bit(%d),size(%dx%d)",
			instance, hw_ip, output_id, format, bitwidth, out_width, out_height);
		return ret;
	}

	fimc_is_scaler_set_otf_out_enable(hw_ip->regs, output_id, true);
	fimc_is_scaler_set_otf_out_path(hw_ip->regs, output_id);

	return ret;
}

int fimc_is_hw_mcsc_dma_output(struct fimc_is_hw_ip *hw_ip, struct param_mcs_output *output,
	u32 output_id, u32 instance)
{
	int ret = 0;
	u32 out_width, out_height, scaled_width = 0, scaled_height = 0;
	u32 format, plane, order,bitwidth;
	u32 y_stride, uv_stride;
	u32 y_2bit_stride, uv_2bit_stride;
	u32 img_format;
	u32 img_10bit_type = 0;
	u32 input_id = 0;
	bool config = true;
	bool conv420_en = false;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!output);
	BUG_ON(!cap);
	BUG_ON(!hw_ip->priv_info);

	/* can't support this function */
	if (cap->out_dma[output_id] != MCSC_CAP_SUPPORT)
		return ret;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	msdbg_hw(2, "[OUT:%d]dma_output_setting cmd(O:%d,D:%d)\n",
		instance, hw_ip, output_id, output->otf_cmd, output->dma_cmd);

	input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
	config = (input_id == hw_ip->id ? true: false);

	if (output->dma_cmd == DMA_OUTPUT_COMMAND_DISABLE) {
		if (cap->enable_shared_output == false
			|| (config && !test_bit(output_id, &hw_mcsc_out_configured)))
			fimc_is_scaler_set_dma_out_enable(hw_ip->regs, output_id, false);
		return ret;
	}

	out_width  = output->width;
	out_height = output->height;
	format     = output->dma_format;
	plane      = output->plane;
	order      = output->dma_order;
	bitwidth   = output->dma_bitwidth;
	y_stride   = output->dma_stride_y;
	uv_stride  = output->dma_stride_c;
	y_2bit_stride = 0;
	uv_2bit_stride = 0;

	ret = fimc_is_hw_mcsc_check_format(HW_MCSC_DMA_OUTPUT,
		format, bitwidth, out_width, out_height);
	if (ret) {
		mserr_hw("[OUT:%d]Invalid MCSC DMA Output format: fmt(%d),bit(%d),size(%dx%d)",
			instance, hw_ip, output_id, format, bitwidth, out_width, out_height);
		return ret;
	}

	ret = fimc_is_hw_mcsc_adjust_output_img_fmt(format, plane, order,
			&img_format, &conv420_en);
	if (ret < 0) {
		mswarn_hw("Invalid dma image format", instance, hw_ip);
		img_format = hw_mcsc->out_img_format[output_id];
		conv420_en = hw_mcsc->conv420_en[output_id];
	} else {
		hw_mcsc->out_img_format[output_id] = img_format;
		hw_mcsc->conv420_en[output_id] = conv420_en;
	}

	fimc_is_scaler_set_wdma_format(hw_ip->regs, output_id, img_format);
	fimc_is_scaler_set_wdma_10bit_type(hw_ip->regs, output_id, img_10bit_type);
	fimc_is_scaler_set_420_conversion(hw_ip->regs, output_id, 0, conv420_en);

	fimc_is_scaler_get_post_dst_size(hw_ip->regs, output_id, &scaled_width, &scaled_height);
	if ((scaled_width != 0) && (scaled_height != 0)) {
		if ((scaled_width != out_width) || (scaled_height != out_height)) {
			msdbg_hw(2, "Invalid output[%d] scaled size (%d/%d)(%d/%d)\n",
				instance, hw_ip, output_id, scaled_width, scaled_height,
				out_width, out_height);
			return -EINVAL;
		}
	}

	fimc_is_scaler_set_wdma_size(hw_ip->regs, output_id, out_width, out_height);

	fimc_is_scaler_set_wdma_stride(hw_ip->regs, output_id, y_stride, uv_stride);
	fimc_is_scaler_set_wdma_2bit_stride(hw_ip->regs, output_id, y_2bit_stride, uv_2bit_stride);

	return ret;
}

int fimc_is_hw_mcsc_hwfc_mode(struct fimc_is_hw_ip *hw_ip, struct param_mcs_input *input,
	u32 hwfc_output_ids, u32 instance)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);
	u32 input_id = 0, output_id;
	bool config = true;

	BUG_ON(!hw_ip);
	BUG_ON(!input);
	BUG_ON(!cap);
	BUG_ON(!hw_ip->priv_info);

	/* can't support this function */
	if (cap->hwfc != MCSC_CAP_SUPPORT)
		return ret;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (cap->enable_shared_output && !hwfc_output_ids)
		return 0;

	msdbg_hw(2, "hwfc_mode_setting output[0x%08X]\n", instance, hw_ip, hwfc_output_ids);

	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
		config = (input_id == hw_ip->id ? true: false);

		if ((config && (hwfc_output_ids & (1 << output_id)))
			|| (fimc_is_scaler_get_dma_out_enable(hw_ip->regs, output_id))) {
			msdbg_hw(2, "hwfc_mode_setting hwfc_output_ids(0x%x)\n",
				instance, hw_ip, hwfc_output_ids);
			fimc_is_scaler_set_hwfc_mode(hw_ip->regs, hwfc_output_ids);
			break;
		}
	}

	if (!config) {
		msdbg_hw(2, "hwfc_mode_setting skip\n", instance, hw_ip);
		return ret;
	}

	return ret;
}

int fimc_is_hw_mcsc_hwfc_output(struct fimc_is_hw_ip *hw_ip, struct param_mcs_output *output,
	u32 output_id, u32 instance)
{
	int ret = 0;
	u32 width, height, format, plane;
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!output);
	BUG_ON(!cap);
	BUG_ON(!hw_ip->priv_info);

	/* can't support this function */
	if (cap->out_hwfc[output_id] != MCSC_CAP_SUPPORT)
		return ret;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	width  = output->width;
	height = output->height;
	format = output->dma_format;
	plane = output->plane;

	msdbg_hw(2, "hwfc_config_setting mode(%dx%d, %d, %d)\n", instance, hw_ip,
			width, height, format, plane);

	if (output->hwfc)
		fimc_is_scaler_set_hwfc_config(hw_ip->regs, output_id, format, plane,
			(output_id * 3), width, height);

	return ret;
}

void fimc_is_hw_bchs_range(void __iomem *base_addr, u32 output_id, int yuv_range)
{
#ifdef ENABLE_10BIT_MCSC
	if (yuv_range == SCALER_OUTPUT_YUV_RANGE_FULL) {
		/* Y range - [0:1024], U/V range - [0:1024] */
		fimc_is_scaler_set_b_c(base_addr, output_id, 0, 1024);
		fimc_is_scaler_set_h_s(base_addr, output_id, 1024, 0, 0, 1024);
	} else {	/* YUV_RANGE_NARROW */
		/* Y range - [64:940], U/V range - [64:960] */
		fimc_is_scaler_set_b_c(base_addr, output_id, 0, 1024);
		fimc_is_scaler_set_h_s(base_addr, output_id, 1024, 0, 0, 1024);
	}
#else
	if (yuv_range == SCALER_OUTPUT_YUV_RANGE_FULL) {
		/* Y range - [0:255], U/V range - [0:255] */
		fimc_is_scaler_set_b_c(base_addr, output_id, 0, 256);
		fimc_is_scaler_set_h_s(base_addr, output_id, 256, 0, 0, 256);
	} else {	/* YUV_RANGE_NARROW */
		/* Y range - [16:235], U/V range - [16:239] */
		fimc_is_scaler_set_b_c(base_addr, output_id, 16, 220);
		fimc_is_scaler_set_h_s(base_addr, output_id, 224, 0, 0, 224);
	}
#endif
}

void fimc_is_hw_bchs_clamp(void __iomem *base_addr, u32 output_id, int yuv_range)
{
#ifdef ENABLE_10BIT_MCSC
	if (yuv_range == SCALER_OUTPUT_YUV_RANGE_FULL)
		fimc_is_scaler_set_bchs_clamp(base_addr, output_id, 1023, 0, 1023, 0);
	else	/* YUV_RANGE_NARROW */
		fimc_is_scaler_set_bchs_clamp(base_addr, output_id, 940, 64, 960, 64);
#else
	if (yuv_range == SCALER_OUTPUT_YUV_RANGE_FULL)
		fimc_is_scaler_set_bchs_clamp(base_addr, output_id, 255, 0, 255, 0);
	else	/* YUV_RANGE_NARROW */
		fimc_is_scaler_set_bchs_clamp(base_addr, output_id, 235, 16, 240, 16);
#endif
}

int fimc_is_hw_mcsc_output_yuvrange(struct fimc_is_hw_ip *hw_ip, struct param_mcs_output *output,
	u32 output_id, u32 instance)
{
	int ret = 0;
	int yuv_range = 0;
	u32 input_id = 0;
	bool config = true;
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
#if !defined(USE_YUV_RANGE_BY_ISP)
	enum exynos_sensor_position sensor_position;
	scaler_setfile_contents contents;
#endif
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip);
	BUG_ON(!output);
	BUG_ON(!hw_ip->priv_info);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;

	if (!test_bit(output_id, &hw_mcsc->out_en))
		return ret;

	input_id = fimc_is_scaler_get_scaler_path(hw_ip->regs, hw_ip->id, output_id);
	config = (input_id == hw_ip->id ? true: false);

	if (output->dma_cmd == DMA_OUTPUT_COMMAND_DISABLE) {
		if (cap->enable_shared_output == false
			|| (config && !test_bit(output_id, &hw_mcsc_out_configured)))
			fimc_is_scaler_set_bchs_enable(hw_ip->regs, output_id, 0);
		return ret;
	}

	yuv_range = output->yuv_range;
	hw_mcsc->yuv_range = yuv_range; /* save for ISP */

	fimc_is_scaler_set_bchs_enable(hw_ip->regs, output_id, 1);
#if !defined(USE_YUV_RANGE_BY_ISP)
	if (test_bit(HW_TUNESET, &hw_ip->state)) {
		/* set yuv range config value by scaler_param yuv_range mode */
		sensor_position = hw_ip->hardware->sensor_position[instance];
		contents = hw_mcsc->applied_setfile[sensor_position]->contents[yuv_range];
		fimc_is_scaler_set_b_c(hw_ip->regs, output_id,
			contents.y_offset, contents.y_gain);
		fimc_is_scaler_set_h_s(hw_ip->regs, output_id,
			contents.c_gain00, contents.c_gain01,
			contents.c_gain10, contents.c_gain11);
		msdbg_hw(2, "set YUV range(%d) by setfile parameter\n",
			instance, hw_ip, yuv_range);
		msdbg_hw(2, "[OUT:%d]output_yuv_setting: yuv_range(%d), cmd(O:%d,D:%d)\n",
			instance, hw_ip, output_id, yuv_range, output->otf_cmd, output->dma_cmd);
		dbg_hw(2, "[Y:offset(%d),gain(%d)][C:gain00(%d),01(%d),10(%d),11(%d)]\n",
			contents.y_offset, contents.y_gain,
			contents.c_gain00, contents.c_gain01,
			contents.c_gain10, contents.c_gain11);
	} else {
		fimc_is_hw_bchs_range(hw_ip->regs, output_id, yuv_range);
		msdbg_hw(2, "YUV range set default settings\n", instance, hw_ip);
	}
#else
	fimc_is_hw_bchs_range(hw_ip->regs, output_id, yuv_range);
	fimc_is_hw_bchs_clamp(hw_ip->regs, output_id, yuv_range);
#endif
	return ret;
}

int fimc_is_hw_mcsc_adjust_input_img_fmt(u32 format, u32 plane, u32 order, u32 *img_format)
{
	int ret = 0;

	switch (format) {
	case DMA_INPUT_FORMAT_YUV420:
		switch (plane) {
		case 2:
			switch (order) {
			case DMA_INPUT_ORDER_CbCr:
				*img_format = MCSC_YUV420_2P_UFIRST;
				break;
			case DMA_INPUT_ORDER_CrCb:
				* img_format = MCSC_YUV420_2P_VFIRST;
				break;
			default:
				err_hw("input order error - (%d/%d/%d)", format, order, plane);
				ret = -EINVAL;
				break;
			}
			break;
		case 3:
			*img_format = MCSC_YUV420_3P;
			break;
		default:
			err_hw("input plane error - (%d/%d/%d)", format, order, plane);
			ret = -EINVAL;
			break;
		}
		break;
	case DMA_INPUT_FORMAT_YUV422:
		switch (plane) {
		case 1:
			switch (order) {
			case DMA_INPUT_ORDER_CrYCbY:
				*img_format = MCSC_YUV422_1P_VYUY;
				break;
			case DMA_INPUT_ORDER_CbYCrY:
				*img_format = MCSC_YUV422_1P_UYVY;
				break;
			case DMA_INPUT_ORDER_YCrYCb:
				*img_format = MCSC_YUV422_1P_YVYU;
				break;
			case DMA_INPUT_ORDER_YCbYCr:
				*img_format = MCSC_YUV422_1P_YUYV;
				break;
			default:
				err_hw("img order error - (%d/%d/%d)", format, order, plane);
				ret = -EINVAL;
				break;
			}
			break;
		case 2:
			switch (order) {
			case DMA_INPUT_ORDER_CbCr:
				*img_format = MCSC_YUV422_2P_UFIRST;
				break;
			case DMA_INPUT_ORDER_CrCb:
				*img_format = MCSC_YUV422_2P_VFIRST;
				break;
			default:
				err_hw("img order error - (%d/%d/%d)", format, order, plane);
				ret = -EINVAL;
				break;
			}
			break;
		case 3:
			*img_format = MCSC_YUV422_3P;
			break;
		default:
			err_hw("img plane error - (%d/%d/%d)", format, order, plane);
			ret = -EINVAL;
			break;
		}
		break;
	default:
		err_hw("img format error - (%d/%d/%d)", format, order, plane);
		ret = -EINVAL;
		break;
	}

	return ret;
}


int fimc_is_hw_mcsc_adjust_output_img_fmt(u32 format, u32 plane, u32 order, u32 *img_format,
	bool *conv420_flag)
{
	int ret = 0;

	switch (format) {
	case DMA_OUTPUT_FORMAT_YUV420:
		if (conv420_flag)
			*conv420_flag = true;
		switch (plane) {
		case 2:
			switch (order) {
			case DMA_OUTPUT_ORDER_CbCr:
				*img_format = MCSC_YUV420_2P_UFIRST;
				break;
			case DMA_OUTPUT_ORDER_CrCb:
				* img_format = MCSC_YUV420_2P_VFIRST;
				break;
			default:
				err_hw("output order error - (%d/%d/%d)", format, order, plane);
				ret = -EINVAL;
				break;
			}
			break;
		case 3:
			*img_format = MCSC_YUV420_3P;
			break;
		default:
			err_hw("output plane error - (%d/%d/%d)", format, order, plane);
			ret = -EINVAL;
			break;
		}
		break;
	case DMA_OUTPUT_FORMAT_YUV422:
		if (conv420_flag)
			*conv420_flag = false;
		switch (plane) {
		case 1:
			switch (order) {
			case DMA_OUTPUT_ORDER_CrYCbY:
				*img_format = MCSC_YUV422_1P_VYUY;
				break;
			case DMA_OUTPUT_ORDER_CbYCrY:
				*img_format = MCSC_YUV422_1P_UYVY;
				break;
			case DMA_OUTPUT_ORDER_YCrYCb:
				*img_format = MCSC_YUV422_1P_YVYU;
				break;
			case DMA_OUTPUT_ORDER_YCbYCr:
				*img_format = MCSC_YUV422_1P_YUYV;
				break;
			default:
				err_hw("img order error - (%d/%d/%d)", format, order, plane);
				ret = -EINVAL;
				break;
			}
			break;
		case 2:
			switch (order) {
			case DMA_OUTPUT_ORDER_CbCr:
				*img_format = MCSC_YUV422_2P_UFIRST;
				break;
			case DMA_OUTPUT_ORDER_CrCb:
				*img_format = MCSC_YUV422_2P_VFIRST;
				break;
			default:
				err_hw("img order error - (%d/%d/%d)", format, order, plane);
				ret = -EINVAL;
				break;
			}
			break;
		case 3:
			*img_format = MCSC_YUV422_3P;
			break;
		default:
			err_hw("img plane error - (%d/%d/%d)", format, order, plane);
			ret = -EINVAL;
			break;
		}
		break;
	default:
		err_hw("img format error - (%d/%d/%d)", format, order, plane);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int fimc_is_hw_mcsc_check_format(enum mcsc_io_type type, u32 format, u32 bit_width,
	u32 width, u32 height)
{
	int ret = 0;

	switch (type) {
	case HW_MCSC_OTF_INPUT:
		/* check otf input */
		if (width < 16 || width > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Input width(%d)", width);
		}

		if (height < 16 || height > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Input height(%d)", height);
		}

		if (format != OTF_INPUT_FORMAT_YUV422) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Input format(%d)", format);
		}

		if (bit_width != OTF_INPUT_BIT_WIDTH_8BIT) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Input format(%d)", bit_width);
		}
		break;
	case HW_MCSC_OTF_OUTPUT:
		/* check otf output */
		if (width < 16 || width > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Output width(%d)", width);
		}

		if (height < 16 || height > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Output height(%d)", height);
		}

		if (format != OTF_OUTPUT_FORMAT_YUV422) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Output format(%d)", format);
		}

		if (bit_width != OTF_OUTPUT_BIT_WIDTH_8BIT) {
			ret = -EINVAL;
			err_hw("Invalid MCSC OTF Output format(%d)", bit_width);
		}
		break;
	case HW_MCSC_DMA_INPUT:
		/* check dma input */
		if (width < 16 || width > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Input width(%d)", width);
		}

		if (height < 16 || height > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Input height(%d)", height);
		}

		if (format != DMA_INPUT_FORMAT_YUV422) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Input format(%d)", format);
		}

		if (bit_width != DMA_INPUT_BIT_WIDTH_8BIT) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Input format(%d)", bit_width);
		}
		break;
	case HW_MCSC_DMA_OUTPUT:
		/* check dma output */
		if (width < 16 || width > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Output width(%d)", width);
		}

		if (height < 16 || height > 8192) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Output height(%d)", height);
		}

		if (!(format == DMA_OUTPUT_FORMAT_YUV422 ||
			format == DMA_OUTPUT_FORMAT_YUV420)) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Output format(%d)", format);
		}

		if (bit_width != DMA_OUTPUT_BIT_WIDTH_8BIT) {
			ret = -EINVAL;
			err_hw("Invalid MCSC DMA Output format(%d)", bit_width);
		}
		break;
	default:
		err_hw("Invalid MCSC type(%d)", type);
		break;
	}

	return ret;
}

void fimc_is_hw_mcsc_djag_init(struct fimc_is_hw_ip *hw_ip)
{
#ifdef ENABLE_DJAG_IN_MCSC
	struct fimc_is_hw_mcsc *hw_mcsc;
	struct fimc_is_hw_mcsc_cap *cap = GET_MCSC_HW_CAP(hw_ip);

	BUG_ON(!hw_ip->priv_info);
	BUG_ON(!cap);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	if (cap->djag == MCSC_CAP_SUPPORT) {
		hw_mcsc->djag_input_source = DEV_HW_MCSC0;
		hw_mcsc->djag_prescale_ratio = MCSC_DJAG_PRESCALE_INDEX_1;
	}

	/* djag config init */
	memcpy(&hw_mcsc->djag_tunecfg, &init_djag_cfgs, sizeof(struct djag_setfile_contents));

#endif
}

int fimc_is_hw_mcsc_update_djag_register(struct fimc_is_hw_ip *hw_ip,
		struct mcs_param *param,
		u32 instance)
{
	int ret = 0;
#ifdef ENABLE_DJAG_IN_MCSC
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
	struct fimc_is_hw_mcsc_cap *cap;
	u32 in_width, in_height;
	u32 out_width = 0, out_height = 0;
	ulong temp_width, temp_height;
	struct djag_setfile_contents djag_tuneset;
	u32 hratio, vratio, min_ratio;
	u32 scale_index = MCSC_DJAG_PRESCALE_INDEX_1;
	enum exynos_sensor_position sensor_position;
	int output_id = 0;

	BUG_ON(!hw_ip);
	BUG_ON(!hw_ip->priv_info);
	BUG_ON(!param);

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	cap = GET_MCSC_HW_CAP(hw_ip);
	sensor_position = hw_ip->hardware->sensor_position[instance];

	if (test_bit(DJAG_SET_DONE, &hw_mcsc->blk_set_ctrl))
		return ret;

	fimc_is_scaler_set_djag_input_source(hw_ip->regs, hw_mcsc->djag_input_source);

	if (hw_mcsc->djag_input_source != (hw_ip->id - DEV_HW_MCSC0)) {
		sdbg_hw(2, "this instance does not use DJAG.\n", hw_ip);
		return ret;
	}

	djag_tuneset = hw_mcsc->djag_tunecfg;

	in_width = param->input.width;
	in_height = param->input.height;

	/* select compare output_port : select max output size */
	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		if (test_bit(output_id, &hw_mcsc->out_en)) {
			if (out_width <= param->output[output_id].width) {
				out_width = param->output[output_id].width;
				out_height = param->output[output_id].height;
			}
		}
	}

	if (param->input.width > out_width) {
		sdbg_hw(2, "DJAG is not applied still.\n", hw_ip);
		return ret;
	}

	/* check scale ratio if over 2.5 times */
	if ((out_width * 10 > in_width * 25) || (out_height * 10 > in_height * 25)) {
		out_width = round_down(in_width * 25 / 10, 2);
		out_height = round_down(in_height * 25 / 10, 2);
	}

	hratio = GET_DJAG_ZOOM_RATIO(in_width, out_width);
	vratio = GET_DJAG_ZOOM_RATIO(in_height, out_height);
	min_ratio = min(hratio, vratio);
	if (min_ratio >= GET_DJAG_ZOOM_RATIO(10, 10)) /* Zoom Ratio = 1.0 */
		scale_index = MCSC_DJAG_PRESCALE_INDEX_1;
	else if (min_ratio >= GET_DJAG_ZOOM_RATIO(10, 14)) /* Zoom Ratio = 1.4 */
		scale_index = MCSC_DJAG_PRESCALE_INDEX_2;
	else if (min_ratio >= GET_DJAG_ZOOM_RATIO(10, 20)) /* Zoom Ratio = 2.0 */
		scale_index = MCSC_DJAG_PRESCALE_INDEX_3;
	else
		scale_index = MCSC_DJAG_PRESCALE_INDEX_4;

	/* overwrite mcsc param for poly-phase */
	for (output_id = MCSC_OUTPUT0; output_id < cap->max_output; output_id++) {
		if (test_bit(output_id, &hw_mcsc->out_en)) {
			if ((param->output[output_id].crop_offset_x != 0) || (param->output[output_id].crop_offset_y != 0)) {
				temp_width = (ulong)in_width;
				temp_height = (ulong)in_height;
				out_width = (u32)round_down((temp_width << MCSC_PRECISION) / min_ratio, 2);
				out_height = (u32)round_down((temp_height << MCSC_PRECISION) / min_ratio, 2);
				hratio = GET_DJAG_ZOOM_RATIO(in_width, out_width);
				vratio = GET_DJAG_ZOOM_RATIO(in_height, out_height);
				param->output[output_id].crop_offset_x =
					ALIGN(CONVRES(param->output[output_id].crop_offset_x, in_width, out_width), 2);
				param->output[output_id].crop_offset_y =
					ALIGN(CONVRES(param->output[output_id].crop_offset_y, in_height, out_height), 2);
			}
			param->output[output_id].crop_width = ALIGN(out_width - param->output[output_id].crop_offset_x * 2, 2);
			param->output[output_id].crop_height = ALIGN(out_height - param->output[output_id].crop_offset_y * 2, 2);
		}
	}

	/* djag image size setting */
	fimc_is_scaler_set_djag_src_size(hw_ip->regs, in_width, in_height);
	fimc_is_scaler_set_djag_dst_size(hw_ip->regs, out_width, out_height);
	fimc_is_scaler_set_djag_scaling_ratio(hw_ip->regs, hratio, vratio);
	fimc_is_scaler_set_djag_init_phase_offset(hw_ip->regs, 0, 0);
	fimc_is_scaler_set_djag_round_mode(hw_ip->regs, 1);

#ifdef MCSC_USE_DEJAG_TUNING_PARAM
	djag_tuneset = hw_mcsc->applied_setfile[sensor_position]->djag_contents[scale_index];
#endif
	fimc_is_scaler_set_djag_tunning_param(hw_ip->regs, djag_tuneset);

	fimc_is_scaler_set_djag_enable(hw_ip->regs, 1);

	set_bit(DJAG_SET_DONE, &hw_mcsc->blk_set_ctrl);
#endif
	return ret;
}

int fimc_is_hw_mcsc_update_dsvra_register(struct fimc_is_hw_ip *hw_ip,
	struct is_param_region *param, u32 instance, enum mcsc_port dsvra_inport)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
	struct mcs_param *mcs_param;
	struct fimc_is_hw_mcsc_cap *cap;
	u32 in_width, in_height;
	u32 out_width, out_height;
	u32 hratio, vratio;
	ulong temp_width, temp_height;

	BUG_ON(!hw_ip);
	BUG_ON(!hw_ip->priv_info);
	BUG_ON(!param);

	mcs_param = &param->mcs;
	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	cap = GET_MCSC_HW_CAP(hw_ip);

	if (!test_bit(MCSC_OUTPUT_DS, &hw_mcsc_out_configured))
		return -EINVAL;

	if (dsvra_inport == MCSC_PORT_NONE)
		return -EINVAL;

	in_width = mcs_param->output[dsvra_inport].width;
	in_height = mcs_param->output[dsvra_inport].height;
	out_width = mcs_param->output[MCSC_OUTPUT_DS].width;
	out_height = mcs_param->output[MCSC_OUTPUT_DS].height;

	temp_width = (ulong)in_width;
	temp_height = (ulong)in_height;
	hratio = (u32)((temp_width << MCSC_PRECISION) / out_width);
	vratio = (u32)((temp_height << MCSC_PRECISION) / out_height);

	fimc_is_scaler_set_ds_src_size(hw_ip->regs, in_width, in_height);
	fimc_is_scaler_set_ds_dst_size(hw_ip->regs, out_width, out_height);
	fimc_is_scaler_set_ds_scaling_ratio(hw_ip->regs, hratio, vratio);
	fimc_is_scaler_set_ds_init_phase_offset(hw_ip->regs, 0, 0);
	fimc_is_scaler_set_ds_gamma_table_enable(hw_ip->regs, true);
	ret = fimc_is_hw_mcsc_dma_output(hw_ip, &mcs_param->output[MCSC_OUTPUT_DS], MCSC_OUTPUT_DS, instance);

	fimc_is_scaler_set_otf_out_path(hw_ip->regs, dsvra_inport);
	fimc_is_scaler_set_ds_enable(hw_ip->regs, true);
	msdbg_hw(2, "%s: dsvra_inport(%d)\n", instance, hw_ip, __func__, dsvra_inport);

	return ret;
}

int fimc_is_hw_mcsc_update_ysum_register(struct fimc_is_hw_ip *hw_ip,
	struct is_param_region *param, enum mcsc_port ysumport)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
	struct mcs_param *mcs_param;
	struct fimc_is_hw_mcsc_cap *cap;
	u32 width, height;
	u32 start_x = 0, start_y = 0;

	BUG_ON(!hw_ip);
	BUG_ON(!hw_ip->priv_info);
	BUG_ON(!param);

	mcs_param = &param->mcs;
	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	cap = GET_MCSC_HW_CAP(hw_ip);

	if (test_bit(YSUM_SET_DONE, &hw_mcsc->blk_set_ctrl))
		return ret;

	if (ysumport == MCSC_PORT_NONE)
		return ret;

	if (!fimc_is_scaler_get_dma_out_enable(hw_ip->regs, ysumport))
		return ret;

	switch (ysumport) {
	case MCSC_PORT_0:
		width = mcs_param->output[MCSC_OUTPUT0].width;
		height = mcs_param->output[MCSC_OUTPUT0].height;
		break;
	case MCSC_PORT_1:
		width = mcs_param->output[MCSC_OUTPUT1].width;
		height = mcs_param->output[MCSC_OUTPUT1].height;
		break;
	case MCSC_PORT_2:
		width = mcs_param->output[MCSC_OUTPUT2].width;
		height = mcs_param->output[MCSC_OUTPUT2].height;
		break;
	case MCSC_PORT_3:
		width = mcs_param->output[MCSC_OUTPUT3].width;
		height = mcs_param->output[MCSC_OUTPUT3].height;
		break;
	case MCSC_PORT_4:
		width = mcs_param->output[MCSC_OUTPUT4].width;
		height = mcs_param->output[MCSC_OUTPUT4].height;
		break;
	default:
		goto no_setting_ysum;
		break;
	}
	fimc_is_scaler_set_ysum_image_size(hw_ip->regs, width, height, start_x, start_y);

	fimc_is_scaler_set_ysum_input_sourece_enable(hw_ip->regs, ysumport, true);

	set_bit(YSUM_SET_DONE, &hw_mcsc->blk_set_ctrl);

no_setting_ysum:
	return ret;
}

static void fimc_is_hw_mcsc_size_dump(struct fimc_is_hw_ip *hw_ip)
{
	int i;
	u32 input_src = 0;
	u32 in_w, in_h = 0;
	u32 rdma_w, rdma_h = 0;
	u32 poly_src_w, poly_src_h = 0;
	u32 poly_dst_w, poly_dst_h = 0;
	u32 post_in_w, post_in_h = 0;
	u32 post_out_w, post_out_h = 0;
	u32 wdma_enable = 0;
	u32 wdma_w, wdma_h = 0;
	u32 rdma_y_stride, rdma_uv_stride = 0;
	u32 wdma_y_stride, wdma_uv_stride = 0;
	struct fimc_is_hw_mcsc_cap *cap;

	BUG_ON(!hw_ip);

	/* skip size dump, if hw_ip wasn't opened */
	if (!test_bit(HW_OPEN, &hw_ip->state))
		return;

	cap = GET_MCSC_HW_CAP(hw_ip);
	if (!cap) {
		err_hw("failed to get hw_mcsc_cap(%p)", cap);
		return;
	}

	input_src = fimc_is_scaler_get_input_source(hw_ip->regs, hw_ip->id);
	fimc_is_scaler_get_input_img_size(hw_ip->regs, hw_ip->id, &in_w, &in_h);
	fimc_is_scaler_get_rdma_size(hw_ip->regs, &rdma_w, &rdma_h);
	fimc_is_scaler_get_rdma_stride(hw_ip->regs, &rdma_y_stride, &rdma_uv_stride);

	sdbg_hw(2, "=SIZE=====================================\n"
		"[INPUT] SRC:%d(0:OTF, 1:DMA), SIZE:%dx%d\n"
		"[RDMA] SIZE:%dx%d, STRIDE: Y:%d, UV:%d\n",
		hw_ip, input_src, in_w, in_h,
		rdma_w, rdma_h, rdma_y_stride, rdma_uv_stride);

	for (i = MCSC_OUTPUT0; i < cap->max_output; i++) {
		fimc_is_scaler_get_poly_src_size(hw_ip->regs, i, &poly_src_w, &poly_src_h);
		fimc_is_scaler_get_poly_dst_size(hw_ip->regs, i, &poly_dst_w, &poly_dst_h);
		fimc_is_scaler_get_post_img_size(hw_ip->regs, i, &post_in_w, &post_in_h);
		fimc_is_scaler_get_post_dst_size(hw_ip->regs, i, &post_out_w, &post_out_h);
		fimc_is_scaler_get_wdma_size(hw_ip->regs, i, &wdma_w, &wdma_h);
		fimc_is_scaler_get_wdma_stride(hw_ip->regs, i, &wdma_y_stride, &wdma_uv_stride);
		wdma_enable = fimc_is_scaler_get_dma_out_enable(hw_ip->regs, i);

		dbg_hw(2, "[POLY%d] SRC:%dx%d, DST:%dx%d\n"
			"[POST%d] SRC:%dx%d, DST:%dx%d\n"
			"[WDMA%d] ENABLE:%d, SIZE:%dx%d, STRIDE: Y:%d, UV:%d\n",
			i, poly_src_w, poly_src_h, poly_dst_w, poly_dst_h,
			i, post_in_w, post_in_h, post_out_w, post_out_h,
			i, wdma_enable, wdma_w, wdma_h, wdma_y_stride, wdma_uv_stride);
	}
	sdbg_hw(2, "==========================================\n", hw_ip);

	return;
}

static int fimc_is_hw_mcsc_get_meta(struct fimc_is_hw_ip *hw_ip,
		struct fimc_is_frame *frame, unsigned long hw_map)
{
	int ret = 0;
	struct fimc_is_hw_mcsc *hw_mcsc;

	if (unlikely(!frame)) {
		mserr_hw("get_meta: frame is null", atomic_read(&hw_ip->instance), hw_ip);
		return 0;
	}

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip->priv_info;
	if (unlikely(!hw_mcsc)) {
		mserr_hw("priv_info is NULL", frame->instance, hw_ip);
		return -EINVAL;
	}

	fimc_is_scaler_get_ysum_result(hw_ip->regs,
		&frame->shot->udm.scaler.ysumdata.higher_ysum_value,
		&frame->shot->udm.scaler.ysumdata.lower_ysum_value);

	return ret;
}

const struct fimc_is_hw_ip_ops fimc_is_hw_mcsc_ops = {
	.open			= fimc_is_hw_mcsc_open,
	.init			= fimc_is_hw_mcsc_init,
	.close			= fimc_is_hw_mcsc_close,
	.enable			= fimc_is_hw_mcsc_enable,
	.disable		= fimc_is_hw_mcsc_disable,
	.shot			= fimc_is_hw_mcsc_shot,
	.set_param		= fimc_is_hw_mcsc_set_param,
	.get_meta		= fimc_is_hw_mcsc_get_meta,
	.frame_ndone		= fimc_is_hw_mcsc_frame_ndone,
	.load_setfile		= fimc_is_hw_mcsc_load_setfile,
	.apply_setfile		= fimc_is_hw_mcsc_apply_setfile,
	.delete_setfile		= fimc_is_hw_mcsc_delete_setfile,
	.size_dump		= fimc_is_hw_mcsc_size_dump,
	.clk_gate		= fimc_is_hardware_clk_gate
};

int fimc_is_hw_mcsc_probe(struct fimc_is_hw_ip *hw_ip, struct fimc_is_interface *itf,
	struct fimc_is_interface_ischain *itfc, int id, const char *name)
{
	int ret = 0;
	int hw_slot = -1;

	BUG_ON(!hw_ip);
	BUG_ON(!itf);
	BUG_ON(!itfc);

	/* initialize device hardware */
	hw_ip->id   = id;
	snprintf(hw_ip->name, sizeof(hw_ip->name), "%s", name);
	hw_ip->ops  = &fimc_is_hw_mcsc_ops;
	hw_ip->itf  = itf;
	hw_ip->itfc = itfc;
	atomic_set(&hw_ip->fcount, 0);
	hw_ip->internal_fcount = 0;
	hw_ip->is_leader = true;
	atomic_set(&hw_ip->status.Vvalid, V_BLANK);
	atomic_set(&hw_ip->rsccount, 0);
	init_waitqueue_head(&hw_ip->status.wait_queue);

	/* set mcsc sfr base address */
	hw_slot = fimc_is_hw_slot_id(id);
	if (!valid_hw_slot_id(hw_slot)) {
		serr_hw("invalid hw_slot (%d)", hw_ip, hw_slot);
		return -EINVAL;
	}

	/* set mcsc interrupt handler */
	itfc->itf_ip[hw_slot].handler[INTR_HWIP1].handler = &fimc_is_hw_mcsc_handle_interrupt;

	clear_bit(HW_OPEN, &hw_ip->state);
	clear_bit(HW_INIT, &hw_ip->state);
	clear_bit(HW_CONFIG, &hw_ip->state);
	clear_bit(HW_RUN, &hw_ip->state);
	clear_bit(HW_TUNESET, &hw_ip->state);
	spin_lock_init(&shared_output_slock);

	sinfo_hw("probe done\n", hw_ip);

	return ret;
}
