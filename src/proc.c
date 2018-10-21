/*
 * picotm-demo - A demo application for picotm
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "proc.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm.h>
#include <picotm/picotm-tm-ctypes.h>
#include <picotm/stdbool.h>
#include <picotm/stdlib.h>
#include <picotm/string.h>
#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "ptr.h"
#include "queue.h"
#include "recovery.h"

static struct queue_entry*
queue_entry_of_txqueue_entry_tx(struct txqueue_entry* entry)
{
    assert(entry);

    return containerof(entry, struct queue_entry, entry);
}

static void
proc_main_loop(struct queue* q, struct data_buf* buf)
{
    assert(q);
    assert(buf);

    int err = pthread_mutex_lock(&q->mutex);
    if (err) {
        errno = err;
        perror("pthread_mutex_lock");
        return;
    }

    while (1) {

        int err = pthread_cond_wait(&q->cond, &q->mutex);
        if (err) {
            errno = err;
            perror("pthread_cond_wait");
            goto err_pthread_cond_wait;
        }

        bool continue_loop;

        do {

            continue_loop = false;

            picotm_begin

                /* Acquire transactional queue for queue state. */
                struct txqueue* queue = txqueue_of_state_tx(&q->queue);

                /* Get next message from queue. */
                if (txqueue_empty_tx(queue)) {
                    goto commit;
                }
                struct queue_entry* entry =
                    queue_entry_of_txqueue_entry_tx(txqueue_front_tx(queue));

                /* Copy message buffer into correct field and fill trailing
                 * bytes with 0. */
                uint8_t* field = buf->field[entry->msg.off];
                memcpy_tx(field, entry->msg.buf, entry->msg.len);
                memset_tx(field + entry->msg.len, 0, 256 - entry->msg.len);

                /* Remove message from queue and free memory. */
                txqueue_pop_tx(queue);
                destroy_queue_entry_tx(entry);

                /* Continue loop until queue runs empty */
                store_bool_tx(&continue_loop, true);

            commit:
            picotm_commit
                int res = recover_from_tx_error(__FILE__, __LINE__);
                if (res < 0) {
                    return;
                }
                picotm_restart();
            picotm_end

        } while (continue_loop);
    }

    err = pthread_mutex_unlock(&q->mutex);
    if (err) {
        errno = err;
        perror("pthread_mutex_unlock");
        abort(); /* abort if cleanup fails */
    }

    return;

err_pthread_cond_wait:
    err = pthread_mutex_unlock(&q->mutex);
    if (err) {
        errno = err;
        perror("pthread_mutex_unlock");
        abort(); /* abort if cleanup fails */
    }
}

struct proc_main_arg {
    struct queue* q;
    struct data_buf* buf;
};

static void
thread_cleanup(void* arg)
{
    picotm_release();

    free(arg);
}

static void
proc_main(struct proc_main_arg* arg)
{
    pthread_cleanup_push(thread_cleanup, arg);

    proc_main_loop(arg->q, arg->buf);

    pthread_cleanup_pop(1);
}

static void*
proc_main_cb(void* arg)
{
    proc_main(arg);
    return NULL;
}

int
run_proc_thread(struct queue* q, struct data_buf* buf, pthread_t* thread)
{
    struct proc_main_arg* arg = NULL;

    picotm_begin
        struct proc_main_arg* tx_arg = malloc_tx(sizeof(*tx_arg));
        tx_arg->q = q;
        tx_arg->buf = buf;

        store_ptr_tx(&arg, tx_arg);

    picotm_commit
        int res = recover_from_tx_error(__FILE__, __LINE__);
        if (res < 0) {
            return -1;
        }
        picotm_restart();
    picotm_end

    assert(arg);

    int err = pthread_create(thread, NULL, proc_main_cb, arg);
    if (err) {
        errno = err;
        perror("pthread_create");
        goto err_pthread_create;
    }

    return 0;

err_pthread_create:
    free(arg);
    return -1;
}
