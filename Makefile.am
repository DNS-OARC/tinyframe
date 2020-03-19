ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in \
  $(srcdir)/src/config.h.in~ \
  $(srcdir)/configure

SUBDIRS = src

pkgconfig_DATA = libtinyframe.pc

dist_doc_DATA = README.md CHANGES COPYING COPYING.LESSER

EXTRA_DIST = m4

test: check
