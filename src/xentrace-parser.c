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
#include <stdint.h>
#include <string.h>

// Xen Project
#include <trace.h>

#include "xentrace-parser.h"

#define ARR_EVENTS_SSIZE 4096
#define ARR_DOMS_SSIZE 8

/**
 * XenTrace Parser instance pointer.
 */
struct __xentrace_parser {
    // Generic vars
    char *file;         // Trace file path
    uint64_t last_tsc;  // Last TSC readed

    // Host CPU related vars
    struct __hcpu {
        uint16_t current,  // Current hCPU
                higher;    // Higher hCPU found
    } hcpu;

    // Per Host CPU "current domain" related vars
    struct __dom_l {
        xt_domain *ptr;   // Array pointer
        uint16_t length;  // Array Length
    } dom_l;

    // Event list related vars
    struct __event_l {
        xt_event *ptr;    // Array pointer
        uint32_t length,  // Array Length
                count,    // Elements count
                iter;     // Iterator position
    } event_l;
};

// Function prototypes
static int expand_dom_list(struct __dom_l *, uint16_t);
void xtp_free(xentrace_parser);

/**
 *
 */
xentrace_parser xtp_init(const char *file) {
    // Check if file var is not NULL
    if (!file)
        return NULL;

    // Initialize struct
    struct __xentrace_parser *xtp = calloc(1, sizeof(*xtp));
    if (!xtp)
        return NULL;

    // Copy file path
    xtp->file = strdup(file);
    if (!xtp->file) {
        xtp_free(xtp);
        return NULL;
    }

    // Initialize hCPU domain list
    int doms_ok = expand_dom_list(&xtp->dom_l, ARR_DOMS_SSIZE);
    if (!doms_ok) {
        xtp_free(xtp);
        return NULL;
    }

    // Initialize event list
    struct __event_l *event_l = &xtp->event_l;
    event_l->length = ARR_EVENTS_SSIZE;
    event_l->ptr = malloc(sizeof(*event_l->ptr) * ARR_EVENTS_SSIZE);
    if (!event_l->ptr) {
        xtp_free(xtp);
        return NULL;
    }

    return xtp;
}

/**
 * 
 */
static int expand_event_list(struct __event_l *event_l)  {
    // Check if expansion is needed
    if (event_l->count + 1 < event_l->length)
        return -1; // Not needed

    // (Try to) Expand array list
    xt_event *new_ptr = realloc(event_l->ptr, sizeof(*event_l->ptr) * event_l->length * 2);
    if (!new_ptr)
        return 0;

    event_l->length *= 2;
    event_l->ptr = new_ptr;
    return 1;
}

/**
 *
 */
static int expand_dom_list(struct __dom_l *dom_l, uint16_t cpu_id) {
    uint32_t old_length = dom_l->length,
            new_length  = cpu_id + 1;

    // Check if expansion is needed
    if (new_length < old_length)
        return -1; // Not needed

    // (Try to) Expand array list
    xt_domain *new_ptr = realloc(dom_l->ptr, sizeof(*dom_l->ptr) * new_length);
    if (!new_ptr)
        return 0;

    dom_l->length = new_length;
    dom_l->ptr = new_ptr;

    // Set all new domains to default value
    uint32_t dom_dflt_u32 = XEN_DOM_DFLT << 16;
    for (int i = old_length; i < new_length; ++i)
        (dom_l->ptr[i]).u32 = dom_dflt_u32;

    return 1;
}

/**
 *
 */
static int __qsort_cmpr(const void *a, const void *b) {
    uint64_t x_tsc = (((xt_event *) a)->rec).tsc,
            y_tsc  = (((xt_event *) b)->rec).tsc;

    return (x_tsc > y_tsc) - (x_tsc < y_tsc);
}

/**
 *
 */
static int read_next_record(FILE *fp, xt_record* rec) {
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
static void set_record_tsc(xentrace_parser xtp, xt_record *record) {
    // If the record doesn't include TSC, 
    // set it as the last record that had it 
    // Else, update the "last_tsc" var for
    // use it in next record(s).
    if (!record->in_tsc)
        record->tsc = xtp->last_tsc;
    else
        xtp->last_tsc = record->tsc;
}

/**
 *
 */
static int upd_current_hcpu(xentrace_parser xtp, xt_record *record) {
    // Check if event is a TRC_TRACE_CPU_CHANGE
    if ((record->id & TRC_TRACE_CPU_CHANGE) != TRC_TRACE_CPU_CHANGE)
        return 0;

    // Utility var
    uint16_t hcpu_curr =
        // Set current CPU
        (xtp->hcpu).current =
            (uint16_t)record->extra[0];

    // Save a higher CPU value
    if (hcpu_curr > (xtp->hcpu).higher)
        (xtp->hcpu).higher = hcpu_curr;

    return 1;
}

/**
 *
 */
static void upd_current_domvcpu(xentrace_parser xtp, xt_record *record) {
    // Check if event is "..._to_running"
    if ((record->id & (TRC_SCHED_MIN | 0xf0f)) != record->id)
        return;

    // Get current domain for CPU X
    uint16_t hcpu_curr = (xtp->hcpu).current;
    expand_dom_list(&xtp->dom_l, hcpu_curr);
    xt_domain *current_dom = (xtp->dom_l).ptr + hcpu_curr;

    // Update DOM and vCPU
    current_dom->u32 = record->extra[0];
}

/**
 *
 */
uint32_t xtp_execute(xentrace_parser xtp) {
    struct __event_l *event_l = &xtp->event_l;

    // If array is already populated return count
    if (event_l->count)
        return event_l->count;

    // Initialize FILE*
    FILE *fp = fopen(xtp->file, "rb");
    if (!fp)
        return 0;

    // Read trace's records
    xt_record rec;
    while (read_next_record(fp, &rec)) {
        // Update current host cpu
        if (upd_current_hcpu(xtp, &rec))
            continue;

        // Update current dom & vcpu
        upd_current_domvcpu(xtp, &rec);

        // Set record TSC
        set_record_tsc(xtp, &rec);

        // Save record into list
        // (and give a plus one to the event counter)
        xt_event *event = event_l->ptr + event_l->count++;

        event->cpu = (xtp->hcpu).current;
        event->dom = (xtp->dom_l).ptr[ event->cpu ];
        event->rec = rec;

        // Expand nodes list (if needed),
        // otherwise stop reading the trace
        if (!expand_event_list(event_l))
            break;
    }

    // Close FILE*
    fclose(fp);

    // Free up no-more-needed dom list
    free((xtp->dom_l).ptr);
    (xtp->dom_l).ptr = NULL;

    // Free up unused array space
    xt_event *new_ptr = realloc(event_l->ptr, sizeof(*event_l->ptr) * event_l->count);
    if (new_ptr)
        event_l->ptr = new_ptr;

    // Sort list
    qsort(event_l->ptr, event_l->count, sizeof(*event_l->ptr), __qsort_cmpr);

    // Return count
    return event_l->count;
}

/**
 *
 */
uint16_t xtp_cpus_count(xentrace_parser xtp) {
    return (xtp->hcpu).higher + 1;
}

/**
 *
 */
uint32_t xtp_events_count(xentrace_parser xtp) {
    return (xtp->event_l).count;
}

/**
 *
 */
xt_event *xtp_get_event(xentrace_parser xtp, uint32_t pos) {
    if (pos >= (xtp->event_l).count)
        return NULL;

    return (xtp->event_l).ptr + pos;
}

/**
 *
 */
xt_event *xtp_next_event(xentrace_parser xtp) {
    if ((xtp->event_l).iter >= (xtp->event_l).count)
        return NULL;

    return (xtp->event_l).ptr + (xtp->event_l).iter++;
}

/**
 *
 */
void xtp_reset_iter(xentrace_parser xtp) {
    (xtp->event_l).iter = 0;
}

/**
 *
 */
void xtp_free(xentrace_parser xtp) {
    free((xtp->dom_l).ptr);
    free((xtp->event_l).ptr);
    free(xtp->file);
    free(xtp);
}