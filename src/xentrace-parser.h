/**
 * Parser for XenTrace binary data - Copyright (C) 2021
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

#ifndef __XTPARSER_H
#define __XTPARSER_H

#include <stdint.h>

#include "xentrace-event.h"

/**
 * XenTrace Parser instance pointer.
 */
typedef struct __xentrace_parser *xentrace_parser;

/**
 * 
 */
xentrace_parser xtp_init(const char*);

/**
 * 
 */
uint32_t xtp_execute(xentrace_parser);

/**
 * 
 */
uint16_t xtp_cpus_count(xentrace_parser);

/**
 * 
 */
uint32_t xtp_events_count(xentrace_parser);

/**
 * 
 */
xt_event *xtp_get_event(xentrace_parser, uint32_t);

/**
 * 
 */
xt_event *xtp_next_event(xentrace_parser);

/**
 * 
 */
void xtp_reset_iter(xentrace_parser);

/**
 * 
 */
void xtp_free(xentrace_parser);

#endif
