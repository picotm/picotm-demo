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
