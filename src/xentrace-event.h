/**
 * Event structs for XenTrace binary data - Copyright (C) 2021
 * Giuseppe Eletto <peppe.eletto@gmail.com>
 * Dario Faggioli  <dfaggioli@suse.com>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU LGPLv3 along with
 * this library. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __XTEVENT_H
#define __XTEVENT_H

#include <stdint.h>

#define XEN_REC_XTRS 7
#define XEN_DOM_IDLE 32767

/**
 * XenTrace domain struct.
 */
typedef struct {
    uint16_t dom,  // Domain
            vcpu,  // Virtual CPU
            cpu;   // Host CPU
} xt_header;

/**
 * XenTrace record struct.
 */
typedef struct {
    uint32_t id:28;                  // Identifier
    uint8_t n_extra:3,               // N# items in extra[] array
            in_tsc:1;                // Include t.s.c. ?
    uint64_t tsc;                    // Time Stamp Counter
    uint32_t extra[ XEN_REC_XTRS ];  // Items of extra[] array
} xt_record;

/**
 * XenTrace event struct.
 */
typedef struct {
    xt_header hdr;  // Header struct
    xt_record rec;  // Record struct
} xt_event;

#endif
