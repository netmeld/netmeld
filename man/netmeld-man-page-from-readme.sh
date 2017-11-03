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

################################################################################
# Targeted script to convert Netmeld readme markdown page to a man page.
################################################################################

# Globals
fileName="Netmeld";
manPage="7";
sourceMd="../README.md";
extraH2mData="Template.h2m";
targetMan="${fileName}.${manPage}";

# general usage sed and options
sedCmd="sed -i -r -e";

# pre-work
echo '' > "${targetMan}";

# add header/footer data
date="$(git log -1 --format="%ad" --date=format:'%B %Y' -- ${sourceMd})";
$sedCmd '$a\.TH '"${fileName} \"${manPage}\""' "'"$date"'" "Linux" "Overview"'\
    "${targetMan}";

# add synopsis
$sedCmd '$a\.SH SYNOPSIS' "${targetMan}";
for folder in $(find ../. -maxdepth 1 -type d \( -name 'clw' -o -name 'nm*' \)\
                     -printf "%f\n" | sort); do
  echo ".B $folder" >> "${targetMan}";
  grep -RIi -E '(Usage:|usage=)' ../$folder/* | \
    sed -r \
        -e 's/^.*PROGRAM_NAME (.*?)/\1/' \
        -e 's/^<< " //' \
        -e 's/" << endl$//' \
        -e 's/^.*\(prog\)s (.*?)/\1/' \
        -e "s/'\)$//" \
        -e 's/(\[|\{)/\1\\fI/g' \
        -e 's/(\]|\})/\\fR\1/g' \
        >> "${targetMan}";
  echo ".PP" >> "${targetMan}";
done;

# add markdown file content
cat "${sourceMd}" >> "${targetMan}";

# add man page general content
cat "${extraH2mData}" | \
  sed -r \
      -e '/=+/,/=+/ {d;}' \
      -e 's/^\[(.*?)\]$/.SH \1/' \
      >> "${targetMan}";
for folder in $(find ../. -maxdepth 1 -type d \( -name 'clw' -o -name 'nm*' \)\
                     -printf "%f\n" | sort); do
  $sedCmd '$i\.BR '"$folder"' (1),' "${targetMan}";
done;

# remove install directions
$sedCmd '/^# Installing Netmeld/,/^(# ){1}/ {
  /^# Installing Netmeld/d
  /^# /{b}
  d
}' "${targetMan}";

# Convert GitLab markdown to usable syntax for a man page

# headings
$sedCmd 's/^##+/.SS/' "${targetMan}";
$sedCmd 's/^#(.*)/.SH\U\1/' "${targetMan}";

# bulleted lists
$sedCmd 's/^\* /.IP \\[bu]\n/' "${targetMan}";
$sedCmd '/^  \*/ {
  s/^  \* /.RS\n.IP \\[bu]\n/
  :a
  n
  s/^  \* /.IP \\[bu]\n/
  /^($|\s|\*)/ {
    i\.RE 
    p
    b
  }
  ba
}' "${targetMan}";

# quotes
$sedCmd '/^>/ {
  i\.IP
  :a
  s/^> //
  n
  /^$/ {
    p
    b
  }
  ba
}' "${targetMan}";

# horizontal line
$sedCmd 's/-----//' "${targetMan}";

# inline code
$sedCmd 's/`([^`]+)`/\\fB\1\\fR/g' "${targetMan}";

# code blocks
perl -i -p -e 's{```}{++$n % 2? ".nf\n.RS" : ".RE\n.fi"}ge' "${targetMan}";

# should occur last (multi-newlines, extra whitespace, etc.)
$sedCmd '/^$/N;/^\n$/D' "${targetMan}";
$sedCmd 's/^$/.PP/' "${targetMan}";
