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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xentrace-parser.h"
#include "trace.h"

#define __ARR_SSIZE 4096

/**
 * XenTrace Parser instance pointer.
 */
struct __xentrace_parser {
    char *file;  // Trace file path
    
    uint16_t higher_cpu;    // Higher CPU readed
    uint64_t last_tsc;      // Last TSC readed
    xt_header current_hrd;  // Current ev. header

    struct __list {
        uint32_t count,  // Actual elements count
                length,  // Array real length
                iter;    // Iterator position
        xt_event *ptr;   // Array pointer
    } list;
};

/**
 * 
 */
xentrace_parser xtp_init(const char *file) {
    // Check if file var is not NULL
    if (!file)
        return NULL;

    // Initialize analyzer struct 
    struct __xentrace_parser *xtp = calloc(1, sizeof(*xtp));
    if (!xtp)
        return NULL;

    // Copy file path into analyzer struct
    xtp->file = strdup(file);
    if (!xtp->file) {
        free(xtp);
        return NULL;
    }

    struct __list *list = &xtp->list;
    // Initialize events list
    list->length = __ARR_SSIZE;
    list->ptr = malloc(sizeof(*list->ptr) * __ARR_SSIZE);
    if (!list->ptr) {
        free(xtp->file);
        free(xtp);
        return NULL;
    }

    return xtp;
}

/**
 *
 */
static int __read_next_record(FILE *fp, xt_record* rec) {
    // Read header
    uint32_t hdr;
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
        return 0;

    rec->id      = TRC_HD_TO_EVENT(hdr);
    rec->n_extra = TRC_HD_EXTRA(hdr);
    rec->in_tsc  = TRC_HD_INCLUDES_CYCLE_COUNT(hdr);

    // Read the Time Stamp Counter (if any)
    if (rec->in_tsc)
        if (fread(&rec->tsc, sizeof(rec->tsc), 1, fp) != 1)
            return 0;

    // Read extra[] array (if any)
    if (rec->n_extra)
        if (fread(&rec->extra, sizeof(rec->extra[0]), rec->n_extra, fp) != rec->n_extra)
            return 0;

    return 1;
}

/**
 * 
 */
static int __expand_list(struct __list *list) {
    // If it is not the "second-last" element
    if (list->length > list->count + 1)
        return 0; // Not needed

    // Dupe array size
    list->length *= 2;
    xt_event *new_ptr = realloc(list->ptr, sizeof(*list->ptr) * list->length);

    // If "realloc" fails
    if (!new_ptr)
        return -1; // Failed

    // Expansion done
    list->ptr = new_ptr;
    return 1; // Success
}

/**
 * 
 */
static void __set_record_tsc(xentrace_parser xtp, xt_record *record) {
    // If already included, just update "last_tsc"
    if (record->in_tsc)
        xtp->last_tsc = record->tsc;
    else
        record->tsc = xtp->last_tsc;
}

/**
 * 
 */
static int __upd_current_domvcpu(xentrace_parser xtp, xt_record *record) {
    // Check if event is "..._to_running"
    if ((record->id & (TRC_SCHED_MIN | 0xf0f)) != record->id)
        return 0;

    // Utility vars
    xt_header *current_hrd = &xtp->current_hrd;
    uint32_t domvcpu = record->extra[0];

    // Update DOM and vCPU
    current_hrd->dom  = domvcpu >> 16;
    current_hrd->vcpu = domvcpu & 0x0000ffff;

    return 1;
}

/**
 * 
 */
static int __upd_current_hstcpu(xentrace_parser xtp, xt_record *record) {
    // Check if event is a TRC_TRACE_CPU_CHANGE
    if ((record->id & TRC_TRACE_CPU_CHANGE) != TRC_TRACE_CPU_CHANGE)
        return 0;

    // Utility var
    uint16_t cpu =
        // Set current CPU
        (xtp->current_hrd).cpu =
            (uint16_t)record->extra[0];

    // Save a higher CPU value
    if (cpu > xtp->higher_cpu)
        xtp->higher_cpu = cpu;
    
    return 1;
}

static int __qsort_cmpr(const void *a, const void *b) {
    xt_record x = ((xt_event *) a)->rec,
              y = ((xt_event *) b)->rec;

    return (x.tsc > y.tsc) - (x.tsc < y.tsc);
}

/**
 * 
 */
uint32_t xtp_execute(xentrace_parser xtp) {
    struct __list *list = &xtp->list;

    // If array is already populated return count
    if (list->count)
        return list->count;

    // Initialize FILE*
    FILE *fp = fopen(xtp->file, "rb");
    if (!fp)
        return 0;

    // Read trace's records
    xt_record rec;
    while (__read_next_record(fp, &rec)) {
        // Update current host cpu
        if (__upd_current_hstcpu(xtp, &rec))
            continue;

        // Update current dom & vcpu
        __upd_current_domvcpu(xtp, &rec);

        // Set record TSC
        __set_record_tsc(xtp, &rec);

        // Save record into list 
        // (and give a plus one to the event counter)
        xt_event *event = list->ptr + list->count++;
        event->hdr = xtp->current_hrd;
        event->rec = rec;

        // Expand nodes list (if needed), 
        // otherwise stop reading the trace
        if (__expand_list(list) == -1)
            break;
    }

    // Close FILE*
    fclose(fp);

    // Free up unused array space
    xt_event *new_ptr = realloc(list->ptr, sizeof(*list->ptr) * list->count);
    if (new_ptr)
        list->ptr = new_ptr;

    // Sort list
    qsort(list->ptr, list->count, sizeof(*list->ptr), __qsort_cmpr);

    // Return count
    return list->count;
}

/**
 * 
 */
uint16_t xtp_cpus_count(xentrace_parser xtp) {
    return xtp->higher_cpu + 1;
}

/**
 * 
 */
uint32_t xtp_events_count(xentrace_parser xtp) {
    return (xtp->list).count;
}

/**
 * 
 */
xt_event *xtp_get_event(xentrace_parser xtp, uint32_t pos) {
    if (pos >= (xtp->list).count)
        return NULL;

    return (xtp->list).ptr + pos;
}

/**
 * 
 */
xt_event *xtp_next_event(xentrace_parser xtp) {
    if ((xtp->list).iter >= (xtp->list).count)
        return NULL;

    return (xtp->list).ptr + (xtp->list).iter++;
}

/**
 * 
 */
void xtp_reset_iter(xentrace_parser xtp) {
    (xtp->list).iter = 0;
}

/**
 * 
 */
void xtp_free(xentrace_parser xtp) {
    free((xtp->list).ptr);
    free(xtp->file);
    free(xtp);
}