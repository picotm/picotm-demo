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
