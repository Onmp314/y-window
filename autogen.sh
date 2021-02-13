#!/usr/bin/env bash

set -e

if [ -f development ]
then
    # Symlinks
    ${ACLOCAL:-aclocal} -I autoconf
    ${AUTOHEADER:-autoheader}
    ${LIBTOOLIZE:-libtoolize} --automake --force
    build-misc/yclmakemake Y/Makefile.ycl
    ${AUTOMAKE:-automake} -a
    ${AUTOCONF:-autoconf}
else
    # No symlinks
    ${ACLOCAL:-aclocal} -I autoconf
    ${AUTOHEADER:-autoheader}
    ${LIBTOOLIZE:-libtoolize} --automake --copy --force
    build-misc/yclmakemake Y/Makefile.ycl
    ${AUTOMAKE:-automake} -a -c
    ${AUTOCONF:-autoconf}
fi

# If it exists and is executable, recheck and regenerate
test -x config.status && ./config.status --recheck
test -x config.status && ./config.status

# Exit true if we got this far
exit 0

# arch-tag: bfde86b9-5804-4d32-a891-c18349a36866
