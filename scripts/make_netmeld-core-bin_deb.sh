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
  netmeld-core, \
  postgresql (>= 11), postgresql-client (>= 11)
Recommends: \
  nmap
Suggests: \
  postgresql-contrib (>= 11), postgresql-autodoc (>= 11)
Description: Netmeld core binaries
 The core binaries necessary to manipulated the Netmeld data store.

EOF
    );

    # Create templates file
    (
     cat > templates <<EOF
Template: ${toolName}/config_db
Type: boolean
Default: true
Description: Configure the default superuser and database with Netmeld schemas?
 Creates the default users, root and netmeld, in the PSQL database as a superuser
 and installs the Netmeld schemas to the default, site, database. Note: This
 will remove any existing default information.
EOF
    );

    # Create config script
    (
     cat > config <<EOF
#!/bin/sh -e

# Source debconf library
. /usr/share/debconf/confmodule;

# Ask question(s)
db_input medium ${toolName}/config_db || true;

# Show interface
db_go || true;
EOF
    );
    chmod 0755 config;
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
  if [ "false" = "\$RET" ]; then
    return 0;
  fi;

  # Ensure DB starts on system start
  /usr/bin/systemctl enable postgresql;
  /usr/bin/systemctl restart postgresql;

  # Needed for if no systemctl available
  hardStart=false;
  if [ ! -z "\$(pg_isready | grep 'no response')" ]; then
    /etc/init.d/postgresql start;
    hardStart=true;
  fi;

  # Add users to DB
  su -c "/usr/bin/dropdb --if-exists site > /dev/null 2>&1;" \\
    -s /bin/sh postgres;
  su -c "/usr/bin/dropuser --if-exists root > /dev/null 2>&1;" \\
    -s /bin/sh postgres;
  su -c "/usr/bin/createuser -s root > /dev/null 2>&1;" \\
    -s /bin/sh postgres;
  su -c "/usr/bin/dropuser --if-exists netmeld > /dev/null 2>&1;" \\
    -s /bin/sh postgres;
  su -c "/usr/bin/createuser -s netmeld > /dev/null 2>&1;" \\
    -s /bin/sh postgres;
  su -c "/usr/bin/createdb site > /dev/null 2>&1;" \\
    -s /bin/sh postgres;

  yes | /usr/local/bin/nmdb-initialize;

  if [ "true" = \$hardStart ]; then
    /etc/init.d/postgresql stop;
  fi;
) > /dev/null 2>&1;
exit 0;
EOF
    );
    chmod 0755 postinst;

    # Create prerm script

    # Create postrm script
    (
     cat > postrm <<EOF
#!/bin/sh -e

if [ "\$1" = "purge" -a -e /usr/share/debconf/confmodule ]; then
  # Source debconf library
  . /usr/share/debconf/confmodule;

  # Remove changes
  db_purge;
fi
EOF
    );
    chmod 0755 postrm;
  );
);

# Build the deb package
dpkg-deb --build $debFolderName;
