/**
 * Parser for XenTrace binary data - Copyright (C) 2021
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

#ifndef __XTPARSER_H
#define __XTPARSER_H

#include <stdint.h>

#include "xentrace-event.h"

/**
 * XenTrace Parser instance pointer.
 */
typedef struct __xentrace_parser *xentrace_parser;

/**
 * Create a new instance based on the
 * file path passed as an argument.
 * Returns NULL on error.
 */
xentrace_parser xtp_init(const char*);

/**
 * Performs trace parsing.
 * If a trace file is damaged, it will
 * read all events until a read error occurs.
 * If executed a second time, it returns
 * only the number of previously read elements.
 * Returns zero on error.
 */
uint32_t xtp_execute(xentrace_parser);

/**
 * Returns the CPUs count of the trace.
 */
uint16_t xtp_cpus_count(xentrace_parser);

/**
 * Returns the events count of the trace.
 */
uint32_t xtp_events_count(xentrace_parser);

/**
 * Returns the event at position X of the list.
 * Returns NULL on error.
 */
xt_event *xtp_get_event(xentrace_parser, uint32_t);

/**
 * Returns the next event in the list,
 * based on the position of the iterator.
 * Returns NULL on error/end-of-list.
 */
xt_event *xtp_next_event(xentrace_parser);

/**
 * Resets the list iterator.
 */
void xtp_reset_iter(xentrace_parser);

/**
 * Frees up an instance.
 */
void xtp_free(xentrace_parser);

#endif
