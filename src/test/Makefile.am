MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

CLEANFILES = test*.log test*.trs test2.fstrm

AM_CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = test1 test2
TESTS = test1.sh test2.sh
EXTRA_DIST = $(TESTS)

test1_SOURCES = test1.c
test1_LDADD = ../libtinyframe.la
test1_LDFLAGS = -static
EXTRA_DIST += test.fstrm

test2_SOURCES = test2.c
test2_LDADD = ../libtinyframe.la
test2_LDFLAGS = -static
