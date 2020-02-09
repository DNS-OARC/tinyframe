MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

CLEANFILES = test*.log test*.trs

AM_CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = test1
TESTS = test1.sh
EXTRA_DIST = $(TESTS)

test1_SOURCES = test1.c
test1_LDADD = ../libtinyframe.la
test1_LDFLAGS = -static
EXTRA_DIST += test.fstrm
