/*
 * Samsung EXYNOS FIMC-IS (Imaging Subsystem) driver
 *
 * Copyright (C) 2014 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "fimc-is-hw-3aa.h"
#include "fimc-is-hw-mcscaler-v2.h"
#include "fimc-is-hw-isp.h"
#include "fimc-is-err.h"

extern struct fimc_is_lib_support gPtr_lib_support;

static int fimc_is_hw_isp_open(struct fimc_is_hw_ip *hw_ip, u32 instance,
	struct fimc_is_group *group)
{
	int ret = 0;
	struct fimc_is_hw_isp *hw_isp = NULL;

	BUG_ON(!hw_ip);

	if (test_bit(HW_OPEN, &hw_ip->state))
		return 0;

	frame_manager_probe(hw_ip->framemgr, FRAMEMGR_ID_HW | (1 << hw_ip->id), "HWISP");
	frame_manager_probe(hw_ip->framemgr_late, FRAMEMGR_ID_HW | (1 << hw_ip->id) | 0xF000, "HWISP LATE");
	frame_manager_open(hw_ip->framemgr, FIMC_IS_MAX_HW_FRAME);
	frame_manager_open(hw_ip->framemgr_late, FIMC_IS_MAX_HW_FRAME_LATE);

	hw_ip->priv_info = vzalloc(sizeof(struct fimc_is_hw_isp));
	if(!hw_ip->priv_info) {
		mserr_hw("hw_ip->priv_info(null)", instance, hw_ip);
		ret = -ENOMEM;
		goto err_alloc;
	}

	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;
#ifdef ENABLE_FPSIMD_FOR_USER
	fpsimd_get();
	ret = get_lib_func(LIB_FUNC_ISP, (void **)&hw_isp->lib_func);
	fpsimd_put();
#else
	ret = get_lib_func(LIB_FUNC_ISP, (void **)&hw_isp->lib_func);
#endif

	if (hw_isp->lib_func == NULL) {
		mserr_hw("hw_isp->lib_func(null)", instance, hw_ip);
		fimc_is_load_clear();
		ret = -EINVAL;
		goto err_lib_func;
	}
	msinfo_hw("get_lib_func is set\n", instance, hw_ip);

	hw_isp->lib_support = &gPtr_lib_support;
	hw_isp->lib[instance].func = hw_isp->lib_func;

	ret = fimc_is_lib_isp_chain_create(hw_ip, &hw_isp->lib[instance], instance);
	if (ret) {
		mserr_hw("chain create fail", instance, hw_ip);
		ret = -EINVAL;
		goto err_chain_create;
	}

	set_bit(HW_OPEN, &hw_ip->state);
	msdbg_hw(2, "open: [G:0x%x], framemgr[%s]", instance, hw_ip,
		GROUP_ID(group->id), hw_ip->framemgr->name);

	return 0;

err_chain_create:
err_lib_func:
	vfree(hw_ip->priv_info);
err_alloc:
	frame_manager_close(hw_ip->framemgr);
	frame_manager_close(hw_ip->framemgr_late);
	return ret;
}

static int fimc_is_hw_isp_init(struct fimc_is_hw_ip *hw_ip, u32 instance,
	struct fimc_is_group *group, bool flag, u32 module_id)
{
	int ret = 0;
	struct fimc_is_hw_isp *hw_isp = NULL;

	BUG_ON(!hw_ip);
	BUG_ON(!hw_ip->priv_info);
	BUG_ON(!group);

	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;

	hw_isp->lib[instance].object = NULL;
	hw_isp->lib[instance].func   = hw_isp->lib_func;
	hw_isp->param_set[instance].reprocessing = flag;

	if (hw_isp->lib[instance].object != NULL) {
		msdbg_hw(2, "object is already created\n", instance, hw_ip);
	} else {
		ret = fimc_is_lib_isp_object_create(hw_ip, &hw_isp->lib[instance],
				instance, (u32)flag, module_id);
		if (ret) {
			mserr_hw("object create fail", instance, hw_ip);
			return -EINVAL;
		}
	}

	set_bit(HW_INIT, &hw_ip->state);
	return ret;
}

static int fimc_is_hw_isp_deinit(struct fimc_is_hw_ip *hw_ip, u32 instance)
{
	int ret = 0;
	struct fimc_is_hw_isp *hw_isp;

	BUG_ON(!hw_ip);
	BUG_ON(!hw_ip->priv_info);

	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;

	fimc_is_lib_isp_object_destroy(hw_ip, &hw_isp->lib[instance], instance);
        hw_isp->lib[instance].object = NULL;

	return ret;
}

static int fimc_is_hw_isp_close(struct fimc_is_hw_ip *hw_ip, u32 instance)
{
	int ret = 0;
	struct fimc_is_hw_isp *hw_isp;

	BUG_ON(!hw_ip);

	if (!test_bit(HW_OPEN, &hw_ip->state))
		return 0;

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;
	BUG_ON(!hw_isp->lib_support);

	fimc_is_lib_isp_chain_destroy(hw_ip, &hw_isp->lib[instance], instance);
	vfree(hw_ip->priv_info);
	frame_manager_close(hw_ip->framemgr);
	frame_manager_close(hw_ip->framemgr_late);

	clear_bit(HW_OPEN, &hw_ip->state);
	msinfo_hw("close (%d)\n", instance, hw_ip, atomic_read(&hw_ip->rsccount));

	return ret;
}

static int fimc_is_hw_isp_enable(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	int ret = 0;

	BUG_ON(!hw_ip);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -EINVAL;
	}

	set_bit(HW_RUN, &hw_ip->state);

	return ret;
}

static int fimc_is_hw_isp_disable(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	int ret = 0;
	long timetowait;
	struct fimc_is_hw_isp *hw_isp;
	struct isp_param_set *param_set;

	BUG_ON(!hw_ip);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	msinfo_hw("isp_disable: Vvalid(%d)\n", instance, hw_ip,
		atomic_read(&hw_ip->status.Vvalid));

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;
	param_set = &hw_isp->param_set[instance];

	timetowait = wait_event_timeout(hw_ip->status.wait_queue,
		!atomic_read(&hw_ip->status.Vvalid),
		FIMC_IS_HW_STOP_TIMEOUT);

	if (!timetowait) {
		mserr_hw("wait FRAME_END timeout (%ld)", instance,
			hw_ip, timetowait);
		ret = -ETIME;
	}

	param_set->fcount = 0;
	if (test_bit(HW_RUN, &hw_ip->state)) {
		/* TODO: need to kthread_flush when isp use task */
		fimc_is_lib_isp_stop(hw_ip, &hw_isp->lib[instance], instance);
	} else {
		msdbg_hw(2, "already disabled\n", instance, hw_ip);
	}

	if (atomic_read(&hw_ip->rsccount) > 1)
		return 0;

	clear_bit(HW_RUN, &hw_ip->state);
	clear_bit(HW_CONFIG, &hw_ip->state);

	return ret;
}

int fimc_is_hw_isp_set_yuv_range(struct fimc_is_hw_ip *hw_ip,
	struct isp_param_set *param_set, u32 fcount, ulong hw_map)
{
	int ret = 0;
	struct fimc_is_hw_ip *hw_ip_mcsc = NULL;
	struct fimc_is_hw_mcsc *hw_mcsc = NULL;
	enum fimc_is_hardware_id hw_id = DEV_HW_END;
	int hw_slot = 0;
	int yuv_range = 0; /* 0: FULL, 1: NARROW */

	if (test_bit(DEV_HW_MCSC0, &hw_map))
		hw_id = DEV_HW_MCSC0;
	else if (test_bit(DEV_HW_MCSC1, &hw_map))
		hw_id = DEV_HW_MCSC1;

	hw_slot = fimc_is_hw_slot_id(hw_id);
	if (valid_hw_slot_id(hw_slot)) {
		hw_ip_mcsc = &hw_ip->hardware->hw_ip[hw_slot];
		BUG_ON(!hw_ip_mcsc->priv_info);
		hw_mcsc = (struct fimc_is_hw_mcsc *)hw_ip_mcsc->priv_info;
		yuv_range = hw_mcsc->yuv_range;
	}

	if (yuv_range == SCALER_OUTPUT_YUV_RANGE_NARROW) {
		switch (param_set->otf_output.format) {
		case OTF_OUTPUT_FORMAT_YUV444:
			param_set->otf_output.format = OTF_OUTPUT_FORMAT_YUV444_TRUNCATED;
			break;
		case OTF_OUTPUT_FORMAT_YUV422:
			param_set->otf_output.format = OTF_OUTPUT_FORMAT_YUV422_TRUNCATED;
			break;
		default:
			break;
		}

		switch (param_set->dma_output_yuv.format) {
		case DMA_OUTPUT_FORMAT_YUV444:
			param_set->dma_output_yuv.format = DMA_OUTPUT_FORMAT_YUV444_TRUNCATED;
			break;
		case DMA_OUTPUT_FORMAT_YUV422:
			param_set->dma_output_yuv.format = DMA_OUTPUT_FORMAT_YUV422_TRUNCATED;
			break;
		default:
			break;
		}
	}

	dbg_hw(2, "[%d][F:%d]%s: OTF[%d]%s(%d), DMA[%d]%s(%d)\n",
		param_set->instance_id, fcount, __func__,
		param_set->otf_output.cmd,
		(param_set->otf_output.format >= OTF_OUTPUT_FORMAT_YUV444_TRUNCATED ? "N": "W"),
		param_set->otf_output.format,
		param_set->dma_output_yuv.cmd,
		(param_set->dma_output_yuv.format >= DMA_OUTPUT_FORMAT_YUV444_TRUNCATED ? "N": "W"),
		param_set->dma_output_yuv.format);

	return ret;
}

static int fimc_is_hw_isp_shot(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame,
	ulong hw_map)
{
	int ret = 0;
	int i;
	struct fimc_is_hw_isp *hw_isp;
	struct isp_param_set *param_set;
	struct is_region *region;
	struct isp_param *param;
	struct fimc_is_group *head;
	u32 lindex, hindex;
	bool frame_done = false;
	u32 fcount = frame->fcount + frame->cur_buf_index;

	BUG_ON(!hw_ip);
	BUG_ON(!frame);

	msdbgs_hw(2, "[F:%d]shot\n", frame->instance, hw_ip, frame->fcount);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", frame->instance, hw_ip);
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

	fimc_is_hw_g_ctrl(hw_ip, hw_ip->id, HW_G_CTRL_FRM_DONE_WITH_DMA, (void *)&frame_done);
	if ((!frame_done)
		|| (!test_bit(ENTRY_IXC, &frame->out_flag) && !test_bit(ENTRY_IXP, &frame->out_flag)))
		set_bit(hw_ip->id, &frame->core_flag);

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;
	param_set = &hw_isp->param_set[frame->instance];
	region = hw_ip->region[frame->instance];
	BUG_ON(!region);

	param = &region->parameter.isp;

	if (frame->type == SHOT_TYPE_INTERNAL) {
		/* OTF INPUT case */
		param_set->dma_input.cmd = DMA_INPUT_COMMAND_DISABLE;
		param_set->input_dva[0] = 0x0;
		param_set->dma_output_chunk.cmd = DMA_OUTPUT_COMMAND_DISABLE;
		param_set->output_dva_chunk[0] = 0x0;
		param_set->dma_output_yuv.cmd  = DMA_OUTPUT_COMMAND_DISABLE;
		param_set->output_dva_yuv[0] = 0x0;
		hw_ip->internal_fcount = fcount;
		goto config;
	} else {
		BUG_ON(!frame->shot);
		/* per-frame control
		* check & update size from region */
		lindex = frame->shot->ctl.vendor_entry.lowIndexParam;
		hindex = frame->shot->ctl.vendor_entry.highIndexParam;

		if (hw_ip->internal_fcount != 0) {
			hw_ip->internal_fcount = 0;
			param_set->dma_output_chunk.cmd = param->vdma4_output.cmd;
			param_set->dma_output_yuv.cmd  = param->vdma5_output.cmd;
		}
	}

	fimc_is_hw_isp_update_param(hw_ip, region, param_set, lindex, hindex,
		frame->instance);

	/* DMA settings */
	if (param_set->dma_input.cmd != DMA_INPUT_COMMAND_DISABLE) {
		for (i = 0; i < frame->num_buffers; i++) {
			param_set->input_dva[i] = frame->dvaddr_buffer[frame->cur_buf_index + i];
			if (frame->dvaddr_buffer[i] == 0) {
				msinfo_hw("[F:%d]dvaddr_buffer[%d] is zero",
					frame->instance, hw_ip, frame->fcount, i);
				BUG_ON(1);
			}
		}
	}

	if (param_set->dma_output_chunk.cmd != DMA_OUTPUT_COMMAND_DISABLE) {
		for (i = 0; i < frame->num_buffers; i++) {
			param_set->output_dva_chunk[i] = frame->ixpTargetAddress[frame->cur_buf_index + i];
			if (frame->ixpTargetAddress[i] == 0) {
				msinfo_hw("[F:%d]ixpTargetAddress[%d] is zero",
					frame->instance, hw_ip, frame->fcount, i);
				param_set->dma_output_chunk.cmd = DMA_OUTPUT_COMMAND_DISABLE;
			}
		}
	}

	if (param_set->dma_output_yuv.cmd != DMA_OUTPUT_COMMAND_DISABLE) {
		for (i = 0; i < frame->num_buffers; i++) {
			param_set->output_dva_yuv[i] = frame->ixcTargetAddress[frame->cur_buf_index + i];
			if (frame->ixcTargetAddress[i] == 0) {
				msinfo_hw("[F:%d]ixcTargetAddress[%d] is zero",
					frame->instance, hw_ip, frame->fcount, i);
				param_set->dma_output_yuv.cmd = DMA_OUTPUT_COMMAND_DISABLE;
			}
		}
	}

config:
	param_set->instance_id = frame->instance;
	param_set->fcount = fcount;

	/* multi-buffer */
	if (frame->num_buffers)
		hw_ip->num_buffers = frame->num_buffers;

	if (frame->type == SHOT_TYPE_INTERNAL) {
		fimc_is_log_write("[@][DRV][%d]isp_shot [T:%d][R:%d][F:%d][IN:0x%x] [%d][OUT:0x%x]\n",
			param_set->instance_id, frame->type, param_set->reprocessing,
			param_set->fcount, param_set->input_dva[0],
			param_set->dma_output_yuv.cmd, param_set->output_dva_yuv[0]);
	}

	if (frame->shot) {
		ret = fimc_is_lib_isp_set_ctrl(hw_ip, &hw_isp->lib[frame->instance], frame);
		if (ret)
			mserr_hw("set_ctrl fail", frame->instance, hw_ip);
	}

	if (param_set->otf_input.cmd == OTF_INPUT_COMMAND_ENABLE) {
		struct fimc_is_hw_ip *hw_ip_3aa = NULL;
		struct fimc_is_hw_3aa *hw_3aa = NULL;
		enum fimc_is_hardware_id hw_id = DEV_HW_END;
		int hw_slot = 0;

		if (test_bit(DEV_HW_3AA0, &hw_map))
			hw_id = DEV_HW_3AA0;
		else if (test_bit(DEV_HW_3AA1, &hw_map))
			hw_id = DEV_HW_3AA1;

		hw_slot = fimc_is_hw_slot_id(hw_id);
		if (valid_hw_slot_id(hw_slot)) {
			hw_ip_3aa = &hw_ip->hardware->hw_ip[hw_slot];
			BUG_ON(!hw_ip_3aa->priv_info);
			hw_3aa = (struct fimc_is_hw_3aa *)hw_ip_3aa->priv_info;
			param_set->taa_param = &hw_3aa->param_set[frame->instance];
			/* When the ISP shot is requested, DDK needs to know the size fo 3AA.
			   This is because DDK calculates the position of the cropped image
			   from the 3AA size. */
			fimc_is_hw_3aa_update_param(hw_ip,
				region, param_set->taa_param,
				lindex, hindex, frame->instance);
		}
	}

	ret = fimc_is_hw_isp_set_yuv_range(hw_ip, param_set, frame->fcount, hw_map);
	fimc_is_lib_isp_shot(hw_ip, &hw_isp->lib[frame->instance], param_set, frame->shot);

	set_bit(HW_CONFIG, &hw_ip->state);

	return ret;
}

static int fimc_is_hw_isp_set_param(struct fimc_is_hw_ip *hw_ip, struct is_region *region,
	u32 lindex, u32 hindex, u32 instance, ulong hw_map)
{
	int ret = 0;
	struct fimc_is_hw_isp *hw_isp;
	struct isp_param_set *param_set;

	BUG_ON(!hw_ip);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -EINVAL;
	}

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;
	param_set = &hw_isp->param_set[instance];

	hw_ip->region[instance] = region;
	hw_ip->lindex[instance] = lindex;
	hw_ip->hindex[instance] = hindex;

	fimc_is_hw_isp_update_param(hw_ip, region, param_set, lindex, hindex, instance);

	return ret;
}

void fimc_is_hw_isp_update_param(struct fimc_is_hw_ip *hw_ip, struct is_region *region,
	struct isp_param_set *param_set, u32 lindex, u32 hindex, u32 instance)
{
	struct isp_param *param;

	BUG_ON(!region);
	BUG_ON(!param_set);

	param = &region->parameter.isp;
	param_set->instance_id = instance;

	/* check input */
	if (lindex & LOWBIT_OF(PARAM_ISP_OTF_INPUT)) {
		memcpy(&param_set->otf_input, &param->otf_input,
			sizeof(struct param_otf_input));
	}

	if (lindex & LOWBIT_OF(PARAM_ISP_VDMA1_INPUT)) {
		memcpy(&param_set->dma_input, &param->vdma1_input,
			sizeof(struct param_dma_input));
	}

	/* check output*/
	if (lindex & LOWBIT_OF(PARAM_ISP_OTF_OUTPUT)) {
		memcpy(&param_set->otf_output, &param->otf_output,
			sizeof(struct param_otf_output));
	}

	if (lindex & LOWBIT_OF(PARAM_ISP_VDMA4_OUTPUT)) {
		memcpy(&param_set->dma_output_chunk, &param->vdma4_output,
			sizeof(struct param_dma_output));
	}

	if (lindex & LOWBIT_OF(PARAM_ISP_VDMA5_OUTPUT)) {
		memcpy(&param_set->dma_output_yuv, &param->vdma5_output,
			sizeof(struct param_dma_output));
	}
}

static int fimc_is_hw_isp_get_meta(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame,
	ulong hw_map)
{
	int ret = 0;
	struct fimc_is_hw_isp *hw_isp;

	BUG_ON(!hw_ip);
	BUG_ON(!frame);

	if (!test_bit_variables(hw_ip->id, &hw_map))
		return 0;

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;

	ret = fimc_is_lib_isp_get_meta(hw_ip, &hw_isp->lib[frame->instance], frame);
	if (ret)
		mserr_hw("get_meta fail", frame->instance, hw_ip);

	return ret;
}

static int fimc_is_hw_isp_frame_ndone(struct fimc_is_hw_ip *hw_ip, struct fimc_is_frame *frame,
	u32 instance, enum ShotErrorType done_type)
{
	int ret = 0;
	int wq_id_ixc, wq_id_ixp, output_id;
	bool flag_get_meta = true;
	struct fimc_is_group *head;

	BUG_ON(!hw_ip);
	BUG_ON(!frame);

	switch (hw_ip->id) {
	case DEV_HW_ISP0:
		wq_id_ixc = WORK_I0C_FDONE;
		wq_id_ixp = WORK_I0P_FDONE;
		break;
	case DEV_HW_ISP1:
		wq_id_ixc = WORK_I1C_FDONE;
		wq_id_ixp = WORK_I1P_FDONE;
		break;
	default:
		mserr_hw("[F:%d]invalid hw(%d)", instance, hw_ip, frame->fcount, hw_ip->id);
		return -EINVAL;
		break;
	}

	output_id = ENTRY_IXC;
	if (test_bit(output_id, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame, wq_id_ixc,
				output_id, done_type, flag_get_meta);
		flag_get_meta = false;
	}

	output_id = ENTRY_IXP;
	if (test_bit(output_id, &frame->out_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame, wq_id_ixp,
				output_id, done_type, flag_get_meta);
		flag_get_meta = false;
	}

	output_id = FIMC_IS_HW_CORE_END;
	if (test_bit(hw_ip->id, &frame->core_flag)) {
		ret = fimc_is_hardware_frame_done(hw_ip, frame, -1,
				output_id, done_type, flag_get_meta);
		flag_get_meta = false;
	}

	head = GET_HEAD_GROUP_IN_DEVICE(FIMC_IS_DEVICE_ISCHAIN, hw_ip->group[instance]);
	if (!test_bit(FIMC_IS_GROUP_OTF_INPUT, &head->state))
		up(&hw_ip->smp_resource);

	return ret;
}

static int fimc_is_hw_isp_load_setfile(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	int flag = 0, ret = 0;
	ulong addr;
	u32 size, index;
	struct fimc_is_hw_isp *hw_isp = NULL;
	struct fimc_is_hw_ip_setfile *setfile;
	enum exynos_sensor_position sensor_position;

	BUG_ON(!hw_ip);

	if (test_bit(DEV_HW_3AA0, &hw_map) || test_bit(DEV_HW_3AA1, &hw_map))
		return 0;

	if (!test_bit_variables(hw_ip->id, &hw_map)) {
		msdbg_hw(2, "%s: hw_map(0x%lx)\n", instance, hw_ip, __func__, hw_map);
		return 0;
	}

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -ESRCH;
	}

	sensor_position = hw_ip->hardware->sensor_position[instance];
	setfile = &hw_ip->setfile[sensor_position];

	switch (setfile->version) {
	case SETFILE_V2:
		flag = false;
		break;
	case SETFILE_V3:
		flag = true;
		break;
	default:
		mserr_hw("invalid version (%d)", instance, hw_ip,
			setfile->version);
		return -EINVAL;
		break;
	}

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;

	for (index = 0; index < setfile->using_count; index++) {
		addr = setfile->table[index].addr;
		size = setfile->table[index].size;
		ret = fimc_is_lib_isp_create_tune_set(&hw_isp->lib[instance],
			addr, size, index, flag, instance);

		set_bit(index, &hw_isp->lib[instance].tune_count);
	}

	set_bit(HW_TUNESET, &hw_ip->state);

	return ret;
}

static int fimc_is_hw_isp_apply_setfile(struct fimc_is_hw_ip *hw_ip, u32 scenario,
	u32 instance, ulong hw_map)
{
	int ret = 0;
	u32 setfile_index = 0;
	struct fimc_is_hw_isp *hw_isp = NULL;
	struct fimc_is_hw_ip_setfile *setfile;
	enum exynos_sensor_position sensor_position;

	BUG_ON(!hw_ip);

	if (test_bit(DEV_HW_3AA0, &hw_map) || test_bit(DEV_HW_3AA1, &hw_map))
		return 0;

	if (!test_bit_variables(hw_ip->id, &hw_map)) {
		msdbg_hw(2, "%s: hw_map(0x%lx)\n", instance, hw_ip, __func__, hw_map);
		return 0;
	}

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		mserr_hw("not initialized!!", instance, hw_ip);
		return -ESRCH;
	}

	sensor_position = hw_ip->hardware->sensor_position[instance];
	setfile = &hw_ip->setfile[sensor_position];

	if (setfile->using_count == 0)
		return 0;

	setfile_index = setfile->index[scenario];
	if (setfile_index >= setfile->using_count) {
		mserr_hw("setfile index is out-of-range, [%d:%d]",
				instance, hw_ip, scenario, setfile_index);
		return -EINVAL;
	}

	msinfo_hw("setfile (%d) scenario (%d)\n", instance, hw_ip,
		setfile_index, scenario);

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;

	ret = fimc_is_lib_isp_apply_tune_set(&hw_isp->lib[instance], setfile_index, instance);

	return ret;
}

static int fimc_is_hw_isp_delete_setfile(struct fimc_is_hw_ip *hw_ip, u32 instance, ulong hw_map)
{
	struct fimc_is_hw_isp *hw_isp = NULL;
	int i, ret = 0;
	struct fimc_is_hw_ip_setfile *setfile;
	enum exynos_sensor_position sensor_position;

	BUG_ON(!hw_ip);

	if (test_bit(DEV_HW_3AA0, &hw_map) || test_bit(DEV_HW_3AA1, &hw_map))
		return 0;

	if (!test_bit_variables(hw_ip->id, &hw_map)) {
		msdbg_hw(2, "%s: hw_map(0x%lx)\n", instance, hw_ip, __func__, hw_map);
		return 0;
	}

	if (!test_bit(HW_INIT, &hw_ip->state)) {
		msdbg_hw(2, "Not initialized\n", instance, hw_ip);
		return 0;
	}

	sensor_position = hw_ip->hardware->sensor_position[instance];
	setfile = &hw_ip->setfile[sensor_position];

	if (setfile->using_count == 0)
		return 0;

	BUG_ON(!hw_ip->priv_info);
	hw_isp = (struct fimc_is_hw_isp *)hw_ip->priv_info;

	for (i = 0; i < setfile->using_count; i++) {
		if (test_bit(i, &hw_isp->lib[instance].tune_count)) {
			ret = fimc_is_lib_isp_delete_tune_set(&hw_isp->lib[instance],
				(u32)i, instance);
			clear_bit(i, &hw_isp->lib[instance].tune_count);
		}
	}

	clear_bit(HW_TUNESET, &hw_ip->state);

	return ret;
}

const struct fimc_is_hw_ip_ops fimc_is_hw_isp_ops = {
	.open			= fimc_is_hw_isp_open,
	.init			= fimc_is_hw_isp_init,
	.deinit			= fimc_is_hw_isp_deinit,
	.close			= fimc_is_hw_isp_close,
	.enable			= fimc_is_hw_isp_enable,
	.disable		= fimc_is_hw_isp_disable,
	.shot			= fimc_is_hw_isp_shot,
	.set_param		= fimc_is_hw_isp_set_param,
	.get_meta		= fimc_is_hw_isp_get_meta,
	.frame_ndone		= fimc_is_hw_isp_frame_ndone,
	.load_setfile		= fimc_is_hw_isp_load_setfile,
	.apply_setfile		= fimc_is_hw_isp_apply_setfile,
	.delete_setfile		= fimc_is_hw_isp_delete_setfile,
	.clk_gate		= fimc_is_hardware_clk_gate
};

int fimc_is_hw_isp_probe(struct fimc_is_hw_ip *hw_ip, struct fimc_is_interface *itf,
	struct fimc_is_interface_ischain *itfc, int id, const char *name)
{
	int ret = 0;

	BUG_ON(!hw_ip);
	BUG_ON(!itf);
	BUG_ON(!itfc);

	/* initialize device hardware */
	hw_ip->id   = id;
	snprintf(hw_ip->name, sizeof(hw_ip->name), "%s", name);
	hw_ip->ops  = &fimc_is_hw_isp_ops;
	hw_ip->itf  = itf;
	hw_ip->itfc = itfc;
	atomic_set(&hw_ip->fcount, 0);
	hw_ip->internal_fcount = 0;
	hw_ip->is_leader = true;
	atomic_set(&hw_ip->status.Vvalid, V_BLANK);
	atomic_set(&hw_ip->rsccount, 0);
	init_waitqueue_head(&hw_ip->status.wait_queue);

	clear_bit(HW_OPEN, &hw_ip->state);
	clear_bit(HW_INIT, &hw_ip->state);
	clear_bit(HW_CONFIG, &hw_ip->state);
	clear_bit(HW_RUN, &hw_ip->state);
	clear_bit(HW_TUNESET, &hw_ip->state);

	sinfo_hw("probe done\n", hw_ip);

	return ret;
}
