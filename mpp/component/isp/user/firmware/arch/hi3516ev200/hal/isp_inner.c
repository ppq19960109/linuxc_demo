/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_inner.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2018/05/09
  Description   :
  History       :
  1.Date        : 2018/05/09
    Author      :
    Modification: Created file

******************************************************************************/

#include "mpi_sys.h"
#include "hi_comm_vi.h"
#include "hi_comm_isp.h"
#include "hi_comm_3a.h"
#include "hi_ae_comm.h"
#include "hi_awb_comm.h"
#include "isp_inner.h"
#include "isp_main.h"
#include "isp_vreg.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_VOID ISP_CalcGridInfo(HI_U16 u16Wdith, HI_U16 u16StartPox, HI_U16 u16BlockNum, HI_U16 *au16GridInfo)
{
    HI_U16 i ;
    HI_U16 integer;
    HI_U16 remainder;

    integer   = u16Wdith / DIV_0_TO_1(u16BlockNum);
    remainder = u16Wdith % DIV_0_TO_1(u16BlockNum);
    au16GridInfo[0] = u16StartPox;
    for (i = 1; i < u16BlockNum; i++)
    {
        if (remainder > 0)
        {
            au16GridInfo[i] =  au16GridInfo[i - 1] + integer + 1;
            remainder = remainder - 1;
        }
        else
        {
            au16GridInfo[i] =  au16GridInfo[i - 1] + integer ;
        }
    }

    return;
}
HI_U32 ISP_GetStripingActiveImgStart(HI_U8 u8BlockIndex, ISP_WORKING_MODE_S *pstIspWorkMode)
{
    HI_U32 u32OverLap ;
    HI_U32 u32BlockStart;

    u32OverLap = pstIspWorkMode->u32OverLap;
    if (u8BlockIndex == 0)
    {
        u32BlockStart = pstIspWorkMode->astBlockRect[u8BlockIndex].s32X ;
    }
    else
    {
        u32BlockStart = pstIspWorkMode->astBlockRect[u8BlockIndex].s32X + u32OverLap;
    }
    return u32BlockStart;
}

HI_U32 ISP_GetStripingActiveImgWidth(HI_U8 u8BlockIndex, ISP_WORKING_MODE_S *pstIspWorkMode)
{
    HI_U32 u32BlockWidth;
    HI_U32 u32OverLap;
    HI_U8   u8BlockNum;

    u32OverLap    = pstIspWorkMode->u32OverLap;
    u32BlockWidth = pstIspWorkMode->astBlockRect[u8BlockIndex].u32Width;
    u8BlockNum    = pstIspWorkMode->u8BlockNum;

    if ((u8BlockIndex == 0 ) || (u8BlockIndex == (u8BlockNum - 1))) //first block and last block
    {
        u32BlockWidth = u32BlockWidth - u32OverLap;
    }
    else
    {
        u32BlockWidth = u32BlockWidth - u32OverLap * 2;
    }
    return u32BlockWidth;
}

HI_U32 ISP_GetStripingGridXInfo(HI_U16 *au16GridPos, HI_U16 u16GridNum, ISP_WORKING_MODE_S *pstIspWorkMode)
{
    HI_U8  i;
    HI_U16 u16Start;
    HI_U16 u16Width;
    HI_U16 u16DivNum;
    HI_U16 index = 0;

    for (i = 0; i < pstIspWorkMode->u8BlockNum; i++)
    {
        u16Start = ISP_GetStripingActiveImgStart(i, pstIspWorkMode);
        u16Width = ISP_GetStripingActiveImgWidth(i, pstIspWorkMode);

        if ( i < u16GridNum % DIV_0_TO_1(pstIspWorkMode->u8BlockNum))
        {
            u16DivNum = u16GridNum / DIV_0_TO_1(pstIspWorkMode->u8BlockNum) + 1;
        }
        else
        {
            u16DivNum = u16GridNum / DIV_0_TO_1(pstIspWorkMode->u8BlockNum) ;
        }
        ISP_CalcGridInfo(u16Width, u16Start, u16DivNum, &(au16GridPos[index]));
        index = index + u16DivNum;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_GetAEGridInfo(VI_PIPE ViPipe, ISP_AE_GRID_INFO_S *pstFEGridInfo, ISP_AE_GRID_INFO_S *pstBEGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16ImgTotalWidth, u16ImgTotalHeight;
    HI_U16  u16ImgStartX, u16ImgStartY;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX, u16BeStartY;
    ISP_WORKING_MODE_S stIspWorkMode;

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    bCropEn = hi_ext_system_ae_crop_en_read(ViPipe);

    if (HI_TRUE == bCropEn)
    {
        u16ImgStartX      = hi_ext_system_ae_crop_x_read(ViPipe);
        u16ImgStartY      = hi_ext_system_ae_crop_y_read(ViPipe);
        u16ImgTotalWidth  = hi_ext_system_ae_crop_width_read(ViPipe);
        u16ImgTotalHeight = hi_ext_system_ae_crop_height_read(ViPipe);
    }
    else
    {
        u16ImgStartX      = 0;
        u16ImgStartY      = 0;
        u16ImgTotalWidth  = hi_ext_sync_total_width_read(ViPipe);
        u16ImgTotalHeight = hi_ext_sync_total_height_read(ViPipe);
    }

    ISP_CalcGridInfo(u16ImgTotalWidth,  u16ImgStartX, AE_ZONE_COLUMN, pstFEGridInfo->au16GridXPos);
    ISP_CalcGridInfo(u16ImgTotalHeight, u16ImgStartY, AE_ZONE_ROW, pstFEGridInfo->au16GridYPos);

    pstFEGridInfo->au16GridXPos[AE_ZONE_COLUMN] = u16ImgTotalWidth - 1 ;
    pstFEGridInfo->au16GridYPos[AE_ZONE_ROW]    = u16ImgTotalHeight - 1;
    pstFEGridInfo->u8Status = 1;

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstBEGridInfo->au16GridXPos, AE_ZONE_COLUMN, &stIspWorkMode);
        u16BeStartY   = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight   = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, AE_ZONE_ROW, pstBEGridInfo->au16GridYPos);
        u16BeWidth    = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_system_ae_crop_x_read(ViPipe);
            u16BeStartY = hi_ext_system_ae_crop_y_read(ViPipe);
            u16BeWidth  = hi_ext_system_ae_crop_width_read(ViPipe);
            u16BeHeight = hi_ext_system_ae_crop_height_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth,  u16BeStartX, AE_ZONE_COLUMN, pstBEGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, AE_ZONE_ROW, pstBEGridInfo->au16GridYPos);
    }

    pstBEGridInfo->au16GridXPos[AE_ZONE_COLUMN] = u16BeWidth  - 1; //last position
    pstBEGridInfo->au16GridYPos[AE_ZONE_ROW]    = u16BeHeight - 1; //last position

    pstBEGridInfo->u8Status = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetMGGridInfo(VI_PIPE ViPipe, ISP_MG_GRID_INFO_S *pstGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX, u16BeStartY;
    ISP_WORKING_MODE_S stIspWorkMode;

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    bCropEn = hi_ext_system_ae_crop_en_read(ViPipe);

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstGridInfo->au16GridXPos, MG_ZONE_COLUMN, &stIspWorkMode);
        u16BeStartY   = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight   = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, MG_ZONE_ROW, pstGridInfo->au16GridYPos);
        u16BeWidth    = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_system_ae_crop_x_read(ViPipe);
            u16BeStartY = hi_ext_system_ae_crop_y_read(ViPipe);
            u16BeWidth  = hi_ext_system_ae_crop_width_read(ViPipe);
            u16BeHeight = hi_ext_system_ae_crop_height_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth,  u16BeStartX, MG_ZONE_COLUMN, pstGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, MG_ZONE_ROW,    pstGridInfo->au16GridYPos);

    }

    pstGridInfo->au16GridXPos[AE_ZONE_COLUMN] = u16BeWidth  - 1; //last position
    pstGridInfo->au16GridYPos[AE_ZONE_ROW]    = u16BeHeight - 1; //last position
    pstGridInfo->u8Status                     = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetAFGridInfo(VI_PIPE ViPipe, ISP_FOCUS_GRID_INFO_S *pstFEGridInfo, ISP_FOCUS_GRID_INFO_S *pstBEGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16ImgTotalWidth, u16ImgTotalHeight;
    HI_U16  u16ImgStartX, u16ImgStartY;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX, u16BeStartY;
    ISP_WORKING_MODE_S stIspWorkMode;
    // 1.get block info
    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    bCropEn = hi_ext_af_crop_enable_read(ViPipe);

    if (HI_TRUE == bCropEn)
    {
        u16ImgStartX      = hi_ext_af_crop_pos_x_read(ViPipe);
        u16ImgStartY      = hi_ext_af_crop_pos_y_read(ViPipe);
        u16ImgTotalWidth  = hi_ext_af_crop_hsize_read(ViPipe);
        u16ImgTotalHeight = hi_ext_af_crop_vsize_read(ViPipe);
    }
    else
    {
        u16ImgStartX      = 0;
        u16ImgStartY      = 0;
        u16ImgTotalWidth  = hi_ext_sync_total_width_read(ViPipe);
        u16ImgTotalHeight = hi_ext_sync_total_height_read(ViPipe);
    }

    ISP_CalcGridInfo(u16ImgTotalWidth,  u16ImgStartX, AF_ZONE_COLUMN, pstFEGridInfo->au16GridXPos);
    ISP_CalcGridInfo(u16ImgTotalHeight, u16ImgStartY, AF_ZONE_ROW,    pstFEGridInfo->au16GridYPos);

    pstFEGridInfo->au16GridXPos[AF_ZONE_COLUMN] = u16ImgTotalWidth  - 1;
    pstFEGridInfo->au16GridYPos[AF_ZONE_ROW]    = u16ImgTotalHeight - 1;
    pstFEGridInfo->u8Status                     = 1;

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstBEGridInfo->au16GridXPos, AF_ZONE_COLUMN, &stIspWorkMode);
        u16BeStartY = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, AF_ZONE_ROW, pstBEGridInfo->au16GridYPos);
        u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_af_crop_pos_x_read(ViPipe);
            u16BeStartY = hi_ext_af_crop_pos_y_read(ViPipe);
            u16BeWidth  = hi_ext_af_crop_hsize_read(ViPipe);
            u16BeHeight = hi_ext_af_crop_vsize_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth,  u16BeStartX, AF_ZONE_COLUMN, pstBEGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, AF_ZONE_ROW,    pstBEGridInfo->au16GridYPos);

    }
    pstBEGridInfo->au16GridXPos[AF_ZONE_COLUMN] = u16BeWidth  - 1; //last position
    pstBEGridInfo->au16GridYPos[AF_ZONE_ROW]    = u16BeHeight - 1; //last position
    pstBEGridInfo->u8Status = 1;
    return HI_SUCCESS;
}

HI_S32 ISP_GetWBGridInfo(VI_PIPE ViPipe, ISP_AWB_GRID_INFO_S *pstGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX, u16BeStartY;
    HI_U16  u16awbXGridNum, u16awbYGridNum;
    ISP_WORKING_MODE_S stIspWorkMode;

    if (HI_SUCCESS !=  ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    memset(pstGridInfo->au16GridXPos, 0, sizeof(HI_U16) * (AWB_ZONE_ORIG_COLUMN + 1));
    memset(pstGridInfo->au16GridYPos, 0, sizeof(HI_U16) * (AWB_ZONE_ORIG_ROW + 1));

    u16awbYGridNum = hi_ext_system_awb_vnum_read(ViPipe);
    u16awbXGridNum = hi_ext_system_awb_hnum_read(ViPipe);
    bCropEn        = hi_ext_system_awb_crop_en_read(ViPipe);

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstGridInfo->au16GridXPos, u16awbXGridNum, &stIspWorkMode);
        u16BeStartY = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, u16awbYGridNum, pstGridInfo->au16GridYPos);
        u16BeWidth    = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_system_awb_crop_x_read(ViPipe);
            u16BeStartY = hi_ext_system_awb_crop_y_read(ViPipe);
            u16BeWidth  = hi_ext_system_awb_crop_width_read(ViPipe);
            u16BeHeight = hi_ext_system_awb_crop_height_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth, u16BeStartX, u16awbXGridNum, pstGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, u16awbYGridNum, pstGridInfo->au16GridYPos);
    }

    pstGridInfo->au16GridXPos[u16awbXGridNum] = u16BeWidth  - 1; //last position
    pstGridInfo->au16GridYPos[u16awbYGridNum] = u16BeHeight - 1; //last position
    pstGridInfo->u8Status                     = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetAEStitchStatistics(VI_PIPE ViPipe, ISP_AE_STITCH_STATISTICS_S *pstAeStitchStat)
{
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetWBStitchStatistics(VI_PIPE ViPipe, ISP_WB_STITCH_STATISTICS_S *pstStitchWBStat)
{
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetFeFocusStatistics(VI_PIPE ViPipe, ISP_FE_FOCUS_STATISTICS_S *pstFEAFStat, ISP_STAT_S *pstIspActStat, HI_U8 u8WdrChn)
{
    memset(pstFEAFStat, 0, sizeof(ISP_FE_FOCUS_STATISTICS_S));

    return HI_SUCCESS;
}

HI_S32 ISP_SetRadialShadingAttr(VI_PIPE ViPipe, const ISP_RADIAL_SHADING_ATTR_S *pstRaShadingAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetRadialShadingAttr(VI_PIPE ViPipe, ISP_RADIAL_SHADING_ATTR_S *pstRaShadingAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetRadialShadingLUT(VI_PIPE ViPipe, const ISP_RADIAL_SHADING_LUT_ATTR_S *pstRaShadingLutAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetRadialShadingLUT(VI_PIPE ViPipe, ISP_RADIAL_SHADING_LUT_ATTR_S *pstRaShadingLutAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetPipeDifferAttr(VI_PIPE ViPipe, const ISP_PIPE_DIFF_ATTR_S *pstPipeDiffer)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetPipeDifferAttr(VI_PIPE ViPipe, ISP_PIPE_DIFF_ATTR_S *pstPipeDiffer)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetRcAttr(VI_PIPE ViPipe, const ISP_RC_ATTR_S *pstRcAttr)
{
    HI_U16 u16Width, u16Height, u16Max;
    HI_U32 u32SqLength;
    HI_U8 u8WdrMode;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstRcAttr);
    ISP_CHECK_BOOL(pstRcAttr->bEnable);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    u16Width  = hi_ext_sync_total_width_read(ViPipe);
    u16Height = hi_ext_sync_total_height_read(ViPipe);
    u32SqLength = (HI_U32)(u16Width * u16Width + u16Height * u16Height);
    u16Max      = Sqrt32(u32SqLength);

    if (pstRcAttr->stCenterCoor.s32X >= u16Width || pstRcAttr->stCenterCoor.s32X < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stCenterCoor.s32X value!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pstRcAttr->stCenterCoor.s32Y >= u16Height || pstRcAttr->stCenterCoor.s32Y < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid stCenterCoor.s32Y value!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pstRcAttr->u32Radius >= (HI_U32)u16Max)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32Radius value!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    u8WdrMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);
    if((0 == u8WdrMode )||(1 == u8WdrMode )) /* line or build-in-wdr */
    {
        hi_ext_system_rc_en_write(ViPipe, pstRcAttr->bEnable);
    }
    else
    {
        hi_ext_system_rc_en_write(ViPipe, HI_FALSE);
        ISP_TRACE(HI_DBG_ERR, "Not support !\n");
        return HI_ERR_ISP_NOT_SUPPORT;
    }
    hi_ext_system_rc_center_hor_coor_write(ViPipe, pstRcAttr->stCenterCoor.s32X);
    hi_ext_system_rc_center_ver_coor_write(ViPipe, pstRcAttr->stCenterCoor.s32Y);
    hi_ext_system_rc_radius_write(ViPipe, pstRcAttr->u32Radius);
    hi_ext_system_rc_coef_update_en_write(ViPipe, HI_TRUE);

    return HI_SUCCESS;
}

HI_S32 ISP_GetRcAttr(VI_PIPE ViPipe, ISP_RC_ATTR_S *pstRcAttr)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstRcAttr);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    pstRcAttr->bEnable           = hi_ext_system_rc_en_read(ViPipe);
    pstRcAttr->stCenterCoor.s32X = hi_ext_system_rc_center_hor_coor_read(ViPipe);
    pstRcAttr->stCenterCoor.s32Y = hi_ext_system_rc_center_ver_coor_read(ViPipe);
    pstRcAttr->u32Radius         = hi_ext_system_rc_radius_read(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_SetRGBIRAttr(VI_PIPE ViPipe, const ISP_RGBIR_ATTR_S *pstRGBIRAttr)
{
    HI_U8 i;
    HI_S16 as16DftCvtMatrix[12] = {1000, 0, 0, -1000, 0, 1000, 0, -1000, 0, 0, 1000, -1000};
    HI_S16 as16MonoCvtMatrix[12] = {1000, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 1000, 0};
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_POINTER(pstRGBIRAttr);
    ISP_CHECK_BOOL(pstRGBIRAttr->bEnable);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);
    if ((HI_TRUE ==  pstRGBIRAttr->bEnable) && (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        ISP_TRACE(HI_DBG_ERR, "RGBIR can not open in WDR mode\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pstRGBIRAttr->enOutPattern >= BAYER_BUTT)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid Bayer Pattern %d!\n", pstRGBIRAttr->enOutPattern);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pstRGBIRAttr->enInPattern >= IRBAYER_BUTT)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid IR Bayer Pattern %d!\n", pstRGBIRAttr->enInPattern);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ( pstRGBIRAttr->enIRCvtMatMode >= ISP_IR_CVTMAT_MODE_BUTT)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid IR status input %d!\n", pstRGBIRAttr->enIRCvtMatMode);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ( (pstRGBIRAttr->au16ExpCtrl[0] >= 2048) || (pstRGBIRAttr->au16ExpCtrl[1] >= 2048) )
    {
        ISP_TRACE(HI_DBG_ERR, "Max Exposure control value is 2047, invalid IR expctrl input ExpCtrl1 = %d, ExpCtrp2 = %d!\n", pstRGBIRAttr->au16ExpCtrl[0], pstRGBIRAttr->au16ExpCtrl[1]);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    if ( (pstRGBIRAttr->au16Gain[0] >= 512) || (pstRGBIRAttr->au16Gain[1] >= 512))
    {
        ISP_TRACE(HI_DBG_ERR, "Max Exposure control gain value is 511, invalid IR gain value input Gain1 = %d, Gain2 = %d!\n", pstRGBIRAttr->au16Gain[0], pstRGBIRAttr->au16Gain[1]);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    hi_ext_system_rgbir_inpattern_write(ViPipe, pstRGBIRAttr->enInPattern);
    hi_ext_system_rgbir_outpattern_write(ViPipe, pstRGBIRAttr->enOutPattern);
    hi_ext_system_rgbir_irstatus_write(ViPipe, pstRGBIRAttr->enIRCvtMatMode);

    hi_ext_system_rgbir_enable_write(ViPipe, pstRGBIRAttr->bEnable);
    //hi_ext_system_rgbir_th_write(ViPipe, ISP_RGBIR_THTV_DFT);
    //hi_ext_system_rgbir_tv_write(ViPipe, ISP_RGBIR_THTV_DFT);

    for ( i = 0 ; i < ISP_EXP_CTRL_NUM ; i++ )
    {
        hi_ext_system_rgbir_expctrl_write(ViPipe, pstRGBIRAttr->au16ExpCtrl[i], i);
        hi_ext_system_rgbir_gain_write(ViPipe, pstRGBIRAttr->au16Gain[i], i);
    }

    for ( i = 0 ; i < ISP_RGBIR_CVTMATRIX_NUM; i++ )
    {
        if (ISP_IR_CVTMAT_MODE_USER == pstRGBIRAttr->enIRCvtMatMode)
        {
            hi_ext_system_rgbir_cvtmatrix_write(ViPipe, pstRGBIRAttr->as16CvtMatrix[i], i);
        }
        else if (ISP_IR_CVTMAT_MODE_MONO == pstRGBIRAttr->enIRCvtMatMode)
        {
            hi_ext_system_rgbir_cvtmatrix_write(ViPipe, as16MonoCvtMatrix[i], i);
        }
        else
        {
            hi_ext_system_rgbir_cvtmatrix_write(ViPipe, as16DftCvtMatrix[i], i);
        }
    }

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_SET_RGBIR_FORMAT, &pstRGBIRAttr->enInPattern))
    {
        ISP_TRACE(HI_DBG_ERR, "Set RGBIR FORMAT Err\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_GetRGBIRAttr(VI_PIPE ViPipe, ISP_RGBIR_ATTR_S *pstRGBIRAttr)
{
    HI_U8 i;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstRGBIRAttr);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    pstRGBIRAttr->bEnable = hi_ext_system_rgbir_enable_read(ViPipe);
    pstRGBIRAttr->enInPattern  = hi_ext_system_rgbir_inpattern_read(ViPipe);
    pstRGBIRAttr->enOutPattern = hi_ext_system_rgbir_outpattern_read(ViPipe);
    pstRGBIRAttr->enIRCvtMatMode = hi_ext_system_rgbir_irstatus_read(ViPipe);

    for ( i = 0; i < ISP_EXP_CTRL_NUM ; i++ )
    {
        pstRGBIRAttr->au16ExpCtrl[i] = hi_ext_system_rgbir_expctrl_read(ViPipe, i);
        pstRGBIRAttr->au16Gain[i] = hi_ext_system_rgbir_gain_read(ViPipe, i);
    }

    for ( i = 0 ; i < ISP_RGBIR_CVTMATRIX_NUM; i++ )
    {
        pstRGBIRAttr->as16CvtMatrix[i] = hi_ext_system_rgbir_cvtmatrix_read(ViPipe, i);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetPreLogLUTAttr(VI_PIPE ViPipe, const ISP_PRELOGLUT_ATTR_S *pstPreLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetPreLogLUTAttr(VI_PIPE ViPipe, ISP_PRELOGLUT_ATTR_S *pstPreLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetLogLUTAttr(VI_PIPE ViPipe, const ISP_LOGLUT_ATTR_S *pstLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetLogLUTAttr(VI_PIPE ViPipe, ISP_LOGLUT_ATTR_S *pstLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetClutCoeff(VI_PIPE ViPipe, const ISP_CLUT_LUT_S *pstClutLUT)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}
HI_S32 ISP_GetClutCoeff(VI_PIPE ViPipe, ISP_CLUT_LUT_S *pstClutLUT)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetClutAttr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S *pstClutAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}
HI_S32 ISP_GetClutAttr(VI_PIPE ViPipe, ISP_CLUT_ATTR_S *pstClutAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
