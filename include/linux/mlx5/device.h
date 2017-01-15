/*
 * Copyright (c) 2013-2015, Mellanox Technologies. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MLX5_DEVICE_H
#define MLX5_DEVICE_H

#include <linux/types.h>
#include <rdma/ib_verbs.h>
#include <linux/mlx5/mlx5_ifc.h>

#if defined(__LITTLE_ENDIAN)
#define MLX5_SET_HOST_ENDIANNESS	0
#elif defined(__BIG_ENDIAN)
#define MLX5_SET_HOST_ENDIANNESS	0x80
#else
#error Host endianness not defined
#endif

/* helper macros */
#define __mlx5_nullp(typ) ((struct mlx5_ifc_##typ##_bits *)0)
#define __mlx5_bit_sz(typ, fld) sizeof(__mlx5_nullp(typ)->fld)
#define __mlx5_bit_off(typ, fld) ((unsigned)(unsigned long)(&(__mlx5_nullp(typ)->fld)))
#define __mlx5_dw_off(typ, fld) (__mlx5_bit_off(typ, fld) / 32)
#define __mlx5_64_off(typ, fld) (__mlx5_bit_off(typ, fld) / 64)
#define __mlx5_dw_bit_off(typ, fld) (32 - __mlx5_bit_sz(typ, fld) - (__mlx5_bit_off(typ, fld) & 0x1f))
#define __mlx5_mask(typ, fld) ((u32)((1ull << __mlx5_bit_sz(typ, fld)) - 1))
#define __mlx5_dw_mask(typ, fld) (__mlx5_mask(typ, fld) << __mlx5_dw_bit_off(typ, fld))
#define __mlx5_st_sz_bits(typ) sizeof(struct mlx5_ifc_##typ##_bits)

#define MLX5_FLD_SZ_BYTES(typ, fld) (__mlx5_bit_sz(typ, fld) / 8)
#define MLX5_ST_SZ_BYTES(typ) (sizeof(struct mlx5_ifc_##typ##_bits) / 8)
#define MLX5_ST_SZ_DW(typ) (sizeof(struct mlx5_ifc_##typ##_bits) / 32)
#define MLX5_ST_SZ_QW(typ) (sizeof(struct mlx5_ifc_##typ##_bits) / 64)
#define MLX5_UN_SZ_BYTES(typ) (sizeof(union mlx5_ifc_##typ##_bits) / 8)
#define MLX5_UN_SZ_DW(typ) (sizeof(union mlx5_ifc_##typ##_bits) / 32)
#define MLX5_BYTE_OFF(typ, fld) (__mlx5_bit_off(typ, fld) / 8)
#define MLX5_ADDR_OF(typ, p, fld) ((char *)(p) + MLX5_BYTE_OFF(typ, fld))

/* insert a value to a struct */
#define MLX5_SET(typ, p, fld, v) do { \
	BUILD_BUG_ON(__mlx5_st_sz_bits(typ) % 32);             \
	*((__be32 *)(p) + __mlx5_dw_off(typ, fld)) = \
	cpu_to_be32((be32_to_cpu(*((__be32 *)(p) + __mlx5_dw_off(typ, fld))) & \
		     (~__mlx5_dw_mask(typ, fld))) | (((v) & __mlx5_mask(typ, fld)) \
		     << __mlx5_dw_bit_off(typ, fld))); \
} while (0)

#define MLX5_SET_TO_ONES(typ, p, fld) do { \
	BUILD_BUG_ON(__mlx5_st_sz_bits(typ) % 32);             \
	*((__be32 *)(p) + __mlx5_dw_off(typ, fld)) = \
	cpu_to_be32((be32_to_cpu(*((__be32 *)(p) + __mlx5_dw_off(typ, fld))) & \
		     (~__mlx5_dw_mask(typ, fld))) | ((__mlx5_mask(typ, fld)) \
		     << __mlx5_dw_bit_off(typ, fld))); \
} while (0)

#define MLX5_GET(typ, p, fld) ((be32_to_cpu(*((__be32 *)(p) +\
__mlx5_dw_off(typ, fld))) >> __mlx5_dw_bit_off(typ, fld)) & \
__mlx5_mask(typ, fld))

#define MLX5_GET_PR(typ, p, fld) ({ \
	u32 ___t = MLX5_GET(typ, p, fld); \
	pr_debug(#fld " = 0x%x\n", ___t); \
	___t; \
})

#define MLX5_SET64(typ, p, fld, v) do { \
	BUILD_BUG_ON(__mlx5_bit_sz(typ, fld) != 64); \
	BUILD_BUG_ON(__mlx5_bit_off(typ, fld) % 64); \
	*((__be64 *)(p) + __mlx5_64_off(typ, fld)) = cpu_to_be64(v); \
} while (0)

#define MLX5_GET64(typ, p, fld) be64_to_cpu(*((__be64 *)(p) + __mlx5_64_off(typ, fld)))

#define MLX5_GET64_PR(typ, p, fld) ({ \
	u64 ___t = MLX5_GET64(typ, p, fld); \
	pr_debug(#fld " = 0x%llx\n", ___t); \
	___t; \
})

/* Big endian getters */
#define MLX5_GET64_BE(typ, p, fld) (*((__be64 *)(p) +\
	__mlx5_64_off(typ, fld)))

#define MLX5_GET_BE(type_t, typ, p, fld) ({				  \
		type_t tmp;						  \
		switch (sizeof(tmp)) {					  \
		case sizeof(u8):					  \
			tmp = (__force type_t)MLX5_GET(typ, p, fld);	  \
			break;						  \
		case sizeof(u16):					  \
			tmp = (__force type_t)cpu_to_be16(MLX5_GET(typ, p, fld)); \
			break;						  \
		case sizeof(u32):					  \
			tmp = (__force type_t)cpu_to_be32(MLX5_GET(typ, p, fld)); \
			break;						  \
		case sizeof(u64):					  \
			tmp = (__force type_t)MLX5_GET64_BE(typ, p, fld); \
			break;						  \
			}						  \
		tmp;							  \
		})
#define MLX5E_IS_FILLER_CQE(cqe)  (be32_to_cpu(cqe->byte_cnt) >> 31)

enum {
	MLX5_MAX_COMMANDS		= 32,
	MLX5_CMD_DATA_BLOCK_SIZE	= 512,
	MLX5_PCI_CMD_XPORT		= 7,
	MLX5_MKEY_BSF_OCTO_SIZE		= 4,
	MLX5_MAX_PSVS			= 4,
};

enum {
	MLX5_EXTENDED_UD_AV		= 0x80000000,
};

enum {
	MLX5_CQ_STATE_ARMED		= 9,
	MLX5_CQ_STATE_ALWAYS_ARMED	= 0xb,
	MLX5_CQ_STATE_FIRED		= 0xa,
};

enum {
	MLX5_CQ_FLAGS_OI	= 2,
};

enum {
	MLX5_STAT_RATE_OFFSET	= 5,
};

enum {
	MLX5_INLINE_SEG = 0x80000000,
};

enum {
	MLX5_HW_START_PADDING = MLX5_INLINE_SEG,
};

enum {
	MLX5_MIN_PKEY_TABLE_SIZE = 128,
	MLX5_MAX_LOG_PKEY_TABLE  = 5,
};

enum {
	MLX5_MKEY_INBOX_PG_ACCESS = 1 << 31
};

enum {
	MLX5_PFAULT_SUBTYPE_WQE = 0,
	MLX5_PFAULT_SUBTYPE_RDMA = 1,
};

enum {
	MLX5_PERM_LOCAL_READ	= 1 << 2,
	MLX5_PERM_LOCAL_WRITE	= 1 << 3,
	MLX5_PERM_REMOTE_READ	= 1 << 4,
	MLX5_PERM_REMOTE_WRITE	= 1 << 5,
	MLX5_PERM_ATOMIC	= 1 << 6,
	MLX5_PERM_UMR_EN	= 1 << 7,
};

enum {
	MLX5_PCIE_CTRL_SMALL_FENCE	= 1 << 0,
	MLX5_PCIE_CTRL_RELAXED_ORDERING	= 1 << 2,
	MLX5_PCIE_CTRL_NO_SNOOP		= 1 << 3,
	MLX5_PCIE_CTRL_TLP_PROCE_EN	= 1 << 6,
	MLX5_PCIE_CTRL_TPH_MASK		= 3 << 4,
};

enum {
	MLX5_ACCESS_MODE_PA	= 0,
	MLX5_ACCESS_MODE_MTT	= 1,
	MLX5_ACCESS_MODE_KLM	= 2
};

enum {
	MLX5_MKEY_REMOTE_INVAL	= 1 << 24,
	MLX5_MKEY_FLAG_SYNC_UMR = 1 << 29,
	MLX5_MKEY_BSF_EN	= 1 << 30,
	MLX5_MKEY_LEN64		= 1 << 31,
};

enum {
	MLX5_EN_RD	= (u64)1,
	MLX5_EN_WR	= (u64)2
};

enum {
	MLX5_BF_REGS_PER_PAGE		= 4,
	MLX5_MAX_UAR_PAGES		= 1 << 8,
	MLX5_NON_FP_BF_REGS_PER_PAGE	= 2,
	MLX5_MAX_UUARS	= MLX5_MAX_UAR_PAGES * MLX5_NON_FP_BF_REGS_PER_PAGE,
};

enum {
	MLX5_MKEY_MASK_LEN		= 1ull << 0,
	MLX5_MKEY_MASK_PAGE_SIZE	= 1ull << 1,
	MLX5_MKEY_MASK_START_ADDR	= 1ull << 6,
	MLX5_MKEY_MASK_PD		= 1ull << 7,
	MLX5_MKEY_MASK_EN_RINVAL	= 1ull << 8,
	MLX5_MKEY_MASK_EN_SIGERR	= 1ull << 9,
	MLX5_MKEY_MASK_BSF_EN		= 1ull << 12,
	MLX5_MKEY_MASK_KEY		= 1ull << 13,
	MLX5_MKEY_MASK_QPN		= 1ull << 14,
	MLX5_MKEY_MASK_LR		= 1ull << 17,
	MLX5_MKEY_MASK_LW		= 1ull << 18,
	MLX5_MKEY_MASK_RR		= 1ull << 19,
	MLX5_MKEY_MASK_RW		= 1ull << 20,
	MLX5_MKEY_MASK_A		= 1ull << 21,
	MLX5_MKEY_MASK_SMALL_FENCE	= 1ull << 23,
	MLX5_MKEY_MASK_FREE		= 1ull << 29,
};

enum {
	MLX5_UMR_TRANSLATION_OFFSET_EN	= (1 << 4),

	MLX5_UMR_CHECK_NOT_FREE		= (1 << 5),
	MLX5_UMR_CHECK_FREE		= (2 << 5),

	MLX5_UMR_INLINE			= (1 << 7),
};

enum {
	MLX5_AN_UNAVAILABLE = 0,
	MLX5_AN_COMPLETE    = 1,
	MLX5_AN_FAILED      = 2,
	MLX5_AN_LINK_UP     = 3,
	MLX5_AN_LINK_DOWN   = 4,
};

#define MLX5_UMR_MTT_ALIGNMENT 0x40
#define MLX5_UMR_MTT_MASK      (MLX5_UMR_MTT_ALIGNMENT - 1)
#define MLX5_UMR_MTT_MIN_CHUNK_SIZE MLX5_UMR_MTT_ALIGNMENT

enum {
	MLX5_EVENT_QUEUE_TYPE_QP = 0,
	MLX5_EVENT_QUEUE_TYPE_RQ = 1,
	MLX5_EVENT_QUEUE_TYPE_SQ = 2,
};

enum mlx5_event {
	MLX5_EVENT_TYPE_COMP		   = 0x0,

	MLX5_EVENT_TYPE_PATH_MIG	   = 0x01,
	MLX5_EVENT_TYPE_COMM_EST	   = 0x02,
	MLX5_EVENT_TYPE_SQ_DRAINED	   = 0x03,
	MLX5_EVENT_TYPE_SRQ_LAST_WQE	   = 0x13,
	MLX5_EVENT_TYPE_SRQ_RQ_LIMIT	   = 0x14,

	MLX5_EVENT_TYPE_CQ_ERROR	   = 0x04,
	MLX5_EVENT_TYPE_WQ_CATAS_ERROR	   = 0x05,
	MLX5_EVENT_TYPE_PATH_MIG_FAILED	   = 0x07,
	MLX5_EVENT_TYPE_WQ_INVAL_REQ_ERROR = 0x10,
	MLX5_EVENT_TYPE_WQ_ACCESS_ERROR	   = 0x11,
	MLX5_EVENT_TYPE_SRQ_CATAS_ERROR	   = 0x12,

	MLX5_EVENT_TYPE_INTERNAL_ERROR	   = 0x08,
	MLX5_EVENT_TYPE_PORT_CHANGE	   = 0x09,
	MLX5_EVENT_TYPE_GPIO_EVENT	   = 0x15,
	MLX5_EVENT_TYPE_PORT_MODULE_EVENT  = 0x16,
	MLX5_EVENT_TYPE_REMOTE_CONFIG	   = 0x19,

	MLX5_EVENT_TYPE_DB_BF_CONGESTION   = 0x1a,
	MLX5_EVENT_TYPE_STALL_EVENT	   = 0x1b,

	MLX5_EVENT_TYPE_CMD		   = 0x0a,
	MLX5_EVENT_TYPE_PAGE_REQUEST	   = 0xb,

	MLX5_EVENT_TYPE_PAGE_FAULT	   = 0xc,
	MLX5_EVENT_TYPE_NIC_VPORT_CHANGE   = 0xd,

	MLX5_EVENT_TYPE_DCT_DRAINED	   = 0x1c,
	MLX5_EVENT_TYPE_DCT_KEY_VIOLATION  = 0x1d,
};

enum {
	MLX5_PORT_CHANGE_SUBTYPE_DOWN		= 1,
	MLX5_PORT_CHANGE_SUBTYPE_ACTIVE		= 4,
	MLX5_PORT_CHANGE_SUBTYPE_INITIALIZED	= 5,
	MLX5_PORT_CHANGE_SUBTYPE_LID		= 6,
	MLX5_PORT_CHANGE_SUBTYPE_PKEY		= 7,
	MLX5_PORT_CHANGE_SUBTYPE_GUID		= 8,
	MLX5_PORT_CHANGE_SUBTYPE_CLIENT_REREG	= 9,
};

enum {
	MLX5_MAX_INLINE_RECEIVE_SIZE		= 64
};

enum {
	MLX5_DEV_CAP_FLAG_XRC		= 1LL <<  3,
	MLX5_DEV_CAP_FLAG_BAD_PKEY_CNTR	= 1LL <<  8,
	MLX5_DEV_CAP_FLAG_BAD_QKEY_CNTR	= 1LL <<  9,
	MLX5_DEV_CAP_FLAG_APM		= 1LL << 17,
	MLX5_DEV_CAP_FLAG_BLOCK_MCAST	= 1LL << 23,
	MLX5_DEV_CAP_FLAG_ON_DMND_PG	= 1LL << 24,
	MLX5_DEV_CAP_FLAG_CQ_MODER	= 1LL << 29,
	MLX5_DEV_CAP_FLAG_RESIZE_CQ	= 1LL << 30,
	MLX5_DEV_CAP_FLAG_ATOMIC	= 1LL << 33,
	MLX5_DEV_CAP_FLAG_ROCE          = 1LL << 34,
	MLX5_DEV_CAP_FLAG_DCT		= 1LL << 37,
	MLX5_DEV_CAP_FLAG_SIG_HAND_OVER	= 1LL << 40,
	MLX5_DEV_CAP_FLAG_CMDIF_CSUM	= 3LL << 46,
	MLX5_DEV_CAP_FLAG_DRAIN_SIGERR	= 1LL << 48,
};

enum {
	MLX5_ROCE_VERSION_1		= 0,
	MLX5_ROCE_VERSION_1_5		= 1,
	MLX5_ROCE_VERSION_2		= 2,
};

enum {
	MLX5_ROCE_VERSION_1_CAP		= 1 << MLX5_ROCE_VERSION_1,
	MLX5_ROCE_VERSION_1_5_CAP	= 1 << MLX5_ROCE_VERSION_1_5,
	MLX5_ROCE_VERSION_2_CAP		= 1 << MLX5_ROCE_VERSION_2,
};

enum {
	MLX5_ROCE_L3_TYPE_IPV4		= 0,
	MLX5_ROCE_L3_TYPE_IPV6		= 1,
};

enum {
	MLX5_ROCE_L3_TYPE_IPV4_CAP	= 1 << 1,
	MLX5_ROCE_L3_TYPE_IPV6_CAP	= 1 << 2,
};

enum {
	MLX5_OPCODE_NOP			= 0x00,
	MLX5_OPCODE_SEND_INVAL		= 0x01,
	MLX5_OPCODE_RDMA_WRITE		= 0x08,
	MLX5_OPCODE_RDMA_WRITE_IMM	= 0x09,
	MLX5_OPCODE_SEND		= 0x0a,
	MLX5_OPCODE_SEND_IMM		= 0x0b,
	MLX5_OPCODE_LSO			= 0x0e,
	MLX5_OPCODE_RDMA_READ		= 0x10,
	MLX5_OPCODE_ATOMIC_CS		= 0x11,
	MLX5_OPCODE_ATOMIC_FA		= 0x12,
	MLX5_OPCODE_ATOMIC_MASKED_CS	= 0x14,
	MLX5_OPCODE_ATOMIC_MASKED_FA	= 0x15,
	MLX5_OPCODE_BIND_MW		= 0x18,
	MLX5_OPCODE_CONFIG_CMD		= 0x1f,

	MLX5_RECV_OPCODE_RDMA_WRITE_IMM	= 0x00,
	MLX5_RECV_OPCODE_SEND		= 0x01,
	MLX5_RECV_OPCODE_SEND_IMM	= 0x02,
	MLX5_RECV_OPCODE_SEND_INVAL	= 0x03,

	MLX5_CQE_OPCODE_ERROR		= 0x1e,
	MLX5_CQE_OPCODE_RESIZE		= 0x16,

	MLX5_OPCODE_SET_PSV		= 0x20,
	MLX5_OPCODE_GET_PSV		= 0x21,
	MLX5_OPCODE_CHECK_PSV		= 0x22,
	MLX5_OPCODE_RGET_PSV		= 0x26,
	MLX5_OPCODE_RCHECK_PSV		= 0x27,

	MLX5_OPCODE_UMR			= 0x25,
	MLX5_OPCODE_SIGNATURE_CANCELED  = (1 << 15),
};

enum {
	MLX5_SET_PORT_RESET_QKEY	= 0,
	MLX5_SET_PORT_GUID0		= 16,
	MLX5_SET_PORT_NODE_GUID		= 17,
	MLX5_SET_PORT_SYS_GUID		= 18,
	MLX5_SET_PORT_GID_TABLE		= 19,
	MLX5_SET_PORT_PKEY_TABLE	= 20,
};

enum {
	MLX5_MAX_PAGE_SHIFT		= 31
};

enum {
	MLX5_ADAPTER_PAGE_SHIFT		= 12,
	MLX5_ADAPTER_PAGE_SIZE		= 1 << MLX5_ADAPTER_PAGE_SHIFT,
};

enum {
	MLX5_CAP_OFF_DCT		= 41,
	MLX5_CAP_OFF_CMDIF_CSUM		= 46,
};

enum {
	MLX5_BW_NO_LIMIT		= 0,
	MLX5_100_MBPS_UNIT		= 3,
	MLX5_GBPS_UNIT			= 4,
};

enum {
	/*
	 * Max wqe size for rdma read is 512 bytes, so this
	 * limits our max_sge_rd as the wqe needs to fit:
	 * - ctrl segment (16 bytes)
	 * - rdma segment (16 bytes)
	 * - scatter elements (16 bytes each)
	 */
	MLX5_MAX_SGE_RD	= (512 - 16 - 16) / 16
};

struct mlx5_inbox_hdr {
	__be16		opcode;
	u8		rsvd[4];
	__be16		opmod;
};

struct mlx5_outbox_hdr {
	u8		status;
	u8		rsvd[3];
	__be32		syndrome;
};

enum mlx5_odp_transport_cap_bits {
	MLX5_ODP_SUPPORT_SEND	 = 1 << 31,
	MLX5_ODP_SUPPORT_RECV	 = 1 << 30,
	MLX5_ODP_SUPPORT_WRITE	 = 1 << 29,
	MLX5_ODP_SUPPORT_READ	 = 1 << 28,
};

struct mlx5_odp_caps {
	char reserved[0x10];
	struct {
		__be32			rc_odp_caps;
		__be32			uc_odp_caps;
		__be32			ud_odp_caps;
	} per_transport_caps;
	char reserved2[0xe4];
};

struct mlx5_cmd_set_dc_cnak_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	u8			enable;
	u8			reserved[47];
	__be64			pa;
};

struct mlx5_cmd_set_dc_cnak_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_cmd_layout {
	u8		type;
	u8		rsvd0[3];
	__be32		inlen;
	__be64		in_ptr;
	__be32		in[4];
	__be32		out[4];
	__be64		out_ptr;
	__be32		outlen;
	u8		token;
	u8		sig;
	u8		rsvd1;
	u8		status_own;
};


struct health_buffer {
	__be32		assert_var[5];
	__be32		rsvd0[3];
	__be32		assert_exit_ptr;
	__be32		assert_callra;
	__be32		rsvd1[2];
	__be32		fw_ver;
	__be32		hw_id;
	__be32		rsvd2;
	u8		irisc_index;
	u8		synd;
	__be16		ext_synd;
};

struct mlx5_init_seg {
	__be32			fw_rev;
	__be32			cmdif_rev_fw_sub;
	__be32			rsvd0[2];
	__be32			cmdq_addr_h;
	__be32			cmdq_addr_l_sz;
	__be32			cmd_dbell;
	__be32			rsvd1[120];
	__be32			initializing;
	struct health_buffer	health;
	__be32			rsvd2[880];
	__be32			internal_timer_h;
	__be32			internal_timer_l;
	__be32			rsvd3[2];
	__be32			health_counter;
	__be32			rsvd4[1019];
	__be64			ieee1588_clk;
	__be32			ieee1588_clk_type;
	__be32			clr_intx;
};

struct mlx5_eqe_comp {
	__be32	reserved[6];
	__be32	cqn;
};

struct mlx5_eqe_qp_srq {
	__be32	reserved1[5];
	u8	type;
	u8	reserved2[3];
	__be32	qp_srq_n;
};

struct mlx5_eqe_cq_err {
	__be32	cqn;
	u8	reserved1[7];
	u8	syndrome;
};

struct mlx5_eqe_port_state {
	u8	reserved0[8];
	u8	port;
};

struct mlx5_eqe_gpio {
	__be32	reserved0[2];
	__be64	gpio_event;
};

struct mlx5_eqe_congestion {
	u8	type;
	u8	rsvd0;
	u8	congestion_level;
};

struct mlx5_eqe_stall_vl {
	u8	rsvd0[3];
	u8	port_vl;
};

struct mlx5_eqe_cmd {
	__be32	vector;
	__be32	rsvd[6];
};

struct mlx5_eqe_page_req {
	u8		rsvd0[2];
	__be16		func_id;
	__be32		num_pages;
	__be32		rsvd1[5];
};

struct mlx5_eqe_page_fault {
	__be32 bytes_committed;
	union {
		struct {
			u16     reserved1;
			__be16  wqe_index;
			u16	reserved2;
			__be16  packet_length;
			u8	reserved3[12];
		} __packed wqe;
		struct {
			__be32  r_key;
			u16	reserved1;
			__be16  packet_length;
			__be32  rdma_op_len;
			__be64  rdma_va;
		} __packed rdma;
	} __packed;
	__be32 flags_qpn;
} __packed;

struct mlx5_eqe_vport_change {
	u8		rsvd0[2];
	__be16		vport_num;
	__be32		rsvd1[6];
} __packed;

struct mlx5_eqe_dct {
	__be32	reserved[6];
	__be32	dctn;
};

#define PORT_MODULE_EVENT_MODULE_STATUS_MASK 0xF
#define PORT_MODULE_EVENT_ERROR_TYPE_MASK         0xF
enum {
	MLX5_MODULE_STATUS_PLUGGED  = 0x1,
	MLX5_MODULE_STATUS_UNPLUGGED  = 0x2,
	MLX5_MODULE_STATUS_ERROR  = 0x3,
};

enum {
	MLX5_MODULE_EVENT_ERROR_POWER_BUDGET_EXCEEDED  = 0x0,
	MLX5_MODULE_EVENT_ERROR_LONG_RANGE_FOR_NON_MLNX_CABLE_MODULE  = 0x1,
	MLX5_MODULE_EVENT_ERROR_BUS_STUCK  = 0x2,
	MLX5_MODULE_EVENT_ERROR_NO_EEPROM_RETRY_TIMEOUT  = 0x3,
	MLX5_MODULE_EVENT_ERROR_ENFORCE_PART_NUMBER_LIST  = 0x4,
	MLX5_MODULE_EVENT_ERROR_UNKNOWN_IDENTIFIER  = 0x5,
	MLX5_MODULE_EVENT_ERROR_HIGH_TEMPERATURE  = 0x6,
	MLX5_MODULE_EVENT_ERROR_BAD_CABLE = 0x7,
};

struct mlx5_eqe_port_module_event {
	u8        rsvd0[1];
	u8        module;
	u8        rsvd1[1];
	u8        module_status;
	u8        rsvd2[2];
	u8        error_type;
};

union ev_data {
	__be32				raw[7];
	struct mlx5_eqe_cmd		cmd;
	struct mlx5_eqe_comp		comp;
	struct mlx5_eqe_qp_srq		qp_srq;
	struct mlx5_eqe_cq_err		cq_err;
	struct mlx5_eqe_port_state	port;
	struct mlx5_eqe_gpio		gpio;
	struct mlx5_eqe_congestion	cong;
	struct mlx5_eqe_stall_vl	stall_vl;
	struct mlx5_eqe_page_req	req_pages;
	struct mlx5_eqe_page_fault	page_fault;
	struct mlx5_eqe_vport_change	vport_change;
	struct mlx5_eqe_dct		dct;
	struct mlx5_eqe_port_module_event port_module_event;
} __packed;

struct mlx5_eqe {
	u8		rsvd0;
	u8		type;
	u8		rsvd1;
	u8		sub_type;
	__be32		rsvd2[7];
	union ev_data	data;
	__be16		rsvd3;
	u8		signature;
	u8		owner;
} __packed;

struct mlx5_cmd_prot_block {
	u8		data[MLX5_CMD_DATA_BLOCK_SIZE];
	u8		rsvd0[48];
	__be64		next;
	__be32		block_num;
	u8		rsvd1;
	u8		token;
	u8		ctrl_sig;
	u8		sig;
};

enum {
	MLX5_CQE_SYND_FLUSHED_IN_ERROR = 5,
};

struct mlx5_err_cqe {
	u8	rsvd0[32];
	__be32	srqn;
	u8	rsvd1[18];
	u8	vendor_err_synd;
	u8	syndrome;
	__be32	s_wqe_opcode_qpn;
	__be16	wqe_counter;
	u8	signature;
	u8	op_own;
};

struct mlx5_cqe64 {
	u8		outer_l3_tunneled;
	u8              rsvd0;
        __be16          wqe_id;
	u8		lro_tcppsh_abort_dupack;
	u8		lro_min_ttl;
	__be16		lro_tcp_win;
	__be32		lro_ack_seq_num;
	__be32		rss_hash_result;
	u8		rss_hash_type;
	u8		ml_path;
	u8		rsvd20[2];
	__be16		check_sum;
	__be16		slid;
	__be32		flags_rqpn;
	u8		hds_ip_ext;
	u8		l4_hdr_type_etc;
	__be16		vlan_info;
	__be32		srqn; /* [31:24]: lro_num_seg, [23:0]: srqn */
	__be32		imm_inval_pkey;
	u8		rsvd40[4];
	__be32		byte_cnt;
	__be32		timestamp_h;
	__be32		timestamp_l;
	__be32		sop_drop_qpn;
	__be16		wqe_counter;
	u8		signature;
	u8		op_own;
};

struct mlx5_mini_cqe8 {
	union {
		u32 rx_hash_result;
		u32 checksum;
		struct {
			u16 wqe_counter;
			u8  s_wqe_opcode;
			u8  reserved;
		} s_wqe_info;
	};
	u32 byte_cnt;
};

enum {
	MLX5_NO_INLINE_DATA,
	MLX5_INLINE_DATA32_SEG,
	MLX5_INLINE_DATA64_SEG,
	MLX5_COMPRESSED,
};

enum mlx5_exp_cqe_zip_recv_type {
	MLX5_CQE_FORMAT_HASH,
	MLX5_CQE_FORMAT_CSUM,
};

static inline int mlx5_get_cqe_format(struct mlx5_cqe64 *cqe)
{
#define MLX5E_CQE_FORMAT_MASK 0xc
	return (cqe->op_own & MLX5E_CQE_FORMAT_MASK) >> 2;
}

static inline int get_cqe_lro_tcppsh(struct mlx5_cqe64 *cqe)
{
	return (cqe->lro_tcppsh_abort_dupack >> 6) & 1;
}

static inline u8 get_cqe_l4_hdr_type(struct mlx5_cqe64 *cqe)
{
	return (cqe->l4_hdr_type_etc >> 4) & 0x7;
}

static inline int cqe_has_vlan(struct mlx5_cqe64 *cqe)
{
	return !!(cqe->l4_hdr_type_etc & 0x1);
}

static inline u64 get_cqe_ts(struct mlx5_cqe64 *cqe)
{
	u32 hi, lo;

	hi = be32_to_cpu(cqe->timestamp_h);
	lo = be32_to_cpu(cqe->timestamp_l);

	return (u64)lo | ((u64)hi << 32);
}

static inline u16 cqe_get_stride_bytes_recv(struct mlx5_cqe64 *cqe)
{
	return be32_to_cpu(cqe->byte_cnt) & 0xffff;
}

static inline u16 cqe_get_consumed_strides(struct mlx5_cqe64 *cqe)
{
	return (be32_to_cpu(cqe->byte_cnt) >> 16) & 0x7fff;
}

enum {
	CQE_L4_HDR_TYPE_NONE			= 0x0,
	CQE_L4_HDR_TYPE_TCP_NO_ACK		= 0x1,
	CQE_L4_HDR_TYPE_UDP			= 0x2,
	CQE_L4_HDR_TYPE_TCP_ACK_NO_DATA		= 0x3,
	CQE_L4_HDR_TYPE_TCP_ACK_AND_DATA	= 0x4,
};

enum {
	CQE_RSS_HTYPE_IP	= 0x3 << 6,
	CQE_RSS_HTYPE_L4	= 0x3 << 2,
};

enum {
	CQE_ROCE_L3_HEADER_TYPE_GRH	= 0x0,
	CQE_ROCE_L3_HEADER_TYPE_IPV6	= 0x1,
	CQE_ROCE_L3_HEADER_TYPE_IPV4	= 0x2,
};

enum {
	CQE_L2_OK	= 1 << 0,
	CQE_L3_OK	= 1 << 1,
	CQE_L4_OK	= 1 << 2,
};

enum {
	CQE_TUNNELED		= 1 << 0,
	CQE_OUTER_L3_TYPE	= 1 << 1,
	CQE_OUTER_IP_EXT_OPTS	= 1 << 2,
};

struct mlx5_sig_err_cqe {
	u8		rsvd0[16];
	__be32		expected_trans_sig;
	__be32		actual_trans_sig;
	__be32		expected_reftag;
	__be32		actual_reftag;
	__be16		syndrome;
	u8		rsvd22[2];
	__be32		mkey;
	__be64		err_offset;
	u8		rsvd30[8];
	__be32		qpn;
	u8		rsvd38[2];
	u8		signature;
	u8		op_own;
};

struct mlx5_wqe_srq_next_seg {
	u8			rsvd0[2];
	__be16			next_wqe_index;
	u8			signature;
	u8			rsvd1[11];
};

union mlx5_ext_cqe {
	struct ib_grh	grh;
	u8		inl[64];
};

struct mlx5_cqe128 {
	union mlx5_ext_cqe	inl_grh;
	struct mlx5_cqe64	cqe64;
};

struct mlx5_srq_ctx {
	u8			state_log_sz;
	u8			rsvd0[3];
	__be32			flags_xrcd;
	__be32			pgoff_cqn;
	u8			rsvd1[4];
	u8			log_pg_sz;
	u8			rsvd2[7];
	__be32			pd;
	__be16			lwm;
	__be16			wqe_cnt;
	u8			rsvd3[8];
	__be64			db_record;
};

struct mlx5_create_srq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			input_srqn;
	u8			rsvd0[4];
	struct mlx5_srq_ctx	ctx;
	u8			rsvd1[208];
	__be64			pas[0];
};

struct mlx5_create_srq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	__be32			srqn;
	u8			rsvd[4];
};

struct mlx5_destroy_srq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			srqn;
	u8			rsvd[4];
};

struct mlx5_destroy_srq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_query_srq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			srqn;
	u8			rsvd0[4];
};

struct mlx5_query_srq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd0[8];
	struct mlx5_srq_ctx	ctx;
	u8			rsvd1[32];
	__be64			pas[0];
};

struct mlx5_arm_srq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			srqn;
	__be16			rsvd;
	__be16			lwm;
};

struct mlx5_arm_srq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_cq_context {
	u8			status;
	u8			cqe_sz_flags;
	u8			st;
	u8			rsvd3;
	u8			rsvd4[6];
	__be16			page_offset;
	__be32			log_sz_usr_page;
	__be16			cq_period;
	__be16			cq_max_count;
	__be16			rsvd20;
	__be16			c_eqn;
	u8			log_pg_sz;
	u8			rsvd25[7];
	__be32			last_notified_index;
	__be32			solicit_producer_index;
	__be32			consumer_counter;
	__be32			producer_counter;
	u8			rsvd48[8];
	__be64			db_record_addr;
};

struct mlx5_create_cq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			input_cqn;
	u8			rsvdx[4];
	struct mlx5_cq_context	ctx;
	u8			rsvd6[192];
	__be64			pas[0];
};

struct mlx5_create_cq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	__be32			cqn;
	u8			rsvd0[4];
};

struct mlx5_destroy_cq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			cqn;
	u8			rsvd0[4];
};

struct mlx5_destroy_cq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd0[8];
};

struct mlx5_query_cq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			cqn;
	u8			rsvd0[4];
};

struct mlx5_query_cq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd0[8];
	struct mlx5_cq_context	ctx;
	u8			rsvd6[16];
	__be64			pas[0];
};

struct mlx5_modify_cq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			cqn;
	__be32			field_select;
	struct mlx5_cq_context	ctx;
	u8			rsvd[192];
	__be64			pas[0];
};

struct mlx5_modify_cq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_eq_context {
	u8			status;
	u8			ec_oi;
	u8			st;
	u8			rsvd2[7];
	__be16			page_pffset;
	__be32			log_sz_usr_page;
	u8			rsvd3[7];
	u8			intr;
	u8			log_page_size;
	u8			rsvd4[15];
	__be32			consumer_counter;
	__be32			produser_counter;
	u8			rsvd5[16];
};

struct mlx5_create_eq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	u8			rsvd0[3];
	u8			input_eqn;
	u8			rsvd1[4];
	struct mlx5_eq_context	ctx;
	u8			rsvd2[8];
	__be64			events_mask;
	u8			rsvd3[176];
	__be64			pas[0];
};

struct mlx5_create_eq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd0[3];
	u8			eq_number;
	u8			rsvd1[4];
};

struct mlx5_map_eq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be64			mask;
	u8			mu;
	u8			rsvd0[2];
	u8			eqn;
	u8			rsvd1[24];
};

struct mlx5_map_eq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_query_eq_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	u8			rsvd0[3];
	u8			eqn;
	u8			rsvd1[4];
};

struct mlx5_query_eq_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
	struct mlx5_eq_context	ctx;
};

enum {
	MLX5_MKEY_STATUS_FREE = 1 << 6,
};

struct mlx5_mkey_seg {
	/* This is a two bit field occupying bits 31-30.
	 * bit 31 is always 0,
	 * bit 30 is zero for regular MRs and 1 (e.g free) for UMRs that do not have tanslation
	 */
	u8		status;
	u8		pcie_control;
	u8		flags;
	u8		version;
	__be32		qpn_mkey7_0;
	u8		rsvd1[4];
	__be32		flags_pd;
	__be64		start_addr;
	__be64		len;
	__be32		bsfs_octo_size;
	u8		rsvd2[16];
	__be32		xlt_oct_size;
	u8		rsvd3[3];
	u8		log2_page_size;
	u8		rsvd4[4];
};

struct mlx5_query_special_ctxs_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_query_special_ctxs_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	__be32			dump_fill_mkey;
	__be32			reserved_lkey;
};

struct mlx5_create_mkey_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			input_mkey_index;
	__be32			flags;
	struct mlx5_mkey_seg	seg;
	u8			rsvd1[16];
	__be32			xlat_oct_act_size;
	__be32			rsvd2;
	u8			rsvd3[168];
	__be64			pas[0];
};

struct mlx5_create_mkey_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	__be32			mkey;
	u8			rsvd[4];
};

struct mlx5_query_mkey_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			mkey;
};

struct mlx5_query_mkey_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	__be64			pas[0];
};

struct mlx5_modify_mkey_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be32			mkey;
	__be64			pas[0];
};

struct mlx5_modify_mkey_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
};

struct mlx5_dump_mkey_mbox_in {
	struct mlx5_inbox_hdr	hdr;
};

struct mlx5_dump_mkey_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	__be32			mkey;
};

struct mlx5_mad_ifc_mbox_in {
	struct mlx5_inbox_hdr	hdr;
	__be16			remote_lid;
	u8			rsvd0;
	u8			port;
	u8			rsvd1[4];
	u8			data[256];
};

struct mlx5_mad_ifc_mbox_out {
	struct mlx5_outbox_hdr	hdr;
	u8			rsvd[8];
	u8			data[256];
};

struct mlx5_access_reg_mbox_in {
	struct mlx5_inbox_hdr		hdr;
	u8				rsvd0[2];
	__be16				register_id;
	__be32				arg;
	__be32				data[0];
};

struct mlx5_access_reg_mbox_out {
	struct mlx5_outbox_hdr		hdr;
	u8				rsvd[8];
	__be32				data[0];
};

#define MLX5_ATTR_EXTENDED_PORT_INFO	cpu_to_be16(0xff90)

enum {
	MLX_EXT_PORT_CAP_FLAG_EXTENDED_PORT_INFO	= 1 <<  0
};

enum {
	DCT_STATE_ACTIVE	= 0,
	DCT_STATE_DRAINING	= 1,
	DCT_STATE_DRAINED	= 2
};

static inline const char *mlx5_dct_state_str(u8 state)
{
	switch (state) {
	case DCT_STATE_ACTIVE:		return "Active";
	case DCT_STATE_DRAINING:	return "Drained";
	case DCT_STATE_DRAINED:		return "Drained";
	default: return "Invalid";
	}
}

enum mlx5_module_id {
	MLX5_MODULE_ID_SFP              = 0x3,
	MLX5_MODULE_ID_QSFP             = 0xC,
	MLX5_MODULE_ID_QSFP_PLUS        = 0xD,
	MLX5_MODULE_ID_QSFP28           = 0x11,
};

struct mlx5_allocate_psv_in {
	struct mlx5_inbox_hdr   hdr;
	__be32			npsv_pd;
	__be32			rsvd_psv0;
};

struct mlx5_allocate_psv_out {
	struct mlx5_outbox_hdr  hdr;
	u8			rsvd[8];
	__be32			psv_idx[4];
};

struct mlx5_destroy_psv_in {
	struct mlx5_inbox_hdr	hdr;
	__be32                  psv_number;
	u8                      rsvd[4];
};

struct mlx5_destroy_psv_out {
	struct mlx5_outbox_hdr  hdr;
	u8                      rsvd[8];
};

static inline int mlx5_host_is_le(void)
{
#if defined(__LITTLE_ENDIAN)
	return 1;
#elif defined(__BIG_ENDIAN)
	return 0;
#else
#error Host endianness not defined
#endif
}

#define MLX5_CMD_OP_MAX 0x920

enum {
	VPORT_STATE_DOWN		= 0x0,
	VPORT_STATE_UP			= 0x1,
};

enum {
	MLX5_L3_PROT_TYPE_IPV4		= 0,
	MLX5_L3_PROT_TYPE_IPV6		= 1,
};

enum {
	MLX5_L4_PROT_TYPE_TCP		= 0,
	MLX5_L4_PROT_TYPE_UDP		= 1,
};

enum {
	MLX5_HASH_FIELD_SEL_SRC_IP	= 1 << 0,
	MLX5_HASH_FIELD_SEL_DST_IP	= 1 << 1,
	MLX5_HASH_FIELD_SEL_L4_SPORT	= 1 << 2,
	MLX5_HASH_FIELD_SEL_L4_DPORT	= 1 << 3,
	MLX5_HASH_FIELD_SEL_IPSEC_SPI	= 1 << 4,
};

enum {
	MLX5_MATCH_OUTER_HEADERS	= 1 << 0,
	MLX5_MATCH_MISC_PARAMETERS	= 1 << 1,
	MLX5_MATCH_INNER_HEADERS	= 1 << 2,

};

enum {
	MLX5_FLOW_TABLE_TYPE_NIC_RCV	= 0,
	MLX5_FLOW_TABLE_TYPE_ESWITCH	= 4,
};

enum {
	MLX5_FLOW_CONTEXT_DEST_TYPE_VPORT	= 0,
	MLX5_FLOW_CONTEXT_DEST_TYPE_FLOW_TABLE	= 1,
	MLX5_FLOW_CONTEXT_DEST_TYPE_TIR		= 2,
};

enum mlx5_list_type {
	MLX5_NVPRT_LIST_TYPE_UC   = 0x0,
	MLX5_NVPRT_LIST_TYPE_MC   = 0x1,
	MLX5_NVPRT_LIST_TYPE_VLAN = 0x2,
};

enum {
	MLX5_RQC_RQ_TYPE_MEMORY_RQ_INLINE = 0x0,
	MLX5_RQC_RQ_TYPE_MEMORY_RQ_RPM    = 0x1,
};

enum {
	MLX5_CMD_STAT_OK			= 0x0,
	MLX5_CMD_STAT_INT_ERR			= 0x1,
	MLX5_CMD_STAT_BAD_OP_ERR		= 0x2,
	MLX5_CMD_STAT_BAD_PARAM_ERR		= 0x3,
	MLX5_CMD_STAT_BAD_SYS_STATE_ERR		= 0x4,
	MLX5_CMD_STAT_BAD_RES_ERR		= 0x5,
	MLX5_CMD_STAT_RES_BUSY			= 0x6,
	MLX5_CMD_STAT_LIM_ERR			= 0x8,
	MLX5_CMD_STAT_BAD_RES_STATE_ERR		= 0x9,
	MLX5_CMD_STAT_IX_ERR			= 0xa,
	MLX5_CMD_STAT_NO_RES_ERR		= 0xf,
	MLX5_CMD_STAT_BAD_INP_LEN_ERR		= 0x50,
	MLX5_CMD_STAT_BAD_OUTP_LEN_ERR		= 0x51,
	MLX5_CMD_STAT_BAD_QP_STATE_ERR		= 0x10,
	MLX5_CMD_STAT_BAD_PKT_ERR		= 0x30,
	MLX5_CMD_STAT_BAD_SIZE_OUTS_CQES_ERR	= 0x40,
};

enum {
	MLX5_IEEE_802_3_COUNTERS_GROUP	      = 0x0,
	MLX5_RFC_2863_COUNTERS_GROUP	      = 0x1,
	MLX5_RFC_2819_COUNTERS_GROUP	      = 0x2,
	MLX5_RFC_3635_COUNTERS_GROUP	      = 0x3,
	MLX5_ETHERNET_EXTENDED_COUNTERS_GROUP = 0x5,
	MLX5_PER_PRIORITY_COUNTERS_GROUP      = 0x10,
	MLX5_PER_TRAFFIC_CLASS_COUNTERS_GROUP = 0x11,
	MLX5_PHYSICAL_LAYER_COUNTERS_GROUP    = 0x12,
	MLX5_INFINIBAND_PORT_COUNTERS_GROUP   = 0x20,
};

/* From mlx5-abi.h */
enum {
	MLX5_NUM_UUARS_PER_PAGE	= MLX5_NON_FP_BF_REGS_PER_PAGE,
	MLX5_DEF_TOT_UUARS		= 8 * MLX5_NUM_UUARS_PER_PAGE,
};

enum {
	NUM_DRIVER_UARS		= 4,
	NUM_LOW_LAT_UUARS	= 4,
};

/* MLX5 DEV CAPs */

/* TODO: EAT.ME */
enum mlx5_cap_mode {
	HCA_CAP_OPMOD_GET_MAX	= 0,
	HCA_CAP_OPMOD_GET_CUR	= 1,
};

enum mlx5_cap_type {
	MLX5_CAP_GENERAL = 0,
	MLX5_CAP_ETHERNET_OFFLOADS,
	MLX5_CAP_ODP,
	MLX5_CAP_ATOMIC,
	MLX5_CAP_ROCE,
	MLX5_CAP_IPOIB_OFFLOADS,
	MLX5_CAP_EOIB_OFFLOADS,
	MLX5_CAP_FLOW_TABLE,
	MLX5_CAP_ESWITCH_FLOW_TABLE,
	MLX5_CAP_ESWITCH,
	MLX5_CAP_RESERVED,
	MLX5_CAP_VECTOR_CALC,
	MLX5_CAP_QOS,
	/* NUM OF CAP Types */
	MLX5_CAP_NUM
};

/* GET Dev Caps macros */
#define MLX5_CAP_GEN(mdev, cap) \
	MLX5_GET(cmd_hca_cap, mdev->hca_caps_cur[MLX5_CAP_GENERAL], cap)

#define MLX5_CAP_GEN_MAX(mdev, cap) \
	MLX5_GET(cmd_hca_cap, mdev->hca_caps_max[MLX5_CAP_GENERAL], cap)

#define MLX5_CAP_ETH(mdev, cap) \
	MLX5_GET(per_protocol_networking_offload_caps,\
		 mdev->hca_caps_cur[MLX5_CAP_ETHERNET_OFFLOADS], cap)

#define MLX5_CAP_ETH_MAX(mdev, cap) \
	MLX5_GET(per_protocol_networking_offload_caps,\
		 mdev->hca_caps_max[MLX5_CAP_ETHERNET_OFFLOADS], cap)

#define MLX5_CAP_ROCE(mdev, cap) \
	MLX5_GET(roce_cap, mdev->hca_caps_cur[MLX5_CAP_ROCE], cap)

#define MLX5_CAP_ROCE_MAX(mdev, cap) \
	MLX5_GET(roce_cap, mdev->hca_caps_max[MLX5_CAP_ROCE], cap)

#define MLX5_CAP_ATOMIC(mdev, cap) \
	MLX5_GET(atomic_caps, mdev->hca_caps_cur[MLX5_CAP_ATOMIC], cap)

#define MLX5_CAP_ATOMIC_MAX(mdev, cap) \
	MLX5_GET(atomic_caps, mdev->hca_caps_max[MLX5_CAP_ATOMIC], cap)

#define MLX5_CAP_FLOWTABLE(mdev, cap) \
	MLX5_GET(flow_table_nic_cap, mdev->hca_caps_cur[MLX5_CAP_FLOW_TABLE], cap)

#define MLX5_CAP_FLOWTABLE_MAX(mdev, cap) \
	MLX5_GET(flow_table_nic_cap, mdev->hca_caps_max[MLX5_CAP_FLOW_TABLE], cap)

#define MLX5_CAP_ESW_FLOWTABLE(mdev, cap) \
	MLX5_GET(flow_table_eswitch_cap, \
		 mdev->hca_caps_cur[MLX5_CAP_ESWITCH_FLOW_TABLE], cap)

#define MLX5_CAP_ESW_FLOWTABLE_MAX(mdev, cap) \
	MLX5_GET(flow_table_eswitch_cap, \
		 mdev->hca_caps_max[MLX5_CAP_ESWITCH_FLOW_TABLE], cap)

#define MLX5_CAP_ESW_FLOWTABLE_FDB(mdev, cap) \
	MLX5_CAP_ESW_FLOWTABLE(mdev, flow_table_properties_nic_esw_fdb.cap)

#define MLX5_CAP_ESW_FLOWTABLE_FDB_MAX(mdev, cap) \
	MLX5_CAP_ESW_FLOWTABLE_MAX(mdev, flow_table_properties_nic_esw_fdb.cap)

#define MLX5_CAP_ESW_EGRESS_ACL(mdev, cap) \
	MLX5_CAP_ESW_FLOWTABLE(mdev, flow_table_properties_esw_acl_egress.cap)

#define MLX5_CAP_ESW_EGRESS_ACL_MAX(mdev, cap) \
	MLX5_CAP_ESW_FLOWTABLE_MAX(mdev, flow_table_properties_esw_acl_egress.cap)

#define MLX5_CAP_ESW_INGRESS_ACL(mdev, cap) \
	MLX5_CAP_ESW_FLOWTABLE(mdev, flow_table_properties_esw_acl_ingress.cap)

#define MLX5_CAP_ESW_INGRESS_ACL_MAX(mdev, cap) \
	MLX5_CAP_ESW_FLOWTABLE_MAX(mdev, flow_table_properties_esw_acl_ingress.cap)

#define MLX5_CAP_ESW(mdev, cap) \
	MLX5_GET(e_switch_cap, \
		 mdev->hca_caps_cur[MLX5_CAP_ESWITCH], cap)

#define MLX5_CAP_ESW_MAX(mdev, cap) \
	MLX5_GET(e_switch_cap, \
		 mdev->hca_caps_max[MLX5_CAP_ESWITCH], cap)

#define MLX5_CAP_ODP(mdev, cap)\
	MLX5_GET(odp_cap, mdev->hca_caps_cur[MLX5_CAP_ODP], cap)

#define MLX5_CAP_ODP_MAX(mdev, cap)\
	MLX5_GET(odp_cap, mdev->hca_caps_max[MLX5_CAP_ODP], cap)

#define MLX5_CAP_ODP_PT(mdev, trans, odp_pt_cap)\
	MLX5_GET(odp_per_transport_service_cap,\
		 MLX5_ADDR_OF(odp_cap, mdev->hca_caps_cur[MLX5_CAP_ODP],\
			      trans),\
		 odp_pt_cap)

#define MLX5_CAP_ODP_PT_MAX(mdev, trans, odp_pt_cap)\
	MLX5_GET(odp_per_transport_service_cap,\
		 MLX5_ADDR_OF(odp_cap, mdev->hca_caps_max[MLX5_CAP_ODP],\
			      trans),\
		 odp_pt_cap)

#define MLX5_CAP_VECTOR_CALC(mdev, cap) \
	MLX5_GET(vector_calc_cap, \
		 mdev->hca_caps_cur[MLX5_CAP_VECTOR_CALC], cap)

#define MLX5_CAP_QOS(mdev, cap)\
	MLX5_GET(qos_cap, mdev->hca_caps_cur[MLX5_CAP_QOS], cap)

/* 8 regular priorities + 1 for multicast */
#define MLX5_NUM_BYPASS_FTS	9

#endif /* MLX5_DEVICE_H */