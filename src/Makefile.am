MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
CLEANFILES = *.gcda *.gcno *.gcov

EXTRA_DIST =

SUBDIRS = test

lib_LTLIBRARIES = libtinyframe.la

libtinyframe_la_SOURCES = tinyframe.c
nobase_include_HEADERS = tinyframe/tinyframe.h
nobase_nodist_include_HEADERS = tinyframe/version.h
libtinyframe_la_LDFLAGS = -version-info $(TINYFRAME_LIBRARY_VERSION)
EXTRA_DIST += tinyframe/version.h.in

if ENABLE_GCOV
gcov-local:
	for src in $(libtinyframe_la_SOURCES); do \
	  gcov -l -r -s "$(srcdir)" "$$src"; \
	done
endif
