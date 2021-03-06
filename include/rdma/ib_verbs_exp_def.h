#ifndef IB_VERBS_EXP_DEF_H
#define IB_VERBS_EXP_DEF_H

enum ib_qpg_type {
	IB_QPG_NONE	= 0,
	IB_QPG_PARENT	= (1<<0),
	IB_QPG_CHILD_RX = (1<<1),
	IB_QPG_CHILD_TX = (1<<2)
};

enum ib_exp_start_values {
	IB_EXP_ACCESS_FLAGS_SHIFT = 0x0F
};

enum ib_exp_access_flags {
	/* Initial values are non-exp defined as part of  ib_access_flags */
	IB_EXP_ACCESS_SHARED_MR_USER_READ   = (1 << (6 + IB_EXP_ACCESS_FLAGS_SHIFT)),
	IB_EXP_ACCESS_SHARED_MR_USER_WRITE  = (1 << (7 + IB_EXP_ACCESS_FLAGS_SHIFT)),
	IB_EXP_ACCESS_SHARED_MR_GROUP_READ  = (1 << (8 + IB_EXP_ACCESS_FLAGS_SHIFT)),
	IB_EXP_ACCESS_SHARED_MR_GROUP_WRITE = (1 << (9 + IB_EXP_ACCESS_FLAGS_SHIFT)),
	IB_EXP_ACCESS_SHARED_MR_OTHER_READ  = (1 << (10 + IB_EXP_ACCESS_FLAGS_SHIFT)),
	IB_EXP_ACCESS_SHARED_MR_OTHER_WRITE = (1 << (11 + IB_EXP_ACCESS_FLAGS_SHIFT)),
	IB_EXP_ACCESS_PHYSICAL_ADDR	    = (1 << (16 + IB_EXP_ACCESS_FLAGS_SHIFT)),
};

enum ib_nvmf_offload_type {
	IB_NVMF_WRITE_OFFLOAD		 = (1ULL << 0),
	IB_NVMF_READ_OFFLOAD		 = (1ULL << 1),
	IB_NVMF_READ_WRITE_OFFLOAD	 = (1ULL << 2),
	IB_NVMF_READ_WRITE_FLUSH_OFFLOAD = (1ULL << 3),
};

struct ib_nvmf_init_data {
	enum ib_nvmf_offload_type	type;
	u8				log_max_namespace;
	u32				offloaded_capsules_count;
	u32				cmd_size;
	u8				data_offset;
	u8				log_max_io_size;
	u8				nvme_memory_log_page_size;
	u8				staging_buffer_log_page_size;
	u16				staging_buffer_number_of_pages;
	u32				staging_buffer_page_offset;
	u32				nvme_queue_size;
	u64				*staging_buffer_pas;
};

struct ib_nvmf_caps {
	u32 offload_type_dc; /* bitmap of ib_nvmf_offload_type enum */
	u32 offload_type_rc; /* bitmap of ib_nvmf_offload_type enum */
	u32 max_namespace;
	u32 max_staging_buffer_sz;
	u32 min_staging_buffer_sz;
	u32 max_io_sz;
	u32 max_be_ctrl;
	u32 max_queue_sz;
	u32 min_queue_sz;
	u32 min_cmd_size;
	u32 max_cmd_size;
	u8  max_data_offset;
};

enum ib_qp_offload_type {
	IB_QP_OFFLOAD_NVMF = 1,
};

struct ib_odp_statistics {
#ifdef CONFIG_INFINIBAND_ON_DEMAND_PAGING

	atomic_t num_page_fault_pages;

	atomic_t num_invalidation_pages;

	atomic_t num_invalidations;

	atomic_t invalidations_faults_contentions;

	atomic_t num_page_faults;

	atomic_t num_prefetches_handled;

	atomic_t num_prefetch_pages;
#endif
};

enum ibv_exp_dm_memcpy_dir {
	IBV_EXP_DM_CPY_TO_DEVICE,
	IBV_EXP_DM_CPY_TO_HOST
};

struct ib_exp_memcpy_dm_attr {
	enum ibv_exp_dm_memcpy_dir memcpy_dir;
	void *host_addr;
	uint64_t dm_offset;
	size_t length;
	uint32_t comp_mask;
};

#endif
