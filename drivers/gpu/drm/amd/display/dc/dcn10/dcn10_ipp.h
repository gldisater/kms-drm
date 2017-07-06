/*
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#ifndef _DCN10_IPP_H_
#define _DCN10_IPP_H_

#include "ipp.h"

#define TO_DCN10_IPP(ipp)\
	container_of(ipp, struct dcn10_ipp, base)

#define IPP_REG_LIST_DCN(id) \
	SRI(CM_ICSC_CONTROL, CM, id), \
	SRI(CM_ICSC_C11_C12, CM, id), \
	SRI(CM_ICSC_C13_C14, CM, id), \
	SRI(CM_ICSC_C21_C22, CM, id), \
	SRI(CM_ICSC_C23_C24, CM, id), \
	SRI(CM_ICSC_C31_C32, CM, id), \
	SRI(CM_ICSC_C33_C34, CM, id), \
	SRI(CM_DGAM_RAMB_START_CNTL_B, CM, id), \
	SRI(CM_DGAM_RAMB_START_CNTL_G, CM, id), \
	SRI(CM_DGAM_RAMB_START_CNTL_R, CM, id), \
	SRI(CM_DGAM_RAMB_SLOPE_CNTL_B, CM, id), \
	SRI(CM_DGAM_RAMB_SLOPE_CNTL_G, CM, id), \
	SRI(CM_DGAM_RAMB_SLOPE_CNTL_R, CM, id), \
	SRI(CM_DGAM_RAMB_END_CNTL1_B, CM, id), \
	SRI(CM_DGAM_RAMB_END_CNTL2_B, CM, id), \
	SRI(CM_DGAM_RAMB_END_CNTL1_G, CM, id), \
	SRI(CM_DGAM_RAMB_END_CNTL2_G, CM, id), \
	SRI(CM_DGAM_RAMB_END_CNTL1_R, CM, id), \
	SRI(CM_DGAM_RAMB_END_CNTL2_R, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_0_1, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_2_3, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_4_5, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_6_7, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_8_9, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_10_11, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_12_13, CM, id), \
	SRI(CM_DGAM_RAMB_REGION_14_15, CM, id), \
	SRI(CM_DGAM_RAMA_START_CNTL_B, CM, id), \
	SRI(CM_DGAM_RAMA_START_CNTL_G, CM, id), \
	SRI(CM_DGAM_RAMA_START_CNTL_R, CM, id), \
	SRI(CM_DGAM_RAMA_SLOPE_CNTL_B, CM, id), \
	SRI(CM_DGAM_RAMA_SLOPE_CNTL_G, CM, id), \
	SRI(CM_DGAM_RAMA_SLOPE_CNTL_R, CM, id), \
	SRI(CM_DGAM_RAMA_END_CNTL1_B, CM, id), \
	SRI(CM_DGAM_RAMA_END_CNTL2_B, CM, id), \
	SRI(CM_DGAM_RAMA_END_CNTL1_G, CM, id), \
	SRI(CM_DGAM_RAMA_END_CNTL2_G, CM, id), \
	SRI(CM_DGAM_RAMA_END_CNTL1_R, CM, id), \
	SRI(CM_DGAM_RAMA_END_CNTL2_R, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_0_1, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_2_3, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_4_5, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_6_7, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_8_9, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_10_11, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_12_13, CM, id), \
	SRI(CM_DGAM_RAMA_REGION_14_15, CM, id), \
	SRI(CM_MEM_PWR_CTRL, CM, id), \
	SRI(CM_DGAM_LUT_WRITE_EN_MASK, CM, id), \
	SRI(CM_DGAM_LUT_INDEX, CM, id), \
	SRI(CM_DGAM_LUT_DATA, CM, id), \
	SRI(CM_CONTROL, CM, id), \
	SRI(CM_DGAM_CONTROL, CM, id), \
	SRI(FORMAT_CONTROL, CNVC_CFG, id), \
	SRI(DPP_CONTROL, DPP_TOP, id), \
	SRI(CURSOR_SETTINS, HUBPREQ, id), \
	SRI(CNVC_SURFACE_PIXEL_FORMAT, CNVC_CFG, id), \
	SRI(CURSOR0_CONTROL, CNVC_CUR, id), \
	SRI(CURSOR0_COLOR0, CNVC_CUR, id), \
	SRI(CURSOR0_COLOR1, CNVC_CUR, id)

#define IPP_REG_LIST_DCN10(id) \
	IPP_REG_LIST_DCN(id), \
	SRI(CM_IGAM_CONTROL, CM, id), \
	SRI(CM_COMA_C11_C12, CM, id), \
	SRI(CM_COMA_C13_C14, CM, id), \
	SRI(CM_COMA_C21_C22, CM, id), \
	SRI(CM_COMA_C23_C24, CM, id), \
	SRI(CM_COMA_C31_C32, CM, id), \
	SRI(CM_COMA_C33_C34, CM, id), \
	SRI(CM_IGAM_LUT_RW_CONTROL, CM, id), \
	SRI(CM_IGAM_LUT_RW_INDEX, CM, id), \
	SRI(CM_IGAM_LUT_SEQ_COLOR, CM, id), \
	SRI(CURSOR_SURFACE_ADDRESS_HIGH, CURSOR, id), \
	SRI(CURSOR_SURFACE_ADDRESS, CURSOR, id), \
	SRI(CURSOR_SIZE, CURSOR, id), \
	SRI(CURSOR_CONTROL, CURSOR, id), \
	SRI(CURSOR_POSITION, CURSOR, id), \
	SRI(CURSOR_HOT_SPOT, CURSOR, id), \
	SRI(CURSOR_DST_OFFSET, CURSOR, id)

#define IPP_SF(reg_name, field_name, post_fix)\
	.field_name = reg_name ## __ ## field_name ## post_fix

#define IPP_MASK_SH_LIST_DCN(mask_sh) \
	IPP_SF(CM0_CM_ICSC_CONTROL, CM_ICSC_MODE, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C11_C12, CM_ICSC_C11, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C11_C12, CM_ICSC_C12, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C13_C14, CM_ICSC_C13, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C13_C14, CM_ICSC_C14, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C21_C22, CM_ICSC_C21, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C21_C22, CM_ICSC_C22, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C23_C24, CM_ICSC_C23, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C23_C24, CM_ICSC_C24, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C31_C32, CM_ICSC_C31, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C31_C32, CM_ICSC_C32, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C33_C34, CM_ICSC_C33, mask_sh), \
	IPP_SF(CM0_CM_ICSC_C33_C34, CM_ICSC_C34, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_START_CNTL_B, CM_DGAM_RAMB_EXP_REGION_START_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_START_CNTL_B, CM_DGAM_RAMB_EXP_REGION_START_SEGMENT_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_START_CNTL_G, CM_DGAM_RAMB_EXP_REGION_START_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_START_CNTL_G, CM_DGAM_RAMB_EXP_REGION_START_SEGMENT_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_START_CNTL_R, CM_DGAM_RAMB_EXP_REGION_START_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_START_CNTL_R, CM_DGAM_RAMB_EXP_REGION_START_SEGMENT_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_SLOPE_CNTL_B, CM_DGAM_RAMB_EXP_REGION_LINEAR_SLOPE_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_SLOPE_CNTL_G, CM_DGAM_RAMB_EXP_REGION_LINEAR_SLOPE_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_SLOPE_CNTL_R, CM_DGAM_RAMB_EXP_REGION_LINEAR_SLOPE_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL1_B, CM_DGAM_RAMB_EXP_REGION_END_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL2_B, CM_DGAM_RAMB_EXP_REGION_END_SLOPE_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL2_B, CM_DGAM_RAMB_EXP_REGION_END_BASE_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL1_G, CM_DGAM_RAMB_EXP_REGION_END_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL2_G, CM_DGAM_RAMB_EXP_REGION_END_SLOPE_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL2_G, CM_DGAM_RAMB_EXP_REGION_END_BASE_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL1_R, CM_DGAM_RAMB_EXP_REGION_END_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL2_R, CM_DGAM_RAMB_EXP_REGION_END_SLOPE_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_END_CNTL2_R, CM_DGAM_RAMB_EXP_REGION_END_BASE_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_0_1, CM_DGAM_RAMB_EXP_REGION0_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_0_1, CM_DGAM_RAMB_EXP_REGION0_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_0_1, CM_DGAM_RAMB_EXP_REGION1_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_0_1, CM_DGAM_RAMB_EXP_REGION1_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_2_3, CM_DGAM_RAMB_EXP_REGION2_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_2_3, CM_DGAM_RAMB_EXP_REGION2_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_2_3, CM_DGAM_RAMB_EXP_REGION3_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_2_3, CM_DGAM_RAMB_EXP_REGION3_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_4_5, CM_DGAM_RAMB_EXP_REGION4_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_4_5, CM_DGAM_RAMB_EXP_REGION4_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_4_5, CM_DGAM_RAMB_EXP_REGION5_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_4_5, CM_DGAM_RAMB_EXP_REGION5_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_6_7, CM_DGAM_RAMB_EXP_REGION6_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_6_7, CM_DGAM_RAMB_EXP_REGION6_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_6_7, CM_DGAM_RAMB_EXP_REGION7_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_6_7, CM_DGAM_RAMB_EXP_REGION7_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_8_9, CM_DGAM_RAMB_EXP_REGION8_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_8_9, CM_DGAM_RAMB_EXP_REGION8_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_8_9, CM_DGAM_RAMB_EXP_REGION9_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_8_9, CM_DGAM_RAMB_EXP_REGION9_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_10_11, CM_DGAM_RAMB_EXP_REGION10_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_10_11, CM_DGAM_RAMB_EXP_REGION10_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_10_11, CM_DGAM_RAMB_EXP_REGION11_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_10_11, CM_DGAM_RAMB_EXP_REGION11_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_12_13, CM_DGAM_RAMB_EXP_REGION12_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_12_13, CM_DGAM_RAMB_EXP_REGION12_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_12_13, CM_DGAM_RAMB_EXP_REGION13_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_12_13, CM_DGAM_RAMB_EXP_REGION13_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_14_15, CM_DGAM_RAMB_EXP_REGION14_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_14_15, CM_DGAM_RAMB_EXP_REGION14_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_14_15, CM_DGAM_RAMB_EXP_REGION15_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMB_REGION_14_15, CM_DGAM_RAMB_EXP_REGION15_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_START_CNTL_B, CM_DGAM_RAMA_EXP_REGION_START_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_START_CNTL_B, CM_DGAM_RAMA_EXP_REGION_START_SEGMENT_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_START_CNTL_G, CM_DGAM_RAMA_EXP_REGION_START_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_START_CNTL_G, CM_DGAM_RAMA_EXP_REGION_START_SEGMENT_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_START_CNTL_R, CM_DGAM_RAMA_EXP_REGION_START_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_START_CNTL_R, CM_DGAM_RAMA_EXP_REGION_START_SEGMENT_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_SLOPE_CNTL_B, CM_DGAM_RAMA_EXP_REGION_LINEAR_SLOPE_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_SLOPE_CNTL_G, CM_DGAM_RAMA_EXP_REGION_LINEAR_SLOPE_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_SLOPE_CNTL_R, CM_DGAM_RAMA_EXP_REGION_LINEAR_SLOPE_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL1_B, CM_DGAM_RAMA_EXP_REGION_END_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL2_B, CM_DGAM_RAMA_EXP_REGION_END_SLOPE_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL2_B, CM_DGAM_RAMA_EXP_REGION_END_BASE_B, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL1_G, CM_DGAM_RAMA_EXP_REGION_END_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL2_G, CM_DGAM_RAMA_EXP_REGION_END_SLOPE_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL2_G, CM_DGAM_RAMA_EXP_REGION_END_BASE_G, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL1_R, CM_DGAM_RAMA_EXP_REGION_END_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL2_R, CM_DGAM_RAMA_EXP_REGION_END_SLOPE_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_END_CNTL2_R, CM_DGAM_RAMA_EXP_REGION_END_BASE_R, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_0_1, CM_DGAM_RAMA_EXP_REGION0_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_0_1, CM_DGAM_RAMA_EXP_REGION0_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_0_1, CM_DGAM_RAMA_EXP_REGION1_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_0_1, CM_DGAM_RAMA_EXP_REGION1_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_2_3, CM_DGAM_RAMA_EXP_REGION2_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_2_3, CM_DGAM_RAMA_EXP_REGION2_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_2_3, CM_DGAM_RAMA_EXP_REGION3_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_2_3, CM_DGAM_RAMA_EXP_REGION3_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_4_5, CM_DGAM_RAMA_EXP_REGION4_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_4_5, CM_DGAM_RAMA_EXP_REGION4_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_4_5, CM_DGAM_RAMA_EXP_REGION5_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_4_5, CM_DGAM_RAMA_EXP_REGION5_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_6_7, CM_DGAM_RAMA_EXP_REGION6_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_6_7, CM_DGAM_RAMA_EXP_REGION6_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_6_7, CM_DGAM_RAMA_EXP_REGION7_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_6_7, CM_DGAM_RAMA_EXP_REGION7_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_8_9, CM_DGAM_RAMA_EXP_REGION8_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_8_9, CM_DGAM_RAMA_EXP_REGION8_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_8_9, CM_DGAM_RAMA_EXP_REGION9_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_8_9, CM_DGAM_RAMA_EXP_REGION9_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_10_11, CM_DGAM_RAMA_EXP_REGION10_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_10_11, CM_DGAM_RAMA_EXP_REGION10_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_10_11, CM_DGAM_RAMA_EXP_REGION11_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_10_11, CM_DGAM_RAMA_EXP_REGION11_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_12_13, CM_DGAM_RAMA_EXP_REGION12_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_12_13, CM_DGAM_RAMA_EXP_REGION12_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_12_13, CM_DGAM_RAMA_EXP_REGION13_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_12_13, CM_DGAM_RAMA_EXP_REGION13_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_14_15, CM_DGAM_RAMA_EXP_REGION14_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_14_15, CM_DGAM_RAMA_EXP_REGION14_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_14_15, CM_DGAM_RAMA_EXP_REGION15_LUT_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_DGAM_RAMA_REGION_14_15, CM_DGAM_RAMA_EXP_REGION15_NUM_SEGMENTS, mask_sh), \
	IPP_SF(CM0_CM_MEM_PWR_CTRL, SHARED_MEM_PWR_DIS, mask_sh), \
	IPP_SF(CM0_CM_DGAM_LUT_WRITE_EN_MASK, CM_DGAM_LUT_WRITE_EN_MASK, mask_sh), \
	IPP_SF(CM0_CM_DGAM_LUT_WRITE_EN_MASK, CM_DGAM_LUT_WRITE_SEL, mask_sh), \
	IPP_SF(CM0_CM_DGAM_LUT_INDEX, CM_DGAM_LUT_INDEX, mask_sh), \
	IPP_SF(CM0_CM_DGAM_LUT_DATA, CM_DGAM_LUT_DATA, mask_sh), \
	IPP_SF(DPP_TOP0_DPP_CONTROL, DPP_CLOCK_ENABLE, mask_sh), \
	IPP_SF(CNVC_CFG0_CNVC_SURFACE_PIXEL_FORMAT, CNVC_SURFACE_PIXEL_FORMAT, mask_sh), \
	IPP_SF(CNVC_CFG0_FORMAT_CONTROL, CNVC_BYPASS, mask_sh), \
	IPP_SF(CNVC_CFG0_FORMAT_CONTROL, ALPHA_EN, mask_sh), \
	IPP_SF(CNVC_CFG0_FORMAT_CONTROL, FORMAT_EXPANSION_MODE, mask_sh), \
	IPP_SF(CM0_CM_DGAM_CONTROL, CM_DGAM_LUT_MODE, mask_sh), \
	IPP_SF(HUBPREQ0_CURSOR_SETTINS, CURSOR0_DST_Y_OFFSET, mask_sh), \
	IPP_SF(HUBPREQ0_CURSOR_SETTINS, CURSOR0_CHUNK_HDL_ADJUST, mask_sh), \
	IPP_SF(CNVC_CUR0_CURSOR0_CONTROL, CUR0_MODE, mask_sh), \
	IPP_SF(CNVC_CUR0_CURSOR0_COLOR0, CUR0_COLOR0, mask_sh), \
	IPP_SF(CNVC_CUR0_CURSOR0_COLOR1, CUR0_COLOR1, mask_sh), \
	IPP_SF(CNVC_CUR0_CURSOR0_CONTROL, CUR0_EXPANSION_MODE, mask_sh), \
	IPP_SF(CNVC_CUR0_CURSOR0_CONTROL, CUR0_ENABLE, mask_sh)

#define IPP_MASK_SH_LIST_DCN10(mask_sh) \
	IPP_MASK_SH_LIST_DCN(mask_sh),\
	IPP_SF(CM0_CM_COMA_C11_C12, CM_COMA_C11, mask_sh), \
	IPP_SF(CM0_CM_COMA_C11_C12, CM_COMA_C12, mask_sh), \
	IPP_SF(CM0_CM_COMA_C13_C14, CM_COMA_C13, mask_sh), \
	IPP_SF(CM0_CM_COMA_C13_C14, CM_COMA_C14, mask_sh), \
	IPP_SF(CM0_CM_COMA_C21_C22, CM_COMA_C21, mask_sh), \
	IPP_SF(CM0_CM_COMA_C21_C22, CM_COMA_C22, mask_sh), \
	IPP_SF(CM0_CM_COMA_C23_C24, CM_COMA_C23, mask_sh), \
	IPP_SF(CM0_CM_COMA_C23_C24, CM_COMA_C24, mask_sh), \
	IPP_SF(CM0_CM_COMA_C31_C32, CM_COMA_C31, mask_sh), \
	IPP_SF(CM0_CM_COMA_C31_C32, CM_COMA_C32, mask_sh), \
	IPP_SF(CM0_CM_COMA_C33_C34, CM_COMA_C33, mask_sh), \
	IPP_SF(CM0_CM_COMA_C33_C34, CM_COMA_C34, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_RW_CONTROL, CM_IGAM_DGAM_CONFIG_STATUS, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_RW_CONTROL, CM_IGAM_LUT_HOST_EN, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_RW_CONTROL, CM_IGAM_LUT_RW_MODE, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_RW_CONTROL, CM_IGAM_LUT_SEL, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_RW_CONTROL, CM_IGAM_LUT_WRITE_EN_MASK, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_RW_INDEX, CM_IGAM_LUT_RW_INDEX, mask_sh), \
	IPP_SF(CM0_CM_IGAM_LUT_SEQ_COLOR, CM_IGAM_LUT_SEQ_COLOR, mask_sh), \
	IPP_SF(CM0_CM_CONTROL, CM_BYPASS_EN, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_SURFACE_ADDRESS_HIGH, CURSOR_SURFACE_ADDRESS_HIGH, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_SURFACE_ADDRESS, CURSOR_SURFACE_ADDRESS, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_SIZE, CURSOR_WIDTH, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_SIZE, CURSOR_HEIGHT, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_CONTROL, CURSOR_MODE, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_CONTROL, CURSOR_PITCH, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_CONTROL, CURSOR_LINES_PER_CHUNK, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_CONTROL, CURSOR_ENABLE, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_POSITION, CURSOR_X_POSITION, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_POSITION, CURSOR_Y_POSITION, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_HOT_SPOT, CURSOR_HOT_SPOT_X, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_HOT_SPOT, CURSOR_HOT_SPOT_Y, mask_sh), \
	IPP_SF(CURSOR0_CURSOR_DST_OFFSET, CURSOR_DST_X_OFFSET, mask_sh), \
	IPP_SF(CM0_CM_IGAM_CONTROL, CM_IGAM_LUT_MODE, mask_sh), \
	IPP_SF(CM0_CM_IGAM_CONTROL, CM_IGAM_LUT_FORMAT_R, mask_sh), \
	IPP_SF(CM0_CM_IGAM_CONTROL, CM_IGAM_LUT_FORMAT_G, mask_sh), \
	IPP_SF(CM0_CM_IGAM_CONTROL, CM_IGAM_LUT_FORMAT_B, mask_sh), \
	IPP_SF(CM0_CM_IGAM_CONTROL, CM_IGAM_INPUT_FORMAT, mask_sh), \
	IPP_SF(CNVC_CFG0_FORMAT_CONTROL, OUTPUT_FP, mask_sh)

#define IPP_DCN10_REG_FIELD_LIST(type) \
	type CM_DGAM_CONFIG_STATUS; \
	type CM_ICSC_MODE; \
	type CM_ICSC_C11; \
	type CM_ICSC_C12; \
	type CM_ICSC_C13; \
	type CM_ICSC_C14; \
	type CM_ICSC_C21; \
	type CM_ICSC_C22; \
	type CM_ICSC_C23; \
	type CM_ICSC_C24; \
	type CM_ICSC_C31; \
	type CM_ICSC_C32; \
	type CM_ICSC_C33; \
	type CM_ICSC_C34; \
	type CM_COMA_C11; \
	type CM_COMA_C12; \
	type CM_COMA_C13; \
	type CM_COMA_C14; \
	type CM_COMA_C21; \
	type CM_COMA_C22; \
	type CM_COMA_C23; \
	type CM_COMA_C24; \
	type CM_COMA_C31; \
	type CM_COMA_C32; \
	type CM_COMA_C33; \
	type CM_COMA_C34; \
	type CM_DGAM_RAMB_EXP_REGION_START_B; \
	type CM_DGAM_RAMB_EXP_REGION_START_SEGMENT_B; \
	type CM_DGAM_RAMB_EXP_REGION_START_G; \
	type CM_DGAM_RAMB_EXP_REGION_START_SEGMENT_G; \
	type CM_DGAM_RAMB_EXP_REGION_START_R; \
	type CM_DGAM_RAMB_EXP_REGION_START_SEGMENT_R; \
	type CM_DGAM_RAMB_EXP_REGION_LINEAR_SLOPE_B; \
	type CM_DGAM_RAMB_EXP_REGION_LINEAR_SLOPE_G; \
	type CM_DGAM_RAMB_EXP_REGION_LINEAR_SLOPE_R; \
	type CM_DGAM_RAMB_EXP_REGION_END_B; \
	type CM_DGAM_RAMB_EXP_REGION_END_SLOPE_B; \
	type CM_DGAM_RAMB_EXP_REGION_END_BASE_B; \
	type CM_DGAM_RAMB_EXP_REGION_END_G; \
	type CM_DGAM_RAMB_EXP_REGION_END_SLOPE_G; \
	type CM_DGAM_RAMB_EXP_REGION_END_BASE_G; \
	type CM_DGAM_RAMB_EXP_REGION_END_R; \
	type CM_DGAM_RAMB_EXP_REGION_END_SLOPE_R; \
	type CM_DGAM_RAMB_EXP_REGION_END_BASE_R; \
	type CM_DGAM_RAMB_EXP_REGION0_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION0_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION1_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION1_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION2_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION2_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION3_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION3_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION4_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION4_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION5_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION5_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION6_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION6_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION7_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION7_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION8_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION8_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION9_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION9_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION10_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION10_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION11_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION11_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION12_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION12_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION13_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION13_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION14_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION14_NUM_SEGMENTS; \
	type CM_DGAM_RAMB_EXP_REGION15_LUT_OFFSET; \
	type CM_DGAM_RAMB_EXP_REGION15_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION_START_B; \
	type CM_DGAM_RAMA_EXP_REGION_START_SEGMENT_B; \
	type CM_DGAM_RAMA_EXP_REGION_START_G; \
	type CM_DGAM_RAMA_EXP_REGION_START_SEGMENT_G; \
	type CM_DGAM_RAMA_EXP_REGION_START_R; \
	type CM_DGAM_RAMA_EXP_REGION_START_SEGMENT_R; \
	type CM_DGAM_RAMA_EXP_REGION_LINEAR_SLOPE_B; \
	type CM_DGAM_RAMA_EXP_REGION_LINEAR_SLOPE_G; \
	type CM_DGAM_RAMA_EXP_REGION_LINEAR_SLOPE_R; \
	type CM_DGAM_RAMA_EXP_REGION_END_B; \
	type CM_DGAM_RAMA_EXP_REGION_END_SLOPE_B; \
	type CM_DGAM_RAMA_EXP_REGION_END_BASE_B; \
	type CM_DGAM_RAMA_EXP_REGION_END_G; \
	type CM_DGAM_RAMA_EXP_REGION_END_SLOPE_G; \
	type CM_DGAM_RAMA_EXP_REGION_END_BASE_G; \
	type CM_DGAM_RAMA_EXP_REGION_END_R; \
	type CM_DGAM_RAMA_EXP_REGION_END_SLOPE_R; \
	type CM_DGAM_RAMA_EXP_REGION_END_BASE_R; \
	type CM_DGAM_RAMA_EXP_REGION0_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION0_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION1_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION1_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION2_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION2_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION3_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION3_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION4_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION4_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION5_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION5_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION6_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION6_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION7_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION7_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION8_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION8_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION9_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION9_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION10_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION10_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION11_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION11_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION12_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION12_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION13_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION13_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION14_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION14_NUM_SEGMENTS; \
	type CM_DGAM_RAMA_EXP_REGION15_LUT_OFFSET; \
	type CM_DGAM_RAMA_EXP_REGION15_NUM_SEGMENTS; \
	type SHARED_MEM_PWR_DIS; \
	type CM_IGAM_LUT_FORMAT_R; \
	type CM_IGAM_LUT_FORMAT_G; \
	type CM_IGAM_LUT_FORMAT_B; \
	type CM_IGAM_LUT_HOST_EN; \
	type CM_IGAM_LUT_RW_INDEX; \
	type CM_IGAM_LUT_RW_MODE; \
	type CM_IGAM_LUT_WRITE_EN_MASK; \
	type CM_IGAM_LUT_SEL; \
	type CM_IGAM_LUT_SEQ_COLOR; \
	type CM_IGAM_DGAM_CONFIG_STATUS; \
	type CM_DGAM_LUT_WRITE_EN_MASK; \
	type CM_DGAM_LUT_WRITE_SEL; \
	type CM_DGAM_LUT_INDEX; \
	type CM_DGAM_LUT_DATA; \
	type DPP_CLOCK_ENABLE; \
	type CM_BYPASS_EN; \
	type CM_BYPASS; \
	type CNVC_SURFACE_PIXEL_FORMAT; \
	type CNVC_BYPASS; \
	type ALPHA_EN; \
	type FORMAT_EXPANSION_MODE; \
	type CM_DGAM_LUT_MODE; \
	type CM_IGAM_LUT_MODE; \
	type CURSOR0_DST_Y_OFFSET; \
	type CURSOR0_CHUNK_HDL_ADJUST; \
	type CUR0_MODE; \
	type CUR0_COLOR0; \
	type CUR0_COLOR1; \
	type CUR0_EXPANSION_MODE; \
	type CURSOR_SURFACE_ADDRESS_HIGH; \
	type CURSOR_SURFACE_ADDRESS; \
	type CURSOR_WIDTH; \
	type CURSOR_HEIGHT; \
	type CURSOR_MODE; \
	type CURSOR_PITCH; \
	type CURSOR_LINES_PER_CHUNK; \
	type CURSOR_ENABLE; \
	type CUR0_ENABLE; \
	type CURSOR_X_POSITION; \
	type CURSOR_Y_POSITION; \
	type CURSOR_HOT_SPOT_X; \
	type CURSOR_HOT_SPOT_Y; \
	type CURSOR_DST_X_OFFSET; \
	type CM_IGAM_INPUT_FORMAT; \
	type OUTPUT_FP

struct dcn10_ipp_shift {
	IPP_DCN10_REG_FIELD_LIST(uint8_t);
};

struct dcn10_ipp_mask {
	IPP_DCN10_REG_FIELD_LIST(uint32_t);
};

struct dcn10_ipp_registers {
	uint32_t CM_ICSC_CONTROL;
	uint32_t CM_ICSC_C11_C12;
	uint32_t CM_ICSC_C13_C14;
	uint32_t CM_ICSC_C21_C22;
	uint32_t CM_ICSC_C23_C24;
	uint32_t CM_ICSC_C31_C32;
	uint32_t CM_ICSC_C33_C34;
	uint32_t CM_COMA_C11_C12;
	uint32_t CM_COMA_C13_C14;
	uint32_t CM_COMA_C21_C22;
	uint32_t CM_COMA_C23_C24;
	uint32_t CM_COMA_C31_C32;
	uint32_t CM_COMA_C33_C34;
	uint32_t CM_DGAM_RAMB_START_CNTL_B;
	uint32_t CM_DGAM_RAMB_START_CNTL_G;
	uint32_t CM_DGAM_RAMB_START_CNTL_R;
	uint32_t CM_DGAM_RAMB_SLOPE_CNTL_B;
	uint32_t CM_DGAM_RAMB_SLOPE_CNTL_G;
	uint32_t CM_DGAM_RAMB_SLOPE_CNTL_R;
	uint32_t CM_DGAM_RAMB_END_CNTL1_B;
	uint32_t CM_DGAM_RAMB_END_CNTL2_B;
	uint32_t CM_DGAM_RAMB_END_CNTL1_G;
	uint32_t CM_DGAM_RAMB_END_CNTL2_G;
	uint32_t CM_DGAM_RAMB_END_CNTL1_R;
	uint32_t CM_DGAM_RAMB_END_CNTL2_R;
	uint32_t CM_DGAM_RAMB_REGION_0_1;
	uint32_t CM_DGAM_RAMB_REGION_2_3;
	uint32_t CM_DGAM_RAMB_REGION_4_5;
	uint32_t CM_DGAM_RAMB_REGION_6_7;
	uint32_t CM_DGAM_RAMB_REGION_8_9;
	uint32_t CM_DGAM_RAMB_REGION_10_11;
	uint32_t CM_DGAM_RAMB_REGION_12_13;
	uint32_t CM_DGAM_RAMB_REGION_14_15;
	uint32_t CM_DGAM_RAMA_START_CNTL_B;
	uint32_t CM_DGAM_RAMA_START_CNTL_G;
	uint32_t CM_DGAM_RAMA_START_CNTL_R;
	uint32_t CM_DGAM_RAMA_SLOPE_CNTL_B;
	uint32_t CM_DGAM_RAMA_SLOPE_CNTL_G;
	uint32_t CM_DGAM_RAMA_SLOPE_CNTL_R;
	uint32_t CM_DGAM_RAMA_END_CNTL1_B;
	uint32_t CM_DGAM_RAMA_END_CNTL2_B;
	uint32_t CM_DGAM_RAMA_END_CNTL1_G;
	uint32_t CM_DGAM_RAMA_END_CNTL2_G;
	uint32_t CM_DGAM_RAMA_END_CNTL1_R;
	uint32_t CM_DGAM_RAMA_END_CNTL2_R;
	uint32_t CM_DGAM_RAMA_REGION_0_1;
	uint32_t CM_DGAM_RAMA_REGION_2_3;
	uint32_t CM_DGAM_RAMA_REGION_4_5;
	uint32_t CM_DGAM_RAMA_REGION_6_7;
	uint32_t CM_DGAM_RAMA_REGION_8_9;
	uint32_t CM_DGAM_RAMA_REGION_10_11;
	uint32_t CM_DGAM_RAMA_REGION_12_13;
	uint32_t CM_DGAM_RAMA_REGION_14_15;
	uint32_t CM_MEM_PWR_CTRL;
	uint32_t CM_IGAM_LUT_RW_CONTROL;
	uint32_t CM_IGAM_LUT_RW_INDEX;
	uint32_t CM_IGAM_LUT_SEQ_COLOR;
	uint32_t CM_DGAM_LUT_WRITE_EN_MASK;
	uint32_t CM_DGAM_LUT_INDEX;
	uint32_t CM_DGAM_LUT_DATA;
	uint32_t CM_CONTROL;
	uint32_t CM_DGAM_CONTROL;
	uint32_t CM_IGAM_CONTROL;
	uint32_t DPP_CONTROL;
	uint32_t CURSOR_SETTINS;
	uint32_t CNVC_SURFACE_PIXEL_FORMAT;
	uint32_t CURSOR0_CONTROL;
	uint32_t CURSOR0_COLOR0;
	uint32_t CURSOR0_COLOR1;
	uint32_t FORMAT_CONTROL;
	uint32_t CURSOR_SURFACE_ADDRESS_HIGH;
	uint32_t CURSOR_SURFACE_ADDRESS;
	uint32_t CURSOR_SIZE;
	uint32_t CURSOR_CONTROL;
	uint32_t CURSOR_POSITION;
	uint32_t CURSOR_HOT_SPOT;
	uint32_t CURSOR_DST_OFFSET;
};

struct dcn10_ipp {
	struct input_pixel_processor base;

	const struct dcn10_ipp_registers *regs;
	const struct dcn10_ipp_shift *ipp_shift;
	const struct dcn10_ipp_mask *ipp_mask;

	struct dc_cursor_attributes curs_attr;
};

void dcn10_ipp_construct(struct dcn10_ipp *ippn10,
	struct dc_context *ctx,
	int inst,
	const struct dcn10_ipp_registers *regs,
	const struct dcn10_ipp_shift *ipp_shift,
	const struct dcn10_ipp_mask *ipp_mask);

#endif /* _DCN10_IPP_H_ */
