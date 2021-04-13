/**
 * Event structs for XenTrace binary data - Copyright (C) 2021
 * Giuseppe Eletto <peppe.eletto@gmail.com>
 * Dario Faggioli  <dfaggioli@suse.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef __XTEVENT_H
#define __XTEVENT_H

#include <stdint.h>

#define XEN_DOM_IDLE 32767
#define XEN_DOM_DFLT 32768

#define XEN_REC_XTRS 7

/**
 * Domain union.
 */
typedef union {
    uint32_t u32;
    struct { uint16_t vcpu, id; };
} xt_domain;

/**
 * Record struct.
 */
typedef struct {
    uint32_t id:28;                  // Identifier
    uint8_t n_extra:3,               // N# items in extra[] array
            in_tsc:1;                // Include t.s.c. ?
    uint64_t tsc;                    // Time Stamp Counter
    uint32_t extra[ XEN_REC_XTRS ];  // Items of extra[] array
} xt_record;

/**
 * Event struct.
 */
typedef struct {
    uint16_t  cpu;  // Host CPU value
    xt_domain dom;  // Domain struct
    xt_record rec;  // Record struct
} xt_event;

#endif
