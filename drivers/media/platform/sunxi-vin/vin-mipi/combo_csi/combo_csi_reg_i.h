/*
 * combo csi module
 *
 * Copyright (c) 2019 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Authors:  Zheng Zequn <zequnzheng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __COMBO_CSI_REG_I_H__
#define __COMBO_CSI_REG_I_H__

/*
 * Detail information of registers----PHY
 */
#define CMB_PHY_TOP_REG_OFF			0x000
#define CMB_PHY_PWDNZ				0
#define CMB_PHY_PWDNZ_MASK			(0x1 << CMB_PHY_PWDNZ)
#define CMB_PHY_RSTN				1
#define CMB_PHY_RSTN_MASK			(0x1 << CMB_PHY_RSTN)
#define CMB_PHY_VREF_EN				2
#define CMB_PHY_VREF_EN_MASK			(0x1 << CMB_PHY_VREF_EN)
#define CMB_PHY_LVLDO_EN			3
#define CMB_PHY_LVLDO_EN_MASK			(0x1 << CMB_PHY_LVLDO_EN)
#define CMB_PHY_VREF_OP2			8
#define CMB_PHY_VREF_OP2_MASK			(0x3 << CMB_PHY_VREF_OP2)
#define CMB_PHY_VREF_OP9			10
#define CMB_PHY_VREF_OP9_MASK			(0x3 << CMB_PHY_VREF_OP9)
#define CMB_PRBS_SEL				24
#define CMB_PRBS_SEL_MASK			(0x3 << CMB_PRBS_SEL)

#define CMB_TRESCAL_REG_OFF			0x004
#define CMB_PHYA_TRESCAL_AUTO			0
#define CMB_PHYA_TRESCAL_AUTO_MASK		(0x1 << CMB_PHYA_TRESCAL_AUTO)
#define CMB_PHYA_TRESCAL_SOFT			1
#define CMB_PHYA_TRESCAL_SOFT_MASK		(0x1 << CMB_PHYA_TRESCAL_SOFT)
#define CMB_PHYA_TRESCAL_RESETN			2
#define CMB_PHYA_TRESCAL_RESETN_MASK		(0x1 << CMB_PHYA_TRESCAL_RESETN)
#define CMB_PHYA_TRESCAL_FLAG			3
#define CMB_PHYA_TRESCAL_FLAG_MASK		(0x1 << CMB_PHYA_TRESCAL_FLAG)
#define CMB_PHYA_TRESCAL_SET			4
#define CMB_PHYA_TRESCAL_SET_MASK		(0x1F << CMB_PHYA_TRESCAL_SET)
#define CMB_PHYA_TRESCAL_RESULT			16
#define CMB_PHYA_TRESCAL_RESULT_MASK		(0x1F << CMB_PHYA_TRESCAL_RESULT)

/*
 * Detail information of registers----PHYA/B
 */
#define CMB_PHY_REG_OFF				0x100
#define CMB_PHY_BETWEEN_OFF			0x100

#define CMB_PHY_CTL_REG_OFF			0x000
#define CMB_PHY0_EN				0
#define CMB_PHY0_EN_MASK			(0x1 << CMB_PHY0_EN)
#define CMB_PHY1_EN				1
#define CMB_PHY1_EN_MASK			(0x1 << CMB_PHY1_EN)
#define CMB_PHY_LINK_MODE			2
#define CMB_PHY_LINK_MODE_MASK			(0x1 << CMB_PHY_LINK_MODE)
#define CMB_PHY_LANEDT_EN			4
#define CMB_PHY_LANEDT_EN_MASK			(0xf << CMB_PHY_LANEDT_EN)
#define CMB_PHY_LANECK_EN			8
#define CMB_PHY_LANECK_EN_MASK			(0x3 << CMB_PHY_LANECK_EN)
#define CMB_PHY0_CK_SEL				10
#define CMB_PHY0_CK_SEL_MASK			(0x1 << CMB_PHY0_CK_SEL)
#define CMB_PHY1_CK_SEL				11
#define CMB_PHY1_CK_SEL_MASK			(0x1 << CMB_PHY1_CK_SEL)
#define CMB_PHY0_LP_PEFI			12
#define CMB_PHY0_LP_PEFI_MASK			(0x1 << CMB_PHY0_LP_PEFI)
#define CMB_PHY1_LP_PEFI			13
#define CMB_PHY1_LP_PEFI_MASK			(0x1 << CMB_PHY1_LP_PEFI)
#define CMB_PHY0_HS_PEFI			16
#define CMB_PHY0_HS_PEFI_MASK			(0x7 << CMB_PHY0_HS_PEFI)
#define CMB_PHY0_VCM				19
#define CMB_PHY0_VCM_MASK			(0x1 << CMB_PHY0_VCM)
#define CMB_PHY1_HS_PEFI			20
#define CMB_PHY1_HS_PEFI_MASK			(0x7 << CMB_PHY1_HS_PEFI)
#define CMB_PHY1_VCM				23
#define CMB_PHY1_VCM_MASK			(0x1 << CMB_PHY1_VCM)
#define CMB_PHY0_IBIAS_EN			26
#define CMB_PHY0_IBIAS_EN_MASK			(0x1 << CMB_PHY0_IBIAS_EN)
#define CMB_PHY1_IBIAS_EN			27
#define CMB_PHY1_IBIAS_EN_MASK			(0x1 << CMB_PHY1_IBIAS_EN)
#define CMB_PHY0_WORK_MODE			28
#define CMB_PHY0_WORK_MODE_MASK			(0x3 << CMB_PHY0_WORK_MODE)
#define CMB_PHY1_WORK_MODE			30
#define CMB_PHY1_WORK_MODE_MASK			(0x3 << CMB_PHY1_WORK_MODE)

#define CMB_PHY_EQ_REG_OFF			0x004
#define CMB_PHY_EQ_DT_EN			0
#define CMB_PHY_EQ_DT_EN_MASK			(0xF << CMB_PHY_EQ_DT_EN)
#define CMB_PHY_EQ_CK_EN			4
#define CMB_PHY_EQ_CK_EN_MASK			(0x3 << CMB_PHY_EQ_CK_EN)
#define CMB_PHY_EQ_LANED0			16
#define CMB_PHY_EQ_LANED0_MASK			(0x3 << CMB_PHY_EQ_LANED0)
#define CMB_PHY_EQ_LANED1			18
#define CMB_PHY_EQ_LANED1_MASK			(0x3 << CMB_PHY_EQ_LANED1)
#define CMB_PHY_EQ_LANED2			20
#define CMB_PHY_EQ_LANED2_MASK			(0x3 << CMB_PHY_EQ_LANED2)
#define CMB_PHY_EQ_LANED3			22
#define CMB_PHY_EQ_LANED3_MASK			(0x3 << CMB_PHY_EQ_LANED3)
#define CMB_PHY_EQ_LANECK0			24
#define CMB_PHY_EQ_LANECK0_MASK			(0x3 << CMB_PHY_EQ_LANECK0)
#define CMB_PHY_EQ_LANECK1			26
#define CMB_PHY_EQ_LANECK1_MASK			(0x3 << CMB_PHY_EQ_LANECK1)

#define CMB_PHY_OFSCAL0_OFF			0x008
#define CMB_PHY0_OFSCAL_AUTO			4
#define CMB_PHY0_OFSCAL_AUTO_MASK		(0x1 << CMB_PHY0_OFSCAL_AUTO)
#define CMB_PHY1_OFSCAL_AUTO			5
#define CMB_PHY1_OFSCAL_AUTO_MASK		(0x1 << CMB_PHY1_OFSCAL_AUTO)
#define CMB_PHY0_OFSCAL_SOFT			12
#define CMB_PHY0_OFSCAL_SOFT_MASK		(0x1 << CMB_PHY0_OFSCAL_SOFT)
#define CMB_PHY1_OFSCAL_SOFT			13
#define CMB_PHY1_OFSCAL_SOFT_MASK		(0x1 << CMB_PHY1_OFSCAL_SOFT)
#define CMB_PHY0_OFSCAL_RESETN			20
#define CMB_PHY0_OFSCAL_RESETN_MASK		(0x1 << CMB_PHY0_OFSCAL_RESETN)
#define CMB_PHY1_OFSCAL_RESETN			21
#define CMB_PHY1_OFSCAL_RESETN_MASK		(0x1 << CMB_PHY1_OFSCAL_RESETN)
#define CMB_PHY0_OFSCAL_FLAG			28
#define CMB_PHY0_OFSCAL_FLAG_MASK		(0x1 << CMB_PHY0_OFSCAL_FLAG)
#define CMB_PHY1_OFSCAL_FLAG			29
#define CMB_PHY1_OFSCAL_FLAG_MASK		(0x1 << CMB_PHY1_OFSCAL_FLAG)

#define CMB_PHY_OFSCAL1_OFF			0x00C
#define CMB_PHY0_OFSCAL_SET			20
#define CMB_PHY0_OFSCAL_SET_MASK		(0x1F << CMB_PHY0_OFSCAL_SET)
#define CMB_PHY1_OFSCAL_SET			25
#define CMB_PHY1_OFSCAL_SET_MASK		(0x1F << CMB_PHY1_OFSCAL_SET)

#define CMB_PHY_DESKEW0_OFF			0x014
#define CMB_PHY_DESKEW_EN			0
#define CMB_PHY_DESKEW_EN_MASK			(0xF << CMB_PHY_DESKEW_EN)
#define CMB_PHY_DESKEW_PERIOD_EN		4
#define CMB_PHY_DESKEW_PERIOD_EN_MASK		(0xF << CMB_PHY_DESKEW_PERIOD_EN)
#define CMB_PHY0_DESKEW_STEP			8
#define CMB_PHY0_DESKEW_STEP_MASK		(0x3 << CMB_PHY0_DESKEW_STEP)
#define CMB_PHY1_DESKEW_STEP			10
#define CMB_PHY1_DESKEW_STEP_MASK		(0x3 << CMB_PHY1_DESKEW_STEP)

#define CMB_PHY_DESKEW1_OFF			0x018
#define CMB_PHY_DESKEW_LANED0_SET		0
#define CMB_PHY_DESKEW_LANED0_SET_MASK		(0x1F << CMB_PHY_DESKEW_LANED0_SET)
#define CMB_PHY_DESKEW_LANED1_SET		5
#define CMB_PHY_DESKEW_LANED1_SET_MASK		(0x1F << CMB_PHY_DESKEW_LANED1_SET)
#define CMB_PHY_DESKEW_LANED2_SET		10
#define CMB_PHY_DESKEW_LANED2_SET_MASK		(0x1F << CMB_PHY_DESKEW_LANED2_SET)
#define CMB_PHY_DESKEW_LANED3_SET		15
#define CMB_PHY_DESKEW_LANED3_SET_MASK		(0x1F << CMB_PHY_DESKEW_LANED3_SET)
#define CMB_PHY_DESKEW_LANECK0_SET		20
#define CMB_PHY_DESKEW_LANECK0_SET_MASK		(0x1F << CMB_PHY_DESKEW_LANECK0_SET)
#define CMB_PHY_DESKEW_LANECK1_SET		20
#define CMB_PHY_DESKEW_LANECK1_SET_MASK		(0x1F << CMB_PHY_DESKEW_LANECK1_SET)

#define CMB_PHY_TERM_CTL_REG_OFF		0x020
#define CMB_PHY_TERMDT_EN			0
#define CMB_PHY_TERMDT_EN_MASK			(0xf << CMB_PHY_TERMDT_EN)
#define CMB_PHY_TERMCK_EN			4
#define CMB_PHY_TERMCK_EN_MASK			(0x3 << CMB_PHY_TERMCK_EN)
#define CMB_PHY0_TERM_EN_DLY			16
#define CMB_PHY0_TERM_EN_DLY_MASK		(0xff << CMB_PHY0_TERM_EN_DLY)
#define CMB_PHY1_TERM_EN_DLY			24
#define CMB_PHY1_TERM_EN_DLY_MASK		(0xff << CMB_PHY1_TERM_EN_DLY)

#define CMB_PHY_HS_CTL_REG_OFF			0x024
#define CMB_PHY_HSDT_EN				0
#define CMB_PHY_HSDT_EN_MASK			(0xf << CMB_PHY_HSDT_EN)
#define CMB_PHY_HSCK_EN				4
#define CMB_PHY_HSCK_EN_MASK			(0x3 << CMB_PHY_HSCK_EN)
#define CMB_PHY_HSDT_POLAR			8
#define CMB_PHY_HSDT_POLAR_MASK			(0xf << CMB_PHY_HSDT_POLAR)
#define CMB_PHY_HSCK_POLAR			12
#define CMB_PHY_HSCK_POLAR_MASK			(0xf << CMB_PHY_HSCK_POLAR)
#define CMB_PHY0_HS_DLY				16
#define CMB_PHY0_HS_DLY_MASK			(0xff << CMB_PHY0_HS_DLY)
#define CMB_PHY1_HS_DLY				24
#define CMB_PHY1_HS_DLY_MASK			(0xff << CMB_PHY1_HS_DLY)

#define CMB_PHY_S2P_CTL_REG_OFF			0x028
#define CMB_PHY_S2P_EN				0
#define CMB_PHY_S2P_EN_MASK			(0xf << CMB_PHY_S2P_EN)
#define CMB_PHY0_S2P_WIDTH			8
#define CMB_PHY0_S2P_WIDTH_MASK			(0x3 << CMB_PHY0_S2P_WIDTH)
#define CMB_PHY1_S2P_WIDTH			10
#define CMB_PHY1_S2P_WIDTH_MASK			(0x3 << CMB_PHY1_S2P_WIDTH)
#define CMB_PHY0_S2P_DLY			16
#define CMB_PHY0_S2P_DLY_MASK			(0xff << CMB_PHY0_S2P_DLY)
#define CMB_PHY1_S2P_DLY			24
#define CMB_PHY1_S2P_DLY_MASK			(0xff << CMB_PHY1_S2P_DLY)

#define CMB_PHY_MIPIRX_CTL_REG_OFF		0x02C
#define CMB_PHY0_MIPIHS_ENDLAN			0
#define CMB_PHY0_MIPIHS_ENDLAN_MASK		(0x1 << CMB_PHY0_MIPIHS_ENDLAN)
#define CMB_PHY1_MIPIHS_ENDLAN			1
#define CMB_PHY1_MIPIHS_ENDLAN_MASK		(0x1 << CMB_PHY1_MIPIHS_ENDLAN)
#define CMB_PHY0_MIPIHS_SYNC_MODE		2
#define CMB_PHY0_MIPIHS_SYNC_MODE_MASK		(0x1 << CMB_PHY0_MIPIHS_SYNC_MODE)
#define CMB_PHY1_MIPIHS_SYNC_MODE		3
#define CMB_PHY1_MIPIHS_SYNC_MODE_MASK		(0x1 << CMB_PHY1_MIPIHS_SYNC_MODE)
#define CMB_PHY0_MIPIHS_8B9B			4
#define CMB_PHY0_MIPIHS_8B9B_MASK		(0x1 << CMB_PHY0_MIPIHS_8B9B)
#define CMB_PHY1_MIPIHS_8B9B			5
#define CMB_PHY1_MIPIHS_8B9B_MASK		(0x1 << CMB_PHY1_MIPIHS_8B9B)
#define CMB_PHY_MIPI_LPDT_EN			8
#define CMB_PHY_MIPI_LPDT_EN_MASK		(0xf << CMB_PHY_MIPI_LPDT_EN)
#define CMB_PHY_MIPI_LPCK_EN			12
#define CMB_PHY_MIPI_LPCK_EN_MASK		(0x3 << CMB_PHY_MIPI_LPCK_EN)
#define CMB_PHY0_MIPILP_DBC_EN			16
#define CMB_PHY0_MIPILP_DBC_EN_MASK		(0x1 << CMB_PHY0_MIPILP_DBC_EN)
#define CMB_PHY1_MIPILP_DBC_EN			17
#define CMB_PHY1_MIPILP_DBC_EN_MASK		(0x1 << CMB_PHY1_MIPILP_DBC_EN)

/*
 * Detail information of registers----PORT0/1
 */
#define CMB_PORT_REG_OFF			0x1000
#define CMB_PORT_BETWEEN_OFF			0x400

#define CMB_PORT_CTL_REG_OFF			0x0000
#define CMB_PORT_EN				0
#define CMB_PORT_EN_MASK			(0x1 << CMB_PORT_EN)
#define CMB_PORT_WORK_MODE			4
#define CMB_PORT_WORK_MODE_MASK			(0x3 << CMB_PORT_WORK_MODE)
#define CMB_PORT_LANE_NUM			8
#define CMB_PORT_LANE_NUM_MASK			(0xf << CMB_PORT_LANE_NUM)
#define CMB_PORT_CHANNEL_NUM			16
#define CMB_PORT_CHANNEL_NUM_MASK		(0x3 << CMB_PORT_CHANNEL_NUM)
#define CMB_PORT_OUT_NUM			31
#define CMB_PORT_OUT_NUM_MASK			(0x1 << CMB_PORT_OUT_NUM)

#define CMB_PORT_LANE_MAP_REG0_OFF		0x0004
#define CMB_PORT_LANE0_ID			0
#define CMB_PORT_LANE0_ID_MASK			(0xf << CMB_PORT_LANE0_ID)
#define CMB_PORT_LANE1_ID			4
#define CMB_PORT_LANE1_ID_MASK			(0xf << CMB_PORT_LANE1_ID)
#define CMB_PORT_LANE2_ID			8
#define CMB_PORT_LANE2_ID_MASK			(0xf << CMB_PORT_LANE2_ID)
#define CMB_PORT_LANE3_ID			12
#define CMB_PORT_LANE3_ID_MASK			(0xf << CMB_PORT_LANE3_ID)
#define CMB_PORT_LANE4_ID			16
#define CMB_PORT_LANE4_ID_MASK			(0xf << CMB_PORT_LANE4_ID)
#define CMB_PORT_LANE5_ID			20
#define CMB_PORT_LANE5_ID_MASK			(0xf << CMB_PORT_LANE5_ID)
#define CMB_PORT_LANE6_ID			24
#define CMB_PORT_LANE6_ID_MASK			(0xf << CMB_PORT_LANE6_ID)
#define CMB_PORT_LANE7_ID			28
#define CMB_PORT_LANE7_ID_MASK			(0xf << CMB_PORT_LANE7_ID)

#define CMB_PORT_LANE_MAP_REG1_OFF		0x0008
#define CMB_PORT_LANE8_ID			0
#define CMB_PORT_LANE8_ID_MASK			(0xf << CMB_PORT_LANE8_ID)
#define CMB_PORT_LANE9_ID			4
#define CMB_PORT_LANE9_ID_MASK			(0xf << CMB_PORT_LANE9_ID)
#define CMB_PORT_LANE10_ID			8
#define CMB_PORT_LANE10_ID_MASK			(0xf << CMB_PORT_LANE10_ID)
#define CMB_PORT_LANE11_ID			12
#define CMB_PORT_LANE11_ID_MASK			(0xf << CMB_PORT_LANE11_ID)

#define CMB_PORT_WDR_MODE_REG_OFF		0x000C
#define CMB_PORT_WDR_MODE			0
#define CMB_PORT_WDR_MODE_MASK			(0x3 << CMB_PORT_WDR_MODE)

#define CMB_PORT_FID_SEL_REG_OFF		0x0010
#define CMB_PORT_FID0_MAP			0
#define CMB_PORT_FID0_MAP_MASK			(0xf << CMB_PORT_FID0_MAP)
#define CMB_PORT_FID1_MAP			4
#define CMB_PORT_FID1_MAP_MASK			(0xf << CMB_PORT_FID1_MAP)
#define CMB_PORT_FID2_MAP			8
#define CMB_PORT_FID2_MAP_MASK			(0xf << CMB_PORT_FID2_MAP)
#define CMB_PORT_FID3_MAP			12
#define CMB_PORT_FID3_MAP_MASK			(0xf << CMB_PORT_FID3_MAP)
#define CMB_PORT_FID_MAP_EN			16
#define CMB_PORT_FID_MAP_EN_MASK		(0xf << CMB_PORT_FID_MAP_EN)
#define CMB_PORT_SYNC_CODE_WITH_PD		20
#define CMB_PORT_SYNC_CODE_WITH_PD_MASK		(0xf << CMB_PORT_SYNC_CODE_WITH_PD)
#define CMB_PORT_FID_MODE			31
#define CMB_PORT_FID_MODE_MASK			(0x1 << CMB_PORT_FID_MODE)

#define CMB_PORT_MIPI_CFG_REG_OFF		0x0100
#define CMB_MIPI_UNPACK_EN			0
#define CMB_MIPI_UNPACK_EN_MASK			(0x1 << CMB_MIPI_UNPACK_EN)
#define CMB_MIPI_NO_UNPACK_ALL			1
#define CMB_MIPI_NO_UNPACK_ALL_MASK		(0x1 << CMB_MIPI_NO_UNPACK_ALL)
#define CMB_EMBED_EN				2
#define CMB_EMBED_EN_MASK			(0x1 << CMB_EMBED_EN)
#define CMB_MIPI_USER_DEF_EN			3
#define CMB_MIPI_USER_DEF_EN_MASK		(0x1 << CMB_MIPI_USER_DEF_EN)
#define CMB_MIPI_PH_BYTEORD			4
#define CMB_MIPI_PH_BYTEORD_MASK		(0x3 << CMB_MIPI_PH_BYTEORD)
#define CMB_MIPI_PH_BITOED			6
#define CMB_MIPI_PH_BITOED_MASK			(0x1 << CMB_MIPI_PH_BITOED)
#define CMB_MIPI_PL_BITORD			7
#define CMB_MIPI_PL_BITORD_MASK			(0x1 << CMB_MIPI_PL_BITORD)
#define CMB_MIPI_LINE_SYNC_EN			8
#define CMB_MIPI_LINE_SYNC_EN_MASK		(0x1 << CMB_MIPI_LINE_SYNC_EN)
#define CMB_MIPI_YUV_SEQ			16
#define CMB_MIPI_YUV_SEQ_MASK			(0x3 << CMB_MIPI_YUV_SEQ)

#define CMB_PORT_MIPI_NO_UNPAK_NUM_REG_OFF	0x0104
#define CMB_MIPI_NO_UNPACK_NUM_REG		0
#define CMB_MIPI_NO_UNPACK_NUM_REG_MASK		(0xFFFF << CMB_MIPI_NO_UNPACK_NUM_REG)

#define CMB_PORT_MIPI_DI_REG_OFF		0x0108
#define CMB_MIPI_CH0_DT				0
#define CMB_MIPI_CH0_DT_MASK			(0x3f << CMB_MIPI_CH0_DT)
#define CMB_MIPI_CH0_VC				6
#define CMB_MIPI_CH0_VC_MASK			(0x3 << CMB_MIPI_CH0_VC)
#define CMB_MIPI_CH1_DT				8
#define CMB_MIPI_CH1_DT_MASK			(0x3f << CMB_MIPI_CH1_DT)
#define CMB_MIPI_CH1_VC				14
#define CMB_MIPI_CH1_VC_MASK			(0x3 << CMB_MIPI_CH1_VC)
#define CMB_MIPI_CH2_DT				16
#define CMB_MIPI_CH2_DT_MASK			(0x3f << CMB_MIPI_CH2_DT)
#define CMB_MIPI_CH2_VC				22
#define CMB_MIPI_CH2_VC_MASK			(0x3 << CMB_MIPI_CH2_VC)
#define CMB_MIPI_CH3_DT				24
#define CMB_MIPI_CH3_DT_MASK			(0x3f << CMB_MIPI_CH3_DT)
#define CMB_MIPI_CH3_VC				30
#define CMB_MIPI_CH3_VC_MASK			(0x3 << CMB_MIPI_CH3_VC)

#define CMB_PORT_MIPI_USER_DI_REG_OFF		0x010C
#define CMB_MIPI_USER0_DT			0
#define CMB_MIPI_USER0_DT_MASK			(0x3f << CMB_MIPI_USER0_DT)
#define CMB_MIPI_USER0_DT_EN			7
#define CMB_MIPI_USER0_DT_EN_MASK		(0x1 << CMB_MIPI_USER0_DT_EN)
#define CMB_MIPI_USER1_DT			8
#define CMB_MIPI_USER1_DT_MASK			(0x3f << CMB_MIPI_USER1_DT)
#define CMB_MIPI_USER1_DT_EN			15
#define CMB_MIPI_USER1_DT_EN_MASK		(0x1 << CMB_MIPI_USER1_DT_EN)
#define CMB_MIPI_USER2_DT			16
#define CMB_MIPI_USER2_DT_MASK			(0x3f << CMB_MIPI_USER2_DT)
#define CMB_MIPI_USER2_DT_EN			23
#define CMB_MIPI_USER2_DT_EN_MASK		(0x1 << CMB_MIPI_USER2_DT_EN)
#define CMB_MIPI_USER3_DT			24
#define CMB_MIPI_USER3_DT_MASK			(0x3f << CMB_MIPI_USER3_DT)
#define CMB_MIPI_USER3_DT_EN			31
#define CMB_MIPI_USER3_DT_EN_MASK		(0x1 << CMB_MIPI_USER3_DT_EN)

#define CMB_PORT_MIPI_DI_TRIG_REG_OFF		0x0110
#define CMB_MIPI_FS				0
#define CMB_MIPI_FS_MASK			(0x1 << CMB_MIPI_FS)
#define CMB_MIPI_FE				1
#define CMB_MIPI_FE_MASK			(0x1 << CMB_MIPI_FE)
#define CMB_MIPI_LS				2
#define CMB_MIPI_LS_MASK			(0x1 << CMB_MIPI_LS)
#define CMB_MIPI_LE				3
#define CMB_MIPI_LE_MASK			(0x1 << CMB_MIPI_LE)
#define CMB_MIPI_GS0				8
#define CMB_MIPI_GS0_MASK			(0x1 << CMB_MIPI_GS0)
#define CMB_MIPI_GS1				9
#define CMB_MIPI_GS1_MASK			(0x1 << CMB_MIPI_GS1)
#define CMB_MIPI_GS2				10
#define CMB_MIPI_GS2_MASK			(0x1 << CMB_MIPI_GS2)
#define CMB_MIPI_GS3				11
#define CMB_MIPI_GS3_MASK			(0x1 << CMB_MIPI_GS3)
#define CMB_MIPI_GS4				12
#define CMB_MIPI_GS4_MASK			(0x1 << CMB_MIPI_GS4)
#define CMB_MIPI_GS5				13
#define CMB_MIPI_GS5_MASK			(0x1 << CMB_MIPI_GS5)
#define CMB_MIPI_GS6				14
#define CMB_MIPI_GS6_MASK			(0x1 << CMB_MIPI_GS6)
#define CMB_MIPI_GS7				15
#define CMB_MIPI_GS7_MASK			(0x1 << CMB_MIPI_GS7)
#define CMB_MIPI_GL				16
#define CMB_MIPI_GL_MASK			(0x1 << CMB_MIPI_GL)
#define CMB_MIPI_YUV				17
#define CMB_MIPI_YUV_MASK			(0x1 << CMB_MIPI_YUV)
#define CMB_MIPI_RGB				18
#define CMB_MIPI_RGB_MASK			(0x1 << CMB_MIPI_RGB)
#define CMB_MIPI_RAW				19
#define CMB_MIPI_RAW_MASK			(0x1 << CMB_MIPI_RAW)
#define CMB_MIPI_SRC_IS_FIELD			30
#define CMB_MIPI_SRC_IS_FIELD_MASK		(0x1 << CMB_MIPI_SRC_IS_FIELD)
#define CMB_MIPI_FIELD_REV			31
#define CMB_MIPI_FIELD_REV_MASK			(0x1 << CMB_MIPI_FIELD_REV)

#endif
