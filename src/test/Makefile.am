MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

CLEANFILES = test*.log test*.trs test2.fstrm *.gcda *.gcno *.gcov

AM_CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = test1 test2 test3
TESTS = test1.sh test2.sh test3.sh
EXTRA_DIST = $(TESTS)

test1_SOURCES = test1.c
test1_LDADD = ../libtinyframe.la
test1_LDFLAGS = -static
EXTRA_DIST += test.fstrm

test2_SOURCES = test2.c
test2_LDADD = ../libtinyframe.la
test2_LDFLAGS = -static

test3_SOURCES = test3.c
test3_LDADD = ../libtinyframe.la
test3_LDFLAGS = -static

if ENABLE_GCOV
gcov-local:
	for src in $(test1_SOURCES) $(test2_SOURCES) $(test3_SOURCES); do \
	  gcov -l -r -s "$(srcdir)" "$$src"; \
	done
endif
