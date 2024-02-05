#!/usr/bin/env bash

function create_test_data()
{
  # empty file; 2x space; newline terminated
  echo '  ' > d1;

  # xml tags
  cat << EOF > d2
<config></config>
<rpc-reply></rpc-reply>
<nmaprun></nmaprun>
EOF

  # json array
  cat << EOF > d3
[
]
EOF

  # empty packet capture
  cat << EOF | base64 -d > d4
Cg0NCrQAAABNPCsaAQAAAP//////////AgA3AEludGVsKFIpIFhlb24oUikgQ1BVIEU1LTE2MjAg
djQgQCAzLjUwR0h6ICh3aXRoIFNTRTQuMikAAwATAExpbnV4IDYuMC4wLTItYW1kNjQABAA6AER1
bXBjYXAgKFdpcmVzaGFyaykgNC4wLjEgKEdpdCB2NC4wLjEgcGFja2FnZWQgYXMgNC4wLjEtMSkA
AAAAAAC0AAAAAQAAAEAAAAABAAAAAAAEAAIAAgBsbwAACQABAAkAAAAMABMATGludXggNi4wLjAt
Mi1hbWQ2NAAAAAAAQAAAAAUAAABsAAAAAAAAACLtBQA0Wfh+AQAcAENvdW50ZXJzIHByb3ZpZGVk
IGJ5IGR1bXBjYXACAAgAIu0FAEUF6X4DAAgAIu0FAFpY+H4EAAgAAAAAAAAAAAAFAAgAAAAAAAAA
AAAAAAAAbAAAAA==
EOF

  # targeted json top levels
  cat << EOF > d5
{
  "NetworkAcls": []
, "NetworkInterfaces": []
, "Reservations": []
, "RouteTables": []
, "SecurityGroups": []
, "Subnets": []
, "TransitGatewayAttachments": []
, "VpcPeeringConnections": []
, "Vpcs": []

, "kind": ""
}
EOF

}

function run_tests()
{
  sudo service postgresql restart;
  create_test_data;
  python3 test.py || exit 1;
  sudo service postgresql stop;
}

if [ "run" = "$1" ]; then
  run_tests
  exit 0;
fi;

exec "$@";
