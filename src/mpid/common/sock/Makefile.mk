## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2011 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

errnames_txt_files += src/mpid/common/sock/errnames.txt

if BUILD_MPID_COMMON_SOCK

# FIXME is top_builddir the right way to handle VPATH builds?
AM_CPPFLAGS +=                             \
    -I${top_srcdir}/src/mpid/common/sock   \
    -I${top_builddir}/src/mpid/common/sock

noinst_HEADERS += src/mpid/common/sock/mpidu_sock.h

include $(top_srcdir)/src/mpid/common/sock/poll/Makefile.mk

endif BUILD_MPID_COMMON_SOCK

