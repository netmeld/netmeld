#!/bin/bash --

modules="\
  all \
  clw \
  datalake \
  datastore-all \
  datastore-db \
  datastore-tools \
  development \
  fetchers \
  playbook-nodb \
  tester \
";

for module in $modules; do
  echo "Building ${module}...";

  python3 docker-netmeld.py "${module}" \
    | docker build -t "netmeld-${module}" -f- .
done;
