MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
EXTRA_DIST =

SUBDIRS = test

lib_LTLIBRARIES = libtinyframe.la

libtinyframe_la_SOURCES = tinyframe.c
nobase_nodist_include_HEADERS = tinyframe/tinyframe.h
libtinyframe_la_LDFLAGS = -version-info $(TINYFRAME_LIBRARY_VERSION)
EXTRA_DIST += tinyframe/tinyframe.h.in
