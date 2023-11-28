#ifndef _COMMON_H_
#define _COMMON_H_

#include <linux/kernel.h>
#include <linux/io.h>

#include "bit_field.h"

#define assert(expr)                                            \
    do {                                                        \
        if (!(expr)) {                                          \
            panic("Assertion failed! %s, %s, %s, line %d\n",    \
                  #expr, __FILE__, __func__, __LINE__);         \
        }                                                       \
    } while (0)

#define phys_to_page(phys) pfn_to_page((phys) >> PAGE_SHIFT)

#endif /* _COMMON_H_ */
