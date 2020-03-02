/*
 * (C) 2005-2019 MediaTek Inc. All rights reserved.
 *
 * Copyright Statement:
 *
 * This MT3620 driver software/firmware and related documentation
 * ("MediaTek Software") are protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. ("MediaTek"). You may only use, reproduce, modify, or
 * distribute (as applicable) MediaTek Software if you have agreed to and been
 * bound by this Statement and the applicable license agreement with MediaTek
 * ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
 * PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO
 * LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED
 * HEREUNDER WILL BE ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * RECEIVER TO MEDIATEK DURING THE PRECEDING TWELVE (12) MONTHS FOR SUCH
 * MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __MHAL_DMA_H__
#define __MHAL_DMA_H__

#include "mhal_osai.h"

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup DMA
 * @{
 * This section introduces the Direct Memory Access (DMA) APIs including
 * terms and acronyms, supported features, software architecture, details
 * on how to use this driver, enums, structures and functions.
 *
 * @section DRIVER_DMA_Terms_Chapter Terms and Acronyms
 *
 * |Terms			|Details |
 * |----------------------------|------------------------------------------|
 * |\b DMA		| Direct Memory Access.|
 * |\b FIFO             | First in, First Out.|
 * |\b VFF \b DMA	| Virtual FIFO DMA.|
 * @section DRIVER_DMA_Features_Chapter Supported Features
 *
 * This controller supports DMA devices, which have multiple hardware DMA
 * channels. Different channels can transfer data simultaneously.\n There
 * are three DMA channel types: FULL-SIZE DMA, HALF-SIZE DMA and Virtual
 * FIFO DMA. FULL-SIZE DMA supports data transfer from \n memory to memory.
 * HALF-SIZE DMA supports data transfer from peripheral to memory or opposite
 * direction. Virtual FIFO DMA also supports\n data transfer between memory and
 * peripheral, but it has an additional FIFO control engine on memory side;
 * the memory space will be FIFO.\n For more details, please refer to the DMA
 * datasheet.
 * @}
 * @}
 */

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup DMA
 * @{
 * @section Note
 *  - DMA Interrupt Description:\n
 *   There are four kinds of interrupt notifications in various DMA channels.\n
 *   The interrupt of transfer completion notifies event of the completion of
 *   transfer for both FULL-SIZE DMA and HALF-SIZE DMA.\n The interrupt of half
 *   transfer completion notifies event that half of tranfser data has been
 *   completed and it is ony for HALF-SIZE DMA.\n VFF DMA (Virtual FIFO DMA) has
 *   two kinds of interrupt, and one notifies event that FIFO data is over or
 *   under a certain threshold,\n the other is FIFO timeout interrupt which
 *   shows there is no new data into the FIFO in the previous FIFO timeout bus
 *   cycles.\n
 *    - VFF DMA FIFO data interrupt:
 *     - For data transfer from peripheral to memory, interrupt is issued
 *       when data in FIFO are more than or equal to threshod;
 *     - For data transfer from memory to peripheral, interrupt is issued
 *       when data in FIFO are less than threshod;

 *  - The Source and Destination Configuration of Data Transfer:
 *   - For FULL-SIZE DMA, the src_addr and dst_addr register specify the base
 *     address of transfer source and destination respectively for a DMA
 *     channel. \n Data transfer is from src_addr to dst_addr. The src_addr and
 *     dst_addr are both memory space.
 *   - For HALF-SIZE DMA and VFF DMA, fix_addr register specifies the
 *     peripheral side and prog_addr register specifies the memory side.\n
 *     If transfer direction is from peripheral to memory, fix_addr is
 *     source and prog_addr is destination. If transfer direction is
 *     opposite,\n fix_addr is destination and prog_addr is source.
 *   - For VFF DMA, the prog_addr register is FIFO base address and the FIFO
 *     engine uses HWPTR and SWPTR to control FIFO access:
 *     - For data transfer from peripheral to memory, HWPTR is write pointer
 *       and SWPTR is read pointer;
 *     - For data transfer from memory to peripheral, HWPTR is read pointer
 *       and SWPTR is write pointer;\n
 *     - HWPTR and SWPTR are offset of FIFO base address.

 * @}
 * @}
 */

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup DMA
 * @{
 * @section MHAL_DMA_Driver_Usage_Chapter How to Use This Driver
 *
 * - \b SW \b Architecture: \n
 * See @ref MHAL_Overview_2_Chapter for details of the SW architecture.
 *
 *  This section describes the Common Interface API definition and provides
 *  an example on freeRTOS to show how to use these APIs to develop
 *  an OS-related DMA driver.\n
 *
 *
 * - \b The \b OS-HAL \b FreeRTOS \b Driver \b Sample \b Code \b as \b
 * Following.
 * \n
 *  - sample code:
 *    @code
 *      #include "os_hal_dma.h"
 *
 *      #define DMA_BASE 0x21080000
 *
 *      #define DMA_CHANNEL_MAX (VDMA_ADC_RX_CH29 + 1)
 *
 *      // brief options for DMA status
 *      enum dma_sta {
 *	       IDLE,	// DMA status as idle state
 *	       RUNNING, // DMA status as running state
 *	       STOP,	// DMA status as stop state
 *	       PAUSE,	// DMA status as pause state
 *      };
 *
 *      struct dma_controller_rtos {
 *	       enum dma_sta status;
 *	       struct dma_interrupt interrupt_1;
 *	       struct dma_interrupt interrupt_2;
 *	       struct dma_controller *ctlr;
 *      };
 *
 *      static struct dma_controller_rtos g_dma_ctlr_rtos[DMA_CHANNEL_MAX];
 *      static struct dma_controller g_dma_ctlr[DMA_CHANNEL_MAX];
 *      static struct dma_ctrl g_dma_ctrl_mode[DMA_CHANNEL_MAX];
 *      static struct dma_config g_dma_config[DMA_CHANNEL_MAX];
 *
 *      static inline enum dma_type _mtk_os_hal_dma_get_chn_type(u8 chn)
 *      {
 *	       if (chn <= DMA_ISU4_RX_CH9)
 *		       return DMA_TYPE_HALFSIZE;
 *	       else if (chn == DMA_M2M_CH12)
 *		       return DMA_TYPE_FULLSIZE;
 *	       else if (chn >= VDMA_ISU0_TX_CH13 && chn <= VDMA_ISU4_RX_CH22)
 *		       return DMA_TYPE_VFF;
 *	       else if (chn >= VDMA_I2S0_TX_CH25 && chn <= VDMA_ADC_RX_CH29)
 *		       return DMA_TYPE_VFF;
 *	       else
 *		       return (enum dma_type)-1;
 *      }
 *
 *      static struct dma_controller_rtos *_mtk_os_hal_dma_get_ctlr(u8 chn)
 *      {
 *	       if (_mtk_os_hal_dma_get_chn_type(chn) == (enum dma_type)-1)
 *		       return NULL;
 *	       return &g_dma_ctlr_rtos[chn];
 *      }
 *
 *      static void _mtk_os_hal_dma_irq_handler(void)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *	       int chn;
 *	       int status;
 *
 *	       NVIC_DisableIRQ((IRQn_Type)CM4_IRQ_M4DMA);
 *
 *	       for (chn = 0; chn < DMA_CHANNEL_MAX; chn++) {
 *		       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *		       if (ctrl_rtos == NULL || ctrl_rtos->ctlr == NULL)
 *			       continue;
 *		       status = mtk_mhal_dma_get_status(ctrl_rtos->ctlr);
 *		       if (status & DMA_STATUS_INTERRUPT) {
 *			       mtk_mhal_dma_clear_irq_status(ctrl_rtos->ctlr);
 *		       }
 *	       }
 *
 *	       NVIC_EnableIRQ((IRQn_Type)CM4_IRQ_M4DMA);
 *      }
 *
 *      static int _mtk_os_hal_dma_done_callback_1(void *data)
 *      {
 *	       struct dma_controller_rtos *ctlr_rtos = data;
 *
 *	       if (ctlr_rtos->interrupt_1.isr_cb != NULL)
 *		       ctlr_rtos->interrupt_1.isr_cb(
 *		       ctlr_rtos->interrupt_1.cb_data);
 *
 *	       return 0;
 *      }
 *
 *      static int _mtk_os_hal_dma_done_callback_2(void *data)
 *      {
 *	       struct dma_controller_rtos *ctlr_rtos = data;
 *
 *	       if (ctlr_rtos->interrupt_2.isr_cb != NULL)
 *		       ctlr_rtos->interrupt_2.isr_cb(
 *		       ctlr_rtos->interrupt_2.cb_data);
 *
 *	       return 0;
 *      }
 *
 *      static void _mtk_os_hal_dma_reset_controller_rtos(
 *	       struct dma_controller_rtos *ctrl_rtos)
 *      {
 *	       if (ctrl_rtos->ctlr != NULL) {
 *		       if (ctrl_rtos->ctlr->cfg != NULL) {
 *			       memset(ctrl_rtos->ctlr->cfg, 0x0,
 *				       sizeof(*(ctrl_rtos->ctlr->cfg)));
 *			       ctrl_rtos->ctlr->cfg = NULL;
 *		       }
 *
 *		       if (ctrl_rtos->ctlr->ctrls != NULL) {
 *			       memset(ctrl_rtos->ctlr->ctrls, 0x0,
 *				       sizeof(*(ctrl_rtos->ctlr->ctrls)));
 *			       ctrl_rtos->ctlr->ctrls = NULL;
 *		       }
 *		       memset(ctrl_rtos->ctlr, 0x0,
 *			       sizeof(*(ctrl_rtos->ctlr)));
 *		       ctrl_rtos->ctlr = NULL;
 *	       }
 *
 *	       ctrl_rtos->interrupt_1.isr_cb = NULL;
 *	       ctrl_rtos->interrupt_1.cb_data = NULL;
 *	       ctrl_rtos->interrupt_2.isr_cb = NULL;
 *	       ctrl_rtos->interrupt_2.cb_data = NULL;
 *	       ctrl_rtos->status = IDLE;
 *      }
 *
 *      int mtk_os_hal_dma_alloc_chan(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *	       int ret = 0;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *	       if (ctrl_rtos->status == IDLE) {
 *		       if (ctrl_rtos->ctlr != NULL)
 *			       _mtk_os_hal_dma_reset_controller_rtos(ctrl_rtos);
 *
 *		       ctrl_rtos->ctlr = &g_dma_ctlr[chn];
 *		       ctrl_rtos->ctlr->ctrls = &g_dma_ctrl_mode[chn];
 *		       ctrl_rtos->ctlr->cfg = &g_dma_config[chn];
 *
 *		       ctrl_rtos->ctlr->base = (void __iomem *)DMA_BASE;
 *		       ctrl_rtos->ctlr->chn = chn;
 *		       ctrl_rtos->ctlr->chn_type =
 *			       _mtk_os_hal_dma_get_chn_type(chn);
 *		       if (ctrl_rtos->ctlr->chn_type == -1) {
 *			       printf("invalid dma channel number: %d\n", chn);
 *			       return -EPARAM;
 *		       }
 *
 *		       mtk_mhal_dma_clock_enable(ctrl_rtos->ctlr);
 *
 *		       CM4_Install_NVIC(CM4_IRQ_M4DMA, CM4_DMA_PRI,
 *		       IRQ_LEVEL_TRIGGER, _mtk_os_hal_dma_irq_handler, TRUE);
 *
 *		       ctrl_rtos->status = RUNNING;
 *	       } else {
 *		       printf("chn %d is being used\n", chn);
 *		       if (ctrl_rtos->ctlr != NULL) {
 *			       printf("chn(%d) status(%d)\n",
 *			       ctrl_rtos->ctlr->chn, ctrl_rtos->status);
 *		       }
 *		       ret = -EBUSY;
 *	       }
 *	       return ret;
 *      }
 *
 *      static void _mtk_os_hal_dma_set_control_mode(
 *		       struct dma_controller *ctrl,
 *		       struct dma_control_mode *ctrl_mode)
 *      {
 *	       ctrl->ctrls->burst_type =
 *			(enum dma_burst_type)ctrl_mode->burst_type;
 *	       ctrl->ctrls->bw_transfer_en = ctrl_mode->bw_transfer_en;
 *	       ctrl->ctrls->transize = (enum dma_transize)ctrl_mode->transize;
 *
 *	       ctrl->ctrls->wrap_en = ctrl_mode->wrap_settings.wrap_en;
 *	       ctrl->ctrls->wrap_side = ctrl_mode->wrap_settings.wrap_side;
 *	       ctrl->cfg->bw_limiter = ctrl_mode->bw_limiter;
 *	       ctrl->cfg->wrap_point = ctrl_mode->wrap_settings.wrap_point;
 *	       ctrl->cfg->wrap_to_addr = ctrl_mode->wrap_settings.wrap_to_addr;
 *      }
 *
 *      int mtk_os_hal_dma_config(u8 chn, struct dma_setting *setting)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *	       struct dma_controller *ctrl;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       ctrl = ctrl_rtos->ctlr;
 *	       if (ctrl == NULL) {
 *		       printf("dma channel has not been allocated!\n");
 *		       return -EPTR;
 *	       }
 *
 *	       _mtk_os_hal_dma_set_control_mode(ctrl, &(setting->ctrl_mode));
 *
 *	       if (ctrl->chn_type == DMA_TYPE_FULLSIZE) {
 *		       ctrl->cfg->addr_1 = setting->src_addr;
 *		       ctrl->cfg->addr_2 = setting->dst_addr;
 *		       ctrl->cfg->count = setting->count;
 *		       ctrl->ctrls->src_inc_en = 1;
 *		       ctrl->ctrls->dst_inc_en = 1;
 *		       ctrl->ctrls->int_en = !!(setting->interrupt_flag
 *			       & DMA_INT_COMPLETION);
 *	       } else {
 *		       ctrl->ctrls->dir = (enum dma_dir)setting->dir;
 *		       if (ctrl->ctrls->dir == MEM_2_PERI) {
 *			       ctrl->cfg->addr_1 = setting->src_addr;
 *			       ctrl->cfg->addr_2 = setting->dst_addr;
 *			       ctrl->ctrls->src_inc_en = 1;
 *			       ctrl->ctrls->dst_inc_en = 0;
 *		       } else {
 *			       ctrl->cfg->addr_1 = setting->dst_addr;
 *			       ctrl->cfg->addr_2 = setting->src_addr;
 *			       ctrl->ctrls->src_inc_en = 0;
 *			       ctrl->ctrls->dst_inc_en = 1;
 *		       }
 *		       ctrl->cfg->count = setting->count;
 *		       ctrl->cfg->reload_en = setting->reload_en;
 *		       if (ctrl->chn_type == DMA_TYPE_VFF) {
 *			       ctrl->cfg->count = setting->vfifo.fifo_thrsh;
 *			       ctrl->cfg->timeout_cnt =
 *					       setting->vfifo.timeout_cnt;
 *			       ctrl->cfg->alert_cmp_type =
 *				   setting->vfifo.alert_cmp_type;
 *			       ctrl->cfg->alert_len = setting->vfifo.alert_len;
 *			       ctrl->cfg->fifo_size = setting->vfifo.fifo_size;
 *			       ctrl->ctrls->int_en = !!(setting->interrupt_flag
 *				       & DMA_INT_VFIFO_THRESHOLD);
 *			       ctrl->ctrls->timeout_int_en =
 *		       !!(setting->interrupt_flag & DMA_INT_VFIFO_TIMEOUT);
 *		       } else {
 *			       ctrl->ctrls->int_en = !!(setting->interrupt_flag
 *				       & DMA_INT_COMPLETION);
 *			       ctrl->ctrls->half_int_en =
 *		       !!(setting->interrupt_flag & DMA_INT_HALF_COMPLETION);
 *		       }
 *	       }
 *
 *	       ctrl->cfg->isr_callback_1 =
 *		   (dma_isr_callback)_mtk_os_hal_dma_done_callback_1;
 *	       ctrl->cfg->isr_cb_data_1 = ctrl_rtos;
 *	       ctrl->cfg->isr_callback_2 =
 *		   (dma_isr_callback)_mtk_os_hal_dma_done_callback_2;
 *	       ctrl->cfg->isr_cb_data_2 = ctrl_rtos;
 *
 *	       return mtk_mhal_dma_config(ctrl);
 *      }
 *
 *      int mtk_os_hal_dma_start(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_start(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_stop(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *	       ctrl_rtos->status = IDLE;
 *
 *	       return mtk_mhal_dma_stop(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_pause(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_pause(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_resume(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_resume(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_get_status(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_get_status(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_register_isr(u8 chn,
 *		       dma_interrupt_callback isr_cb,
 *		       void *cb_data, enum dma_interrupt_type *isr_type)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       switch (isr_type) {
 *	       case DMA_INT_COMPLETION:
 *		       ctrl_rtos->interrupt_1.isr_cb = isr_cb;
 *		       ctrl_rtos->interrupt_1.cb_data = cb_data;
 *		       break;
 *	       case DMA_INT_HALF_COMPLETION:
 *		       ctrl_rtos->interrupt_2.isr_cb = isr_cb;
 *		       ctrl_rtos->interrupt_2.cb_data = cb_data;
 *		       break;
 *	       case DMA_INT_VFIFO_THRESHOLD:
 *		       ctrl_rtos->interrupt_1.isr_cb = isr_cb;
 *		       ctrl_rtos->interrupt_1.cb_data = cb_data;
 *		       break;
 *	       case DMA_INT_VFIFO_TIMEOUT:
 *		       ctrl_rtos->interrupt_2.isr_cb = isr_cb;
 *		       ctrl_rtos->interrupt_2.cb_data = cb_data;
 *		       break;
 *	       default:
 *		       printf("unknown dma isr_type %d\n", isr_type);
 *		       return -EPARAM;
 *	       }
 *
 *	       return 0;
 *      }
 *
 *      int mtk_os_hal_dma_dump_register(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_dump_reg(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_set_param(u8 chn, enum dma_param_type param_type,
 *				    u32 value)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_set_param(ctrl_rtos->ctlr,
 *			       (enum dma_param)param_type, value);
 *      }
 *
 *      int mtk_os_hal_dma_get_param(u8 chn, enum dma_param_type param_type)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos = NULL;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_get_param(ctrl_rtos->ctlr,
 *					     (enum dma_param)param_type);
 *      }
 *
 *      int mtk_os_hal_dma_update_swptr(u8 chn, u32 length_byte)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos = NULL;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_update_swptr(ctrl_rtos->ctlr, length_byte);
 *      }
 *
 *      int mtk_os_hal_dma_vff_read_data(u8 chn, u8 *buffer, u32 length)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos = NULL;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_vff_read_data(ctrl_rtos->ctlr, buffer,
 *					       length);
 *      }
 *
 *      int mtk_os_hal_dma_release_chan(u8 chn)
 *      {
 *	       int ret;
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       ret = mtk_mhal_dma_stop(ctrl_rtos->ctlr);
 *	       if (ret) {
 *		       printf("stop dma fail!\n");
 *		       return ret;
 *	       }
 *
 *	       mtk_mhal_dma_clock_disable(ctrl_rtos->ctlr);
 *
 *	       _mtk_os_hal_dma_reset_controller_rtos(ctrl_rtos);
 *
 *	       return ret;
 *      }
 *
 *      int mtk_os_hal_dma_reset(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *	       ctrl_rtos->status = IDLE;
 *
 *	       return mtk_mhal_dma_reset(ctrl_rtos->ctlr);
 *      }
 *
 *      int mtk_os_hal_dma_clr_dreq(u8 chn)
 *      {
 *	       struct dma_controller_rtos *ctrl_rtos;
 *
 *	       ctrl_rtos = _mtk_os_hal_dma_get_ctlr(chn);
 *	       if (ctrl_rtos == NULL)
 *		       return -EPTR;
 *
 *	       return mtk_mhal_dma_clr_dreq(ctrl_rtos->ctlr);
 *      }
 *
 *    @endcode
 *
 * - \b Device \b driver \b sample \b code \b is \b as \b follows: \n
 *  - sample code (this is the user application sample code on freeRTos):
 *    @code
 *	//the sample code show the usage of FULL-SIZE DMA to copy data from
 *	//source buffer(src_phy_addr) to destination buffer(dst_phy_addr).
 *	- Define the callback function for DMA transfer completion interrupt.
 *	  The callback function example is as follows:
 *         static int done_flag;
 *         void dma_done_callback(void *user_data)
 *         {
 *             done_flag = 1;
 *         }
 *	- Allcoate DMA channel resource and config DMA channel:
 *	  -Call mtk_os_hal_dma_alloc_chan(DMA_M2M_CH12) to allocate DMA channel
 *	    resource.
 *	  -Use struct dma_setting to store data transfer parameters. The example
 *	    code is as follows:
 *            struct dma_setting setting;
 *            memset(&setting, 0x0, sizeof(struct dma_setting));
 *            setting.count = byte_count;
 *            setting.src_addr = (u32)src_phy_addr;
 *            setting.dst_addr = (u32)dst_phy_addr;
 *            //enable the increment flag of source and destination address.
 *            setting.ctrl_mode.src_inc_en = 1;
 *            setting.ctrl_mode.dst_inc_en = 1;
 *            //enable transfer completion interrupt flag.
 *            setting.interrupt_flag = DMA_INT_COMPLETION;
 *	  -Call mtk_os_hal_dma_config(DMA_M2M_CH12, &setting) to config dma
 *	    channel.
 *	- Register the callback function for the DMA transfer completion
 *	   interrupt.
 *        -Call mtk_os_hal_dma_register_isr(DMA_M2M_CH12, dma_done_callback,
 *	    NULL, DMA_INT_COMPLETION) to register callback function.
 *	- Call mtk_os_hal_dma_start(DMA_M2M_CH12) to start DMA transfer.
 *	- Wait DMA transfer completion interrupt. The example code is as
 *	   follows:
 *	     while (!done_flag) {
 *	        int status = mtk_os_hal_dma_get_status(DMA_M2M_CH12);
 *	        printf("mtk_os_hal_dma_get_status %d\n", status);
 *	     }
 *	- Stop DMA channel and release DMA channel resource.
 *	  -Call mtk_os_hal_dma_stop(DMA_M2M_CH12) to stop dma channel after
 *	    transfer completion.
 *	  -Call mtk_os_hal_dma_release_chan(DMA_M2M_CH12) to release dma channel
 *	    resource.
 *
 *    @endcode
 *
 * @}
 * @}
 */

/**
 * @addtogroup M-HAL
 * @{
 * @addtogroup DMA
 * @{
 *
 */

/** @defgroup driver_dma_define Define
 * @{
 * This section introduces the Macro definition
 * which is used as DMA M-HAL's API error return type.
 */

/** Invalid argument, it means the pointer is NULL. */
#define DMA_EPTR 1
/** Invalid parameter, it means the users input error parameters. */
#define DMA_EPARAM 2
/** DMA channel is busy. */
#define DMA_EBUSY 3

/**
 * @}
 */

/** @defgroup driver_dma_enum Enum
 * @{
 * This section describes the enumeration definition in DMA device driver.
 * These enumerations include data transfer direction, DMA channel type, and
 * DMA parameters type.\n If users want to set the value for the above
 * variables, please refer to Enumerations.
 */

/** @brief DMA data transfer direction definition.
 * This definition is only for HALF-SIZE DMA and VFF DMA,
 * and users should set the data direction value before
 * starting DMA hardware.
 */
enum dma_dir {
	/** move data from memory to peripheral */
	MEM_2_PERI = 0,
	/** move data from peripheral to memory */
	PERI_2_MEM = 1
};

/** @brief DMA channel type definition.
 * DMA hardware has multiple channels and every channel
 * can work separately. According to hardware features, DMA
 * hardware has three different channels: FULL-SIZE DMA,
 * HALF-SIZE DMA and Virtual FIFO DMA.
 */
enum dma_type {
	/** FULL-SIZE DMA */
	DMA_TYPE_FULLSIZE = 0,
	/** HALF-SIZE DMA */
	DMA_TYPE_HALFSIZE = 1,
	/** Virtual FIFO DMA */
	DMA_TYPE_VFF = 2,
};

/** @brief DMA parameter-type definition. Sometimes users
 * want to set or get some parameters of DMA channel for
 * interaction with DMA channel during data transfer.
 */
enum dma_param {
	/** remaining length of current transfer,
	 * only for FULL-SIZE DMA and HALF-SIZE DMA.
	 */
	DMA_PARAM_RLCT = 0,
	/** fix_addr specifies the address for peripheral;
	 * must be 4-Byte alignment; only for HALF-SIZE DMA
	 * and VFF DMA.
	 */
	DMA_PARAM_FIX_ADDR = 1,
	/** prog_addr specifies the memory space address;
	 * only for HALF-SIZE DMA and VFF DMA.
	 */
	DMX_PARAM_PROG_ADDR = 2,
	/** FIFO size, only for VFF DMA */
	DMA_PARAM_VFF_FIFO_SIZE = 3,
	/** Data count of FIFO, only for VFF DMA */
	DMA_PARAM_VFF_FIFO_CNT = 4,
	/** Hardware pointer,  only for VFF DMA */
	DMA_PARAM_VFF_HWPTR = 5,
	/** Software pointer, only for VFF DMA */
	DMA_PARAM_VFF_SWPTR = 6,
};

/** @brief DMA dma_status definition. DMA device has registers to check the
 * status of every DMA channel, including interrupt flag, running flag and
 * pause flag.
 */
enum dma_status {
	/** The interrupt of DMA channel is pending and waitting for service. */
	DMA_STATUS_INTERRUPT = 1U << 0,
	/** The DMA channel is currently running. */
	DMA_STATUS_RUNNING = 1U << 1,
	/** The DMA channel is paused. */
	DMA_STATUS_PAUSE = 1U << 2,
};

/** @brief DMA dma_transize definition. The transaction size indicate transfer
 * data bytes in one bus cycle.
 */
enum dma_transize {
	/** Select DMA transaction size as Byte (1-byte) */
	DMA_SIZE_BYTE = 0,
	/** Select DMA transaction size as Short (2-byte) */
	DMA_SIZE_SHORT,
	/** Select DMA transaction size as Long (4-byte) */
	DMA_SIZE_LONG,
};

/** @brief DMA dma_burst_type definition. The burst type indicate the number of
 * continuous data transfer in a burst. Note that burst transfer does not stop
 * until all of the beats in a burst are completed or transfer length is
 * reached.\n If burst type is not DMA_BURST_TYPE_SINGLE, the FIFO threshold of
 * peripherals must be configured carefully.\n The selection of burst type is
 * restricted by the #dma_transize.
 */
enum dma_burst_type {
	/** Select DMA burst type as single-beat */
	DMA_BURST_TYPE_SINGLE = 0x0,
	/** Select DMA burst type as 4-beat */
	DMA_BURST_TYPE_4BEAT = 0x2,
	/** Select DMA burst type as 8-beat */
	DMA_BURST_TYPE_8BEAT = 0x4,
	/** Select DMA burst type as 16-beat */
	DMA_BURST_TYPE_16BEAT = 0x6
};

/**
 * @}
 */

/** @defgroup driver_dma_typedef Typedef
 * @{
 * This section introduces the typedef used by DMA M-HAL.
 */

/** @brief This defines the function prototype of DMA interrupt service
 * routines callback.
 * Different DMA channels have different interrupts. FULL-SIZE DMA has data
 * transfer completion interrupt;\n HALF-SIZE DMA has data transfer completion
 * interrupt and half-completion interrupt;\n VFF DMA has FIFO data overflow/
 * underflow interrupt and FIFO timeout interrupt.\n
 * dma_isr_callback will be called in the DMA interrupt service routine and
 * it can notify users that corresponding HW event occurs.\n
 * This typedef is to describe the callback indicating what users want to do.
 *
 * @param [in] user_data is a OS-HAL defined parameter provided
 * by #mtk_mhal_dma_config().
 * @sa  #mtk_mhal_dma_config()
 */
typedef void (*dma_isr_callback)(void *user_data);

/**
 * @}
 */

/** @defgroup driver_dma_struct Struct
 * @{
 * This section introduces the structure definition in DMA M-HAL. When
 * users want to call DMA M-HAL API, these structures should be allocated
 * and filled by right values.
 */

/** @brief dma_ctrl specifies the DMA channel settings which control the DMA
 * channel transfer mode. Users fill the field of dma_ctrl according to the DMA
 * channel type and transfer mode.
 */
struct dma_ctrl {
	/** Transfer direction, only for HALF-SIZE DMA
	 * and VFF DMA. Refer to #dma_dir.
	 */
	enum dma_dir dir;
	/** Flag of transfer address-wrapping feature. If it is enabled,
	 * wrap_point and wrap_to_addr need to be set. Refer to struct
	 * dma_config.\n Only for FULL-SIZE DMA and HALF-SIZE DMA.
	 */
	u8 wrap_en;
	/** The side for using address-wrapping: source or destination, and
	 * only one side can activate address-wrapping feature at a time.\n
	 * Only for FULL-SIZE DMA and HALF-SIZE DMA.
	 */
	u8 wrap_side;
	/** Enable transfer completion interrupt for FULL-SIZE
	 * DMA and HALF-SIZE DMA, or enable FIFO data interrupt
	 * for VFF DMA.
	 */
	u8 int_en;
	/** Enable FIFO data timeout interrupt for VFF DMA*/
	u8 timeout_int_en;
	/** Enable half transfer completion interrupt for HALF-SIZE DMA*/
	u8 half_int_en;
	/** Burst-type, only for FULL-SIZE DMA and HALF-SIZE DMA.
	 * Refer to #dma_burst_type.
	 */
	enum dma_burst_type burst_type;
	/** Word to byte or byte to word transfer flag, only for
	 * HALF-SIZE DMA.
	 */
	u8 bw_transfer_en;
	/** Throttle and handshake control for DMA Transfer between
	 * DMA and Peripheral
	 */
	u8 dreq;
	/** Destination addresses automatically increase after every transfer,
	 * only for FULL-SIZE DMA and HALF-SIZE DMA.
	 */
	u8 dst_inc_en;
	/** Source addresses automatically increase after every transfer,
	 * only for FULL-SIZE DMA and HALF-SIZE DMA.
	 */
	u8 src_inc_en;
	/** Data size within the confine of a bus cycle per transfer.
	 * Refer to #dma_transize.
	 */
	enum dma_transize transize;
};

/** @brief dma_config specifies the DMA channel parameters for data
 * transfer, such as transfer source, transfer destination and transfer
 * count.\n Users fill the field of dma_config according to the DMA channel
 * type.
 */
struct dma_config {
	/** For FULL-SIZE DMA, addr_1 is src_addr. For HALF-SIZE
	 *  DMA and VFF DMA, addr_1 is prog_addr.
	 */
	u32 addr_1;
	/** For FULL-SIZE DMA, addr_2 is dst_addr. For HALF-SIZE
	 *  DMA and VFF DMA, addr_2 is fix_addr.
	 */
	u32 addr_2;
	/** Wrap point from start address. If wrap_en in dma_ctrl is enable, DMA
	 * channel read/write pointer will jump to #wrap_to_addr at #wrap_point
	 * during data transfer.\n Only for FULL-SIZE DMA and HALF-SIZE DMA.
	 */
	u32 wrap_point;
	/** The address which DMA will wrap to. If wrap_en in dma_ctrl is
	 * enable, DMA channel read/write pointer will jump to #wrap_to_addr
	 * at #wrap_point during data transfer.\n Only for FULL-SIZE DMA and
	 * HALF-SIZE DMA.
	 */
	u32 wrap_to_addr;
	/** Reload transfer count flag, only for HALF-SIZE DMA */
	u8 reload_en;
	/** For FULL-SIZE DMA and HALF-SIZE DMA, it specifies transfer
	 * byte count. FOR VFF DMA, it specifies the threshold for FIFO
	 * data interrupt.
	 */
	u32 count;
	/** Bandwidth limiter. The value range is from 0 to 255. */
	u8 bw_limiter;
	/** Specifies the Compare Equation Between ALTLEN and
	 * FIFO_SIZE - FIFO_CNT
	 */
	u8 alert_cmp_type;
	/** Specifies the Alert Length of Virtual FIFO DMA */
	u32 alert_len;
	/** Virtual fifo size, only for VFF DMA */
	u32 fifo_size;
	/** Interrupt will assert if there is no new data into FIFO more
	 * than n T(Bus Clock)
	 */
	u32 timeout_cnt;
	/** For FULL-SIZE DMA and HALF-SIZE DMA, isr_callback_1 is completion
	 * isr callback. For VFF DMA, isr_callback_1 is FIFO data underflow or
	 * overflow interrupt callback.
	 */
	dma_isr_callback isr_callback_1;
	/** For HALF-SIZE DMA, isr_callback_2 is half completion isr callback.
	 *  For VFF DMA, isr_callback_2 is FIFO DMA timeout interrupt callback.
	 */
	dma_isr_callback isr_callback_2;
	/** isr_callback_1 data */
	void *isr_cb_data_1;
	/** isr_callback_2 data */
	void *isr_cb_data_2;
};

/** @brief Interface to DMA, used to pass arguments between
 * OS_HAL and M-HAL.
 */
struct dma_controller {
	/** DMA base address */
	void __iomem *base;
	/** DMA channel */;
	u8 chn;
	/** DMA channel type */
	enum dma_type chn_type;
	/** DMA control setting */
	struct dma_ctrl *ctrls;
	/** DMA config */
	struct dma_config *cfg;
};

/**
 * @}
 */

/** @defgroup driver_dma_function Function
 * @{
 * This section provides DMA M-HAL APIs(defined as Common Interface)
 * to fully control the MediaTek DMA HW.
 */
/**
 * @brief This function is used to configure one DMA channel.
 * @brief Usage: OS-HAL driver should call it before starting DMA. According
 * to the DMA channel type,\n users should set right dma_ctrl and dma_config
 * value in dma_controller. Stop the DMA channel first\n before calling
 * this function.
 * @param [in] controller : The pointer of struct dma_controller.
 *
 * @return
 * Return 0 if users configure DMA channel successfully.\n
 * Return -#DMA_EPARAM if users input error parameters.\n
 * Return -#DMA_EBUSY if DMA channel is running.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_config(struct dma_controller *controller);

/**
 * @brief This function is used to start one DMA channel.
 * @brief Usage: OS-HAL driver should call it after configuring DMA
 * successfully.
 * @param [in] controller : The pointer of struct dma_controller.
 *
 * @return
 * Return 0 if users start DMA channel successfully.\n
 * Return -#DMA_EBUSY if DMA channel is running.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_start(struct dma_controller *controller);

/**
 * @brief This function is used to stop one DMA channel.
 * @brief Usage: OS-HAL driver should call it after starting DMA.
 * Calling this function will stop transfer immediately and
 * DMA channel will return idle status.
 * @param [in] controller : The pointer of struct dma_controller.
 *
 * @return
 * Return 0 if users stop DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_stop(struct dma_controller *controller);

/**
 * @brief This function is used to pause one DMA channel.
 * @brief Usage: OS-HAL driver should call it after starting DMA.
 * After calling this function, DMA channel will complete
 * last count data transfer and then be into pause status.
 * @param [in] controller : The pointer of struct dma_controller.
 *
 * @return
 * Return 0 if users pause DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_pause(struct dma_controller *controller);

/**
 * @brief This function is used to resume one DMA channel.
 * @brief Usage: OS-HAL driver should call it after pausing DMA.
 * Users call this function to cancel pause status for DMA channel.
 * @param [in] controller : The pointer of struct dma_controller.
 *
 * @return
 * Return 0 if users resume DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_resume(struct dma_controller *controller);

/**
 * @brief This function is used to get DMA channel status.
 * @brief Usage: If OS-HAL driver call MHAL API without DMA callback,
 * it can call this API to get DMA channel status so as to check DMA
 * channel working status.
 * @param [in] controller : The pointer of struct dma_controller.
 *
 * @return
 * Return the DMA channel status if get status successfully.\n
 * Return -#DMA_EPTR if controller is NULL.\n
 * Bit 0 specifies interrupt status, Bit 1 specifies running status and Bit 2
 * specifies pause status. Bit 0 is 1 when interrupt is pending and is 0 when
 * \n no interrupt is generated. Bit 1 is 0 when DMA channel has been stopped
 * or has completed transfer already and is 1 when DMA channel is running.\n
 * Refer to #dma_status.
 */
int mtk_mhal_dma_get_status(struct dma_controller *controller);

/**
 * @brief This function is used to set DMA parameter which is
 * defined in the #dma_param.
 * @brief Usage: Users should call this function carefully if DMA channel
 * is running.
 * @param [in] controller : The pointer of struct dma_controller.
 * @param [in] param : The parameter type whose value to be set.
 * @param [in] value : The new parameter value.
 *
 * @return
 * Return 0 if users set DMA channel parameter successfully.\n
 * Return -#DMA_EPARAM if users input error parameter type or value.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_set_param(struct dma_controller *controller,
			   enum dma_param param, u32 value);

/**
 * @brief This function is used to get DMA parameter which is
 * defined in the #dma_param.
 * @brief Usage: Users should call this function carefully if DMA channel
 * is running.
 * @param [in] controller : The pointer of struct dma_controller.
 * @param [in] param : The parameter type whose value to be got.
 *
 * @return
 * Return the parameter value if users get DMA parameters successfully.\n
 * Return -#DMA_EPARAM if users input error parameter type.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_get_param(struct dma_controller *controller,
			   enum dma_param param);

/**
 * @brief This function is used to clear DMA HW status.
 * @brief Usage: It's called in IRQ function to clear DMA HW IRQ status.
 * @param [in] controller : DMA controller used with the device.
 *
 * @return
 * Return 0 if users resume DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_clear_irq_status(struct dma_controller *controller);

/**
 * @brief This function is used to dump DMA register for debug.
 * @brief Usage:Used for OS-HAL to dump DMA register.
 * @param [in] controller : DMA controller used with the device.
 *
 * @return
 * Return 0 if users resume DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_dump_reg(struct dma_controller *controller);

/**
 * @brief This function is used to enable DMA channel clock.
 * @brief Usage:Used for OS-HAL to enable DMA channel clock before config.
 * @param [in] controller : DMA controller used with the device.
 *
 * @return
 * Return 0 if users enable DMA channel clock successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_clock_enable(struct dma_controller *controller);

/**
 * @brief This function is used to disable DMA channel clock.
 * @brief Usage:Used for OS-HAL to disable DMA channel clock.
 * @param [in] controller : DMA controller used with the device.
 *
 * @return
 * Return 0 if users disable DMA channel clock successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_clock_disable(struct dma_controller *controller);

/**
 * @brief This function is used to reset DMA channel.
 * @brief Usage:Used for OS-HAL to reset DMA channel.
 * @param [in] controller : DMA controller used with the device.
 *
 * @return
 * Return 0 if users reset DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_reset(struct dma_controller *controller);

/**
 * @brief This function is used to update VFF DMA channel swptr.
 * @brief Usage: Used for OS-HAL to update VFF DMA channel swptr.
 * If user read data from vfifo or write data into vfifo, user
 * should update swptr to control read pointer or write pointer
 * of fifo control engine.
 * @param [in] controller : DMA controller used with the device.
 * @param [in] length_byte : The updated byte count. It cannot be
 * larger than vfifo size.
 *
 * @return
 * Return 0 if users update VFF DMA channel swptr successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_update_swptr(struct dma_controller *controller,
					u32 length_byte);

/**
 * @brief This function is used to read data from FIFO of VFF DMA channel.
 * @brief Usage:Used for OS-HAL to read data from FIFO of VFF DMA channel.
 * This api will also update swptr after reading data.
 * @param [in] controller : DMA controller used with the device.
 * @param [in] buffer : buffer for saving read data.
 * @param [in] length : byte number which users want to read. It cannot be
 * larger than vfifo size.
 *
 * @return
 * Return the actual read data byte count if successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_vff_read_data(struct dma_controller *controller,
					u8 *buffer, u32 length);

/**
 * @brief This function is used to clear dreq signal of DMA channel.
 * @brief Usage:Used for OS-HAL to clear dreq signal of DMA channel.
 * @param [in] controller : DMA controller used with the device.
 *
 * @return
 * Return 0 if users clear dreq signal of DMA channel successfully.\n
 * Return -#DMA_EPTR if controller is NULL.
 */
int mtk_mhal_dma_clr_dreq(struct dma_controller *controller);
/**
 * @}
 */

/**
 * @}
 * @}
 */

#endif
