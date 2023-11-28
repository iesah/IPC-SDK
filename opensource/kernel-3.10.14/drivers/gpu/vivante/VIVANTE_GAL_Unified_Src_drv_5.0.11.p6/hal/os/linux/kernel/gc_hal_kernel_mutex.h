/****************************************************************************
*
*    Copyright (C) 2005 - 2015 by Vivante Corp.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the license, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****************************************************************************/


#ifndef _gc_hal_kernel_mutex_h_
#define _gc_hal_kernel_mutex_h_

#include "gc_hal.h"
#include <linux/mutex.h>

/* Create a new mutex. */
#define gckOS_CreateMutex(Os, Mutex)                                \
({                                                                  \
    gceSTATUS _status;                                              \
    gcmkHEADER_ARG("Os=0x%X", Os);                                  \
                                                                    \
    /* Validate the arguments. */                                   \
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);                               \
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);                          \
                                                                    \
    /* Allocate the mutex structure. */                             \
    _status = gckOS_Allocate(Os, gcmSIZEOF(struct mutex), Mutex);   \
                                                                    \
    if (gcmIS_SUCCESS(_status))                                     \
    {                                                               \
        /* Initialize the mutex. */                                 \
        mutex_init(*(struct mutex **)Mutex);                        \
    }                                                               \
                                                                    \
    /* Return status. */                                            \
    gcmkFOOTER_ARG("*Mutex=0x%X", *(struct mutex **)Mutex);         \
    _status;                                                        \
})

#endif



