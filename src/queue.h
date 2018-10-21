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

#pragma once

#include <picotm/picotm-txqueue.h>
#include <pthread.h>
#include "data.h"

struct queue_entry {
    struct txqueue_entry entry;
    struct hdr msg;
};

void
queue_entry_init(struct queue_entry* self);

struct queue_entry*
create_queue_entry_tx(void);

void
destroy_queue_entry_tx(struct queue_entry* entry);

struct queue {

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    struct txqueue_state queue;
};

#define QUEUE_INITIALIZER(_queue)                   \
    {                                               \
        PTHREAD_MUTEX_INITIALIZER,                  \
        PTHREAD_COND_INITIALIZER,                   \
        TXQUEUE_STATE_INITIALIZER((_queue).queue)   \
    }
