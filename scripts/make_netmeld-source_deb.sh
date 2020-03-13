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
Vcs-Git: $gitRepo
Vcs-browser: https://github.com/netmeld/${toolName}.git
Homepage: https://github.com/netmeld/${toolName}
Package: ${toolName}
Version: $debVersion
Architecture: amd64
Depends: debconf, \
  build-essential, cmake,
  libpqxx-dev, \
  libboost-date-time-dev, libboost-iostreams-dev, \
    libboost-program-options-dev, libboost-system-dev, \
    libboost-test-dev, libboost-regex-dev, \
  libpugixml-dev, \
  libpcap0.8-dev
Recommends: \
  git, help2man, pandoc, \
  postgresql (>= 11), postgresql-client (>= 11)
Suggests: \
  nmap, \
  postgresql-contrib (>= 11), postgresql-autodoc (>= 11)
Description: Netmeld Source Code base tool set
 System assessments typically yield large quantities of data from disparate
 sources for an analyst to scrutinize for issues.  Netmeld is used to parse
 input from different file formats, store the data in a common format, allow
 users to easily query it, and enable analysts to tie different analysis tools
 together using a common back-end.

EOF
    );

    # Create templates file

    # Create config script

    # Do work
    (
      installSrcDir="${debFolderName}/usr/local/src/netmeld";
      mkdir -p "${installSrcDir}";
      cd "${srcRoot}";

      # Recreate normal files
      find . \
        -type f \
        -not -path "./build/*" \
        -not -path "./.git/*" \
        -not -path "./.gitlab/*" \
        -not -path "./scripts/netmeld*" \
        -exec cp --parents '{}' "${installSrcDir}" \;
      # Recreate links
      find -L . \
        -xtype l \
        -not -path "./build/*" \
        -not -path "./.git/*" \
        -not -path "./.gitlab/*" \
        -not -path "./scripts/netmeld*" \
        -exec cp --parents -P '{}' "${installSrcDir}" \;
    );
  );
);

# Build the deb package
dpkg-deb --build $debFolderName;
