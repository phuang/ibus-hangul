# vim:set noet ts=4:
#
# ibus-hangul - The Hangul engine for IBus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

engine_hangul_PYTHON = \
	engine.py \
	factory.py \
	main.py \
	$(NULL)
engine_hanguldir = $(pkgdatadir)/engine

hangul_DATA = \
	hangul.py \
	$(NULL)
hangul_LTLIBRARIES = _hangul.la
hanguldir = @pyexecdir@

_hangul_la_SOURCES = \
	$(NULL)

nodist__hangul_la_SOURCES = \
	hangul_wrap.c \
	$(NULL)

_hangul_la_CFLAGS = \
	@HANGUL_CFLAGS@ \
	@PYTHON_CFLAGS@ \
	$(NULL)

_hangul_la_LDFLAGS = \
	@HANGUL_LIBS@ \
	@PYTHON_LIBS@ \
	-rpath $(hanguldir) \
	-avoid-version \
	-module \
	$(NULL)

bin_SCRIPTS = ibus-engine-hangul
engine_DATA = hangul.engine
enginedir = $(datadir)/ibus/engine

hangul.py hangul_wrap.c: hangul.i
	$(SWIG) -python -I/usr/include -o hangul_wrap.c $(srcdir)/hangul.i

test:
	$(ENV) \
		DBUS_DEBUG=true \
		IBUS_HANGUL_PKGDATADIR=$(abs_top_srcdir) \
		PYTHONPATH=$${PYTHONPATH}:$(builddir)/.libs \
		$(PYTHON) \
		$(srcdir)/main.py

EXTRA_DIST = \
	hangul.i \
	hangul.engine.in \
	ibus-engine-hangul.in \
	$(NULL)

CLEANFILES = \
	hangul.py \
	hangul_wrap.* \
	*.pyc \
	$(MULL)

DISTCLEANFILES = \
	$(MULL)
