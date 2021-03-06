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

#include "in.h"
#include <errno.h>
#include <picotm/fcntl.h>
#include <picotm/picotm.h>
#include <picotm/picotm-tm-ctypes.h>
#include <picotm/stdlib.h>
#include <picotm/unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "queue.h"
#include "recovery.h"

static int
open_input_file(const char* filename)
{
    int fd;

    picotm_begin

        /* Privatize the memory of the argument and filename */
        privatize_c_tx(filename, '\0', PICOTM_TM_PRIVATIZE_LOAD);

        /* Open input file */
        int tx_fd = open_tx(filename, O_RDONLY);

        /* Export the file descriptor from the transaction */
        store_int_tx(&fd, tx_fd);

    picotm_commit
        int res = recover_from_tx_error(__FILE__, __LINE__);
        if (res < 0) {
            return -1;
        }
        picotm_restart();
    picotm_end

    return fd;
}

static void
close_file_descriptor(int fd)
{
    picotm_begin
        close_tx(fd);
    picotm_commit
        int res = recover_from_tx_error(__FILE__, __LINE__);
        if (res < 0) {
            return;
        }
        picotm_restart();
    picotm_end
}

static struct queue_entry*
read_file(int fd)
{
    struct queue_entry* entry;

    picotm_begin

        struct queue_entry* tx_entry = create_queue_entry_tx();

        /* Read message header from input stream. The value of `fd` is a
         * constant on the stack; no need to load or privatize. The first
         * 4 byte are considered meta data. */
        read_tx(fd, &tx_entry->msg, 4);

        /* Read data into buffer. With `read_tx()` the buffer `msg->buf` is
         * automatically privatized by the TM module.
         */
        read_tx(fd, tx_entry->msg.buf, tx_entry->msg.len);

        /* Export message from transaction context.
         */
        store_ptr_tx(&entry, tx_entry);

    picotm_commit
        int res = recover_from_tx_error(__FILE__, __LINE__);
        if (res < 0) {
            return NULL;
        }
        picotm_restart();
    picotm_end

    return entry;
}

static void
in_main_loop(const char* filename, struct queue* outq, size_t noutqs)
{
    int fd = open_input_file(filename);
    if (fd < 0) {
        return;
    }

    while (1) {
        struct queue_entry* entry = read_file(fd);
        if (!entry) {
            goto out;
        }

        /* Pick one of the output queues, enqueue the message and
         * send a signal to one of the processing threads.
         */

        struct queue* q = outq + (entry->msg.queue % noutqs);

        picotm_begin
            struct txqueue* queue = txqueue_of_state_tx(&q->queue);
            txqueue_push_tx(queue, &entry->entry);
        picotm_commit
            int res = recover_from_tx_error(__FILE__, __LINE__);
            if (res < 0) {
                goto out;
            }
            picotm_restart();
        picotm_end

        /* TODO: Could signalling be done transactionally? */
        int err = pthread_mutex_lock(&q->mutex);
        if (err) {
            errno = err;
            perror("pthread_mutex_lock");
            abort();
        }
        err = pthread_cond_signal(&q->cond);
        if (err) {
            errno = err;
            perror("pthread_cond_signal");
            abort();
        }
        err = pthread_mutex_unlock(&q->mutex);
        if (err) {
            errno = err;
            perror("pthread_mutex_unlock");
            abort();
        }

        sleep(1);
    }

out:
    close_file_descriptor(fd);
}

struct in_main_arg {
    const char* filename;
    struct queue* outq;
    size_t noutqs;
};

static void
thread_cleanup(void* arg)
{
    picotm_release();

    free(arg);
}

static void
in_main(struct in_main_arg* arg)
{
    pthread_cleanup_push(thread_cleanup, arg);

    in_main_loop(arg->filename, arg->outq, arg->noutqs);

    pthread_cleanup_pop(1);
}

static void*
in_main_cb(void* arg)
{
    in_main(arg);
    return NULL;
}

int
run_in_thread(const char* filename, struct queue* outq, size_t noutqs,
              pthread_t* thread)
{
    struct in_main_arg* arg = NULL;

    picotm_begin
        struct in_main_arg* tx_arg = malloc_tx(sizeof(*tx_arg));
        tx_arg->filename = filename;
        tx_arg->outq = outq;
        tx_arg->noutqs = noutqs;

        store_ptr_tx(&arg, tx_arg);

    picotm_commit
        int res = recover_from_tx_error(__FILE__, __LINE__);
        if (res < 0) {
            return -1;
        }
        picotm_restart();
    picotm_end

    int err = pthread_create(thread, NULL, in_main_cb, arg);
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
