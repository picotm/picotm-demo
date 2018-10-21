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

#include "queue.h"
#include <assert.h>
#include <picotm/stdlib.h>

void
queue_entry_init(struct queue_entry* self)
{
    assert(self);

    txqueue_entry_init(&self->entry);
}

struct queue_entry*
create_queue_entry_tx()
{
    struct queue_entry* entry = malloc_tx(sizeof(*entry));
    txqueue_entry_init_tm(&entry->entry);
    return entry;
}

void
destroy_queue_entry_tx(struct queue_entry* self)
{
    txqueue_entry_uninit_tm(&self->entry);
    free_tx(self);
}
