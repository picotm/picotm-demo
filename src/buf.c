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

#include "buf.h"
#include <assert.h>
#include <string.h>

void
data_buf_init(struct data_buf* self)
{
    assert(self);

    uint8_t (* beg)[256] = self->field;
    uint8_t (* end)[256] = self->field + 256;

    for (; beg < end; ++beg) {
        memset(beg, 0, 256);
    }
}
