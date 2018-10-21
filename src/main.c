/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
