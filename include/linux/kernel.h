#ifndef COMPAT_KERNEL_H
#define COMPAT_KERNEL_H

#include "../../compat/config.h"

#include_next <linux/kernel.h>

#ifndef DIV_ROUND_UP_ULL
#define DIV_ROUND_UP_ULL(ll,d) \
	({ unsigned long long _tmp = (ll)+(d)-1; do_div(_tmp, d); _tmp; })
#endif

#ifndef SIZE_MAX
#define SIZE_MAX       (~(size_t)0)
#endif

#ifdef __KERNEL__
#ifndef HAVE_RECIPROCAL_SCALE
static inline u32 reciprocal_scale(u32 val, u32 ep_ro)
{
        return (u32)(((u64) val * ep_ro) >> 32);
}
#endif
#endif /* __KERNEL__ */

#endif /* COMPAT_KERNEL_H */