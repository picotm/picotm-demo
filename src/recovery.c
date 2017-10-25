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

#include "recovery.h"
#include <picotm/picotm.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ptr.h"

static void
print_error_str(const char* file, int line, const char* fmt, ...)
{
    fprintf(stderr, "%s:%d: ", file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

int
recover_from_tx_error(const char* file, int line)
{
    static const char* const error_string[] = {
        [PICOTM_GENERAL_ERROR] = "General Error",
        [PICOTM_OUT_OF_MEMORY] = "Out Of Memory"
    };

    /* Application-specific error recovery should be implemented here. In
     * most applications there's a number of recovery strategies for each
     * error. If all of them fails, the application can at least abort in
     * a consistent state.
     */

    switch (picotm_error_status()) {
        case PICOTM_ERROR_CODE:
            print_error_str(file, line, "Error: %s\n",
                            error_string[picotm_error_as_error_code()]);
            break;
        case PICOTM_ERRNO: {
            int errnum = picotm_error_as_errno();
            char errstr[256];
            strerror_r(errnum, errstr, arraylen(errstr));
            print_error_str(file, line, "Errno: %d (%s)\n", errnum, errstr);
            break;
        }
        default:
            print_error_str(file, line, "Unknown error.\n");
            break;
    }

    if (picotm_error_is_non_recoverable()) {
        print_error_str(file, line, "Error is non-recoverable. Aborting.\n");
        abort();
    }

    /* No actual recovery is implemented, so we return -1 to stop the
     * transaction. A real-world application could run the GC to free up
     * memory, remove temporary files on the disk, flush caches, etc. */
    return -1;
}
