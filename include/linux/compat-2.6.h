#ifndef LINUX_26_COMPAT_H
#define LINUX_26_COMPAT_H

#define LINUX_BACKPORT(__sym) backport_ ##__sym

/* Include the autogenerated header file */
#include "../../compat/config.h"

#include <linux/version.h>
#include <linux/kconfig.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0))
#include <linux/if.h>
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/compat_autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,16))
#include <linux/uidgid.h>
#endif

/*
 * The define overwriting module_init is based on the original module_init
 * which looks like this:
 * #define module_init(initfn)					\
 *	static inline initcall_t __inittest(void)		\
 *	{ return initfn; }					\
 *	int init_module(void) __attribute__((alias(#initfn)));
 *
 * To the call to the initfn we added the symbol dependency on compat
 * to make sure that compat.ko gets loaded for any compat modules.
 */
void backport_dependency_symbol(void);

#undef module_init
#define module_init(initfn)						\
	static int __init __init_backport(void)				\
	{								\
		backport_dependency_symbol();				\
		return initfn();					\
	}								\
	int init_module(void) __attribute__((alias("__init_backport")));

/*
 * Each compat file represents compatibility code for new kernel
 * code introduced for *that* kernel revision.
 */

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
#include <linux/compat-2.6.30.h>
#include <linux/compat-2.6.31.h>
#include <linux/compat-2.6.32.h>
#include <linux/compat-2.6.33.h>
#include <linux/compat-2.6.34.h>
#include <linux/compat-2.6.35.h>
#include <linux/compat-2.6.36.h>
#include <linux/compat-2.6.37.h>
#include <linux/compat-2.6.38.h>
#include <linux/compat-2.6.39.h>
#include <linux/compat-3.0.h>
#include <linux/compat-3.1.h>
#include <linux/compat-3.2.h>
#include <linux/compat-3.3.h>
#include <linux/compat-3.4.h>
#include <linux/compat-3.5.h>
#include <linux/compat-3.6.h>
#include <linux/compat-3.7.h>
#include <linux/compat-3.8.h>
#include <linux/compat-3.9.h>
#include <linux/compat-3.10.h>
#include <linux/compat-3.11.h>
#include <linux/compat-3.12.h>
#include <linux/compat-3.13.h>
#include <linux/compat-3.14.h>
#include <linux/compat-3.15.h>
#include <linux/compat-3.16.h>
#include <linux/compat-3.17.h>
#include <linux/compat-4.0.h>
#include <linux/compat-4.1.h>
#include <linux/compat-4.10.h>
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29) */

#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18))
#include <linux/compat-2.6.19.h>
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 16))
#include <linux/compat-2.6.17.h>
#endif


#ifndef HAVE_ELFCOREHDR_ADDR_EXPORTED
#define elfcorehdr_addr LINUX_BACKPORT(elfcorehdr_addr)
#endif

#endif /* LINUX_26_COMPAT_H */
