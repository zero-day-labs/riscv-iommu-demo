/*
 * Copyright (c) 2021-2022 ETH Zurich and University of Bologna
 * Copyright and related rights are licensed under the Solderpad Hardware
 * License, Version 0.51 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
 * or agreed to in writing, software, hardware and materials distributed under
 * this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * Author: Andreas Kuster <kustera@ethz.ch>
 * Description: Generated register defines for dma_frontend
 */

#ifndef _DMA_FRONTEND_REG_DEFS_
#define _DMA_FRONTEND_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif

#define DMA_BASE(idx)     (0x50000000 + (idx * 0x1000))  // iDMA

// Register width
#define DMA_FRONTEND_PARAM_REG_WIDTH        64

// Source Address
#define DMA_FRONTEND_SRC_ADDR_REG_OFFSET    0x0
#define DMA_FRONTEND_SRC_ADDR_REG_RESVAL    0x0

// Destination Address
#define DMA_FRONTEND_DST_ADDR_REG_OFFSET    0x8
#define DMA_FRONTEND_DST_ADDR_REG_RESVAL    0x0

// Number of bytes
#define DMA_FRONTEND_NUM_BYTES_REG_OFFSET   0x10
#define DMA_FRONTEND_NUM_BYTES_REG_RESVAL   0x0

// Configuration Register for DMA settings
#define DMA_FRONTEND_CONF_REG_OFFSET        0x18
#define DMA_FRONTEND_CONF_REG_RESVAL        0x0
#define DMA_FRONTEND_CONF_DECOUPLE_BIT      0
#define DMA_FRONTEND_CONF_DEBURST_BIT       1
#define DMA_FRONTEND_CONF_SERIALIZE_BIT     2

// DMA Status
#define DMA_FRONTEND_STATUS_REG_OFFSET      0x20
#define DMA_FRONTEND_STATUS_REG_RESVAL      0x0
#define DMA_FRONTEND_STATUS_BUSY_BIT        0

// Next ID, launches transfer, returns 0 if transfer not set up properly.
#define DMA_FRONTEND_NEXT_ID_REG_OFFSET     0x28
#define DMA_FRONTEND_NEXT_ID_REG_RESVAL     0x0

// Get ID of finished transactions.
#define DMA_FRONTEND_DONE_REG_OFFSET        0x30
#define DMA_FRONTEND_DONE_REG_RESVAL        0x0

// Software-hardware interface register
#define DMA_FRONTEND_INTF_REG_OFFSET        0x38
#define DMA_FRONTEND_INTF_REG_RESVAL        0x0

#define DMA_FRONTEND_INTF_BTNU_BIT          0
#define DMA_FRONTEND_INTF_BTND_BIT          1
#define DMA_FRONTEND_INTF_BTNL_BIT          2
#define DMA_FRONTEND_INTF_BTNR_BIT          3
#define DMA_FRONTEND_INTF_BTNC_BIT          4

#define DMA_FRONTEND_INTF_ALL_BUTTONS_MASK  0x1FULL

// iDMA base physical address
#define DMA_SRC_ADDR(idx)      (DMA_BASE(idx) + DMA_FRONTEND_SRC_ADDR_REG_OFFSET)
#define DMA_DST_ADDR(idx)      (DMA_BASE(idx) + DMA_FRONTEND_DST_ADDR_REG_OFFSET)
#define DMA_NUMBYTES_ADDR(idx) (DMA_BASE(idx) + DMA_FRONTEND_NUM_BYTES_REG_OFFSET)
#define DMA_CONF_ADDR(idx)     (DMA_BASE(idx) + DMA_FRONTEND_CONF_REG_OFFSET)
#define DMA_STATUS_ADDR(idx)   (DMA_BASE(idx) + DMA_FRONTEND_STATUS_REG_OFFSET)
#define DMA_NEXTID_ADDR(idx)   (DMA_BASE(idx) + DMA_FRONTEND_NEXT_ID_REG_OFFSET)
#define DMA_DONE_ADDR(idx)     (DMA_BASE(idx) + DMA_FRONTEND_DONE_REG_OFFSET)
#define DMA_INTF_ADDR(idx)     (DMA_BASE(idx) + DMA_FRONTEND_INTF_REG_OFFSET)

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DMA_FRONTEND_REG_DEFS_
