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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "in.h"
#include "proc.h"
#include "ptr.h"
#include "queue.h"
#include "ui.h"

static const char DEV_URANDOM[] = "/dev/urandom";

static struct queue g_queue[4] = {
    QUEUE_INITIALIZER(g_queue[0]),
    QUEUE_INITIALIZER(g_queue[1]),
    QUEUE_INITIALIZER(g_queue[2]),
    QUEUE_INITIALIZER(g_queue[3])
};

static struct data_buf g_data_buf[4];

int
main(int argc, char* argv[])
{
    /* Data buffers */
    {
        struct data_buf* beg = g_data_buf;
        struct data_buf* end = g_data_buf + 4;

        for (struct data_buf* buf = beg; buf < end; ++buf) {
            data_buf_init(buf);
        }
    }

    /* Input thread */
    pthread_t in_thread;
    int res = run_in_thread(DEV_URANDOM, g_queue, arraylen(g_queue),
                            &in_thread);
    if (res < 0) {
        return EXIT_FAILURE;
    }

    /* Output threads */

    pthread_t proc_thread[4];

    {
        pthread_t* beg = proc_thread;
        pthread_t* end = proc_thread + 4;

        for (pthread_t* thread = beg; thread < end; ++thread) {
            int res = run_proc_thread(g_queue + (thread - beg),
                                      g_data_buf + (thread - beg),
                                      thread);
            if (res < 0) {
                return EXIT_FAILURE;
            }
        }
    }

    /* UI */
    ui_main(g_data_buf, arraylen(g_data_buf));

    /* Clean up */

    void* retval;
    {
        pthread_t* beg = proc_thread;
        pthread_t* end = proc_thread + 4;

        for (pthread_t* thread = beg; thread < end; ++thread) {
            pthread_join(*thread, &retval);
        }
    }
    pthread_join(in_thread, &retval);

    return EXIT_SUCCESS;
}
