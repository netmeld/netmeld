#!/bin/bash --

# =============================================================================
# Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
# (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# =============================================================================
# Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
# =============================================================================

. make_common-functions;
configure_vars;

# Create package dir
mkdir -p "${debFolderName}";
(
  cd "$debFolderName";

  # Create deb control files
  (
    mkdir -m 0755 -p DEBIAN;
    cd DEBIAN;

    # Create control file
    (
     cat > control <<EOF
Source: ${toolName}
Section: utilities
Priority: optional
Maintainer: Netmeld@sandia.gov
Standards-Version: 3.9.4
Package: ${toolName}
Version: $debVersion
Architecture: amd64
Depends: debconf, \
  libpqxx-6.4, \
  libboost-date-time1.67.0, libboost-iostreams1.67.0, \
    libboost-program-options1.67.0, libboost-system1.67.0, \
    libboost-test1.67.0, libboost-regex1.67.0
Description: Netmeld core libraries
 The core libraries and headers necessary to run the Netmeld tool suite or
 expand capabilities through tool development.

EOF
    );

    # Create templates file

    # Create config script
  );

  # Do work
  (build_test_install "${toolName}");

  # Install related scripts
  (
    cd DEBIAN;

    # Create preinst script

    # Create postinst script
    (
     cat > postinst <<EOF
#!/bin/sh

# Source debconf library
. /usr/share/debconf/confmodule;

# Configure default DB, based on answer
db_get ${toolName}/config_db;
(
  # Ensure our new library is linked appropriately
  /sbin/ldconfig;
) > /dev/null 2>&1;
exit 0;
EOF
    );
    chmod 0755 postinst;

    # Create prerm script

    # Create postrm script
  );
);

# Build the deb package
dpkg-deb --build $debFolderName;
