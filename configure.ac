dnl
dnl picotm-demo - A demo application for picotm
dnl Copyright (c) 2017-2018   Thomas Zimmermann <contact@tzimmermann.org>
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.
dnl
dnl SPDX-License-Identifier: GPL-3.0-or-later
dnl

AC_PREREQ([2.69])
AC_INIT([picotm-demo], [0.0.0], [bugs@picotm.org], [], [http://picotm.org])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])


dnl
dnl Compiler support
dnl

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC

AX_PTHREAD
CC="$PTHREAD_CC"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"

dnl Request C11
AX_CHECK_COMPILE_FLAG([-std=c11], [CFLAGS="$CFLAGS -std=c11"])


dnl
dnl System headers and libraries
dnl

AC_CHECK_HEADERS([sys/cdefs.h])


dnl
dnl Ncurses terminal library
dnl

AX_WITH_CURSES
AX_WITH_CURSES_FORM


dnl
dnl picotm
dnl

CHECK_PICOTM
CHECK_PICOTM_TM
CHECK_PICOTM_TXLIB
CHECK_PICOTM_C


dnl
dnl Output
dnl

AC_CONFIG_FILES([Makefile
                 LICENSES/Makefile
                 src/Makefile])
AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([  Configuration finished.])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Continue installation with:])
AC_MSG_RESULT([    make])
AC_MSG_RESULT([    make install])
AC_MSG_RESULT([])
