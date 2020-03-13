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

doWork="doWork";

processConnection()
{
  # General variables used to provide feedback and consistency
  session_id="sessionid123";
  scan_id="scanid123";
  scan_uuid="scanuuid123";
  file_prefix="fileid";
  # append random number to support multi-format requests
  file_id="${file_prefix}$RANDOM";

  # More explicit tests would match on more than just the GET/POST/DELETE line
  #   We don't need more explicity tests, yet
  while read line; do
    # Session login
    if [[ ${line} =~ ^POST\ /session\ .*$ ]]; then
      echo -n '{"token":"'${session_id}'"}';
      return;
    fi;

    # All policy list
    if [[ ${line} =~ ^GET\ /policies\ .*$ ]]; then
      echo -n '{"policies":[{"name":"Kitchen Sink + VMware Compliance - Linux","template_uuid":"123456","id":"12"}]}';
      return;
    fi;

    # Create scan
    if [[ ${line} =~ ^POST\ /scans\ .*$ ]]; then
      echo -n '{"scan":{"id":"'${scan_id}'"}}';
      return;
    fi;

    # Launch scan
    if [[ ${line} =~ ^POST\ /scans/"${scan_id}"/launch\ .*$ ]]; then
      echo -n '{"scan_uuid":"'${scan_uuid}'"}';
      return;
    fi;

    # Scan completed
    if [[ ${line} =~ ^GET\ /scans/"${scan_id}"\ .*$ ]]; then
      echo -n '{"history":[{"status":"completed"}]}';
      return;
    fi;

    # Export scan
    if [[ ${line} =~ ^POST\ /scans/"${scan_id}"/export\ .*$ ]]; then
      echo -n '{"file":"'${file_id}'"}';
      return;
    fi;

    # Export complete
    if [[ ${line} =~ ^GET\ /scans/"${scan_id}"/export/"${file_prefix}"[[:digit:]]+/status\ .*$ ]]; then
      echo -n '{"status":"ready"}';
      return;
    fi;

    # Download
    if [[ ${line} =~ ^GET\ /scans/"${scan_id}"/export/"${file_prefix}"[[:digit:]]+/download\ .*$ ]]; then
      echo -n 'downloaded';
      return;
    fi;

    # Logout
    if [[ ${line} =~ ^DELETE\ /session\ .*$ ]]; then
      echo -n '';
      return;
    fi;

    # Unknown input, just echo it back
    echo $line;
    return;
  done;
}

# Create ncat listener to call itself in testing mode, listen forever
listen()
{
  echo "Server listening...";
  ncat --ssl -kl -p 8834 -e '/bin/bash '${0}' '${doWork};
}

# Main
{
  if [ "$1" == "${doWork}" ]; then
    processConnection;
  else
    listen;
  fi;
}
