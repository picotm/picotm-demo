/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "ui.h"
#include <assert.h>
#include <picotm/picotm.h>
#include <picotm/picotm-tm-ctypes.h>
#include <picotm/stdlib.h>
#include <stdlib.h>
#include <unistd.h>

#if defined HAVE_NCURSESW_CURSES_H
#include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_CURSES_H
#include <curses.h>
#else
#error "SysV or X/Open-compatible Curses header file required"
#endif

#if defined HAVE_NCURSESW_FORM_H
#include <ncursesw/form.h>
#elif defined HAVE_NCURSES_FORM_H
#include <ncurses/form.h>
#elif defined HAVE_FORM_H
#include <form.h>
#else
#error "SysV-compatible Curses Form header file required"
#endif

#include "buf.h"
#include "ptr.h"
#include "recovery.h"

static void
ncurses_atexit(void)
{
    endwin();
}

static unsigned int
field_sum_tx(const uint8_t* field)
{
    unsigned int sum = 0;

    privatize_tx(field, 256, PICOTM_TM_PRIVATIZE_LOAD);

    const uint8_t* beg = field;
    const uint8_t* end = field + 256;

    for (const uint8_t* pos = beg; pos < end; ++pos) {
        sum += *pos;
    }

    return sum;
}

static int
fill_out_buffer(char* out, size_t outlen, struct data_buf* buf)
{
    const uint8_t (*field)[256] = buf->field;

    const size_t nsteps = arraylen(buf->field) / outlen;

    char* out_beg = out;
    char* out_end = out + outlen;

    for (char* out_pos = out_beg; out_pos < out_end; ++out_pos) {

        static const char character[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };

        picotm_begin

            unsigned int sum = 0;

            for (size_t i = 0; i < nsteps; ++i, ++field) {
                sum += field_sum_tx(*field);
            }

            size_t i = sum / (nsteps * (256 * arraylen(character)));

            store_char_tx(out_pos, character[i]);
        picotm_commit
            int res = recover_from_tx_error(__FILE__, __LINE__);
            if (res < 0) {
                return -1;
            }
            picotm_restart();
        picotm_end
    }

    return 0;
}

void
ui_main(struct data_buf* buf, size_t nbufs)
{
    /* Init ncurses
     */

    WINDOW* w = initscr();

    /* clean up ncurses and terminal on exit */
    atexit(ncurses_atexit);

    nodelay(w, true);

    /* Setup fields for buffer output.
     */

    FIELD** field;

    picotm_begin
        size_t tx_nbufs = load_size_t_tx(&nbufs);
        FIELD** tx_field = malloc_tx((1 + tx_nbufs) * sizeof(*tx_field));
        store_ptr_tx(&field, tx_field);
    picotm_commit
        int res = recover_from_tx_error(__FILE__, __LINE__);
        if (res < 0) {
            return;
        }
        picotm_restart();
    picotm_end

    for (size_t i = 0; i < nbufs; ++i) {
        field[i] = new_field(1, 64, 12 + 4 * i, 8, 0, 0);
        set_field_back(field[i], A_UNDERLINE);
    }

    field[nbufs] = NULL;

    FORM* form = new_form(field);
    post_form(form);

    for (size_t i = 0; i < nbufs; ++i) {
        mvprintw(10 + 4 * i, 2, "Buffer %zu", i + 1);
    }

    /* Display some text and the periodically refresh the output
     * from the buffers.
     */

    mvprintw(2, 0, "picotm Demo application");
    mvprintw(3, 0, "=======================");
    mvprintw(5, 0, "This program reads random data and sorts it into a set of internal "
                   "buffers. You should see the buffers slowly filling up.\n"
                   "Press <ctrl-c> to exit");

    while (true) {

        struct data_buf* buf_beg = buf;
        struct data_buf* buf_end = buf + nbufs;

        for (struct data_buf* buf = buf_beg; buf < buf_end; ++buf) {

            char out[64];

            static_assert(arraylen(buf->field) >= arraylen(out),
                          "output length is larger than field length");
            static_assert(!(arraylen(buf->field) % arraylen(out)),
                          "field length is not a multiple of output length");

            int res = fill_out_buffer(out, arraylen(out), buf);
            if (res < 0) {
                return;
            }

            mvprintw(12 + 4 * (buf -  buf_beg), 8, "%.*s", arraylen(out), out);

            refresh();

            /* In a real-world application, we might wake up from an
             * interupt, timeout or some other input. For the demo, we
             * simply sleep for a second.
             */
            sleep(1);
        }
    }
}
