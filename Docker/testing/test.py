#!/usr/bin/python3

import os
import subprocess
import sys
import shlex
import logging

from re import sub


def run_command(command):
  args = {}
  args["capture_output"] = True
  args["args"] = shlex.split(command)
  
  logging.info("Testing with: %s", command)
  r = subprocess.run(**args)
  output = str(r.stdout, "utf8")
  err = str(r.stderr, "utf8")
  rtc = r.returncode
  logging.debug("out: %s", output)
  logging.debug("err: %s", err)
  logging.debug("rtc: %d", rtc)
  if 0 != rtc:
    logging.info("======================")
    logging.info("Command returned unexpected status: %d", rtc)
    logging.info("ERR: %s", err)
    logging.info("======================")

  return 0 == rtc


def run_commands(commands):
  allSuccess = True
  for command in commands:
    allSuccess = run_command(command) and allSuccess

  if allSuccess:
    logging.info("All commands ran successfully")
    return 0
  else:
    logging.info("Some command failed")
    return 1


def test_nmTools():
  commands = [
    "clw ls",
  ]

  return run_commands(commands)



def test_nmDatalake():
  commands = [
    "nmdl-initialize",
    "git config --global user.email 'test@localhost'",
    "git config --global user.name 'test'",
    "nmdl-insert --device-id test --tool nmdb-import-hosts /etc/hosts",
    "nmdl-list",
    "nmdl-remove --device-id test",
  ]

  return run_commands(commands)


def test_nmDatastore():
  commands = [
    "echo 'yes' | nmdb-initialize",
    # Other
    "nmdb-analyze-data --example",
    "nmdb-convert-acls",
    # Exporters
    "nmdb-export-port-list -TUYD",
    "nmdb-export-query -q 'select * from tool_runs'",
    "nmdb-export-scans --intra-network",
    # Importers
    "nmdb-import-aws-ec2-describe-instances --device-id test d1",
    "nmdb-import-aws-ec2-describe-network-acls --device-id test d1",
    "nmdb-import-aws-ec2-describe-route-tables --device-id test d1",
    "nmdb-import-aws-ec2-describe-security-groups --device-id test d1",
    "nmdb-import-aws-ec2-describe-subnets --device-id test d1",
    "nmdb-import-aws-ec2-describe-vpcs --device-id test d1",
    "nmdb-import-brocade --device-id test d1",
    "nmdb-import-cisco --device-id test d1",
    "nmdb-import-cisco-show-ip-route --device-id test /dev/null",
    "nmdb-import-cisco-wireless --device-id test d1",
    #"nmdb-import-clw --device-id test d0",
    #"nmdb-import-dig --device-id test d0",
    #"nmdb-import-f5-json --device-id test d0",
    "nmdb-import-hosts --device-id test d1",
    "nmdb-import-ip-addr-show --device-id test /dev/null",
    #"nmdb-import-ipconfig --device-id test d0",
    #"nmdb-import-ip-route-show --device-id test d0",
    #"nmdb-import-iptables-save --device-id test d0",
    "nmdb-import-juniper-conf --device-id test /dev/null",
    "nmdb-import-juniper-set --device-id test d1",
    "nmdb-import-juniper-show-route --device-id test /dev/null",
    "nmdb-import-juniper-xml --device-id test d2",
    #"nmdb-import-nessus --device-id test d0",
    #"nmdb-import-nmap --device-id test d0",
    "nmdb-import-paloalto-xml --device-id test d2",
    "nmdb-import-pcap --device-id test d4",
    "nmdb-import-ping --device-id test d1",
    "nmdb-import-powerconnect --device-id test d1",
    "nmdb-import-prowler --device-id test d1",
    "nmdb-import-show-cdp-neighbor --device-id test d1",
    "nmdb-import-show-inventory --device-id test d1",
    "nmdb-import-show-mac-address-table --device-id test /dev/null",
    "nmdb-import-show-neighbor --device-id test d1",
    "nmdb-import-traceroute --device-id test /dev/null",
    "nmdb-import-tshark d3",
    "nmdb-import-vyos --device-id test d1",
    # Inserters
    "nmdb-insert-ac --device-id test",
    "nmdb-insert-address --device-id test",
    "nmdb-insert-device --device-id test",
    "nmdb-insert-device-hardware --device-id test",
    "nmdb-insert-network",
    # Graphers; Tested later so data to pull exists
    "nmdb-graph-ac --device-id test",
    "nmdb-graph-network --device-id test --layer 3",
    # Other
    "nmdb-remove-tool-run 12345678-1234-1234-1234-123456789012",
  ]

  return run_commands(commands)


def test_nmFetchers():
  commands = [
    "echo 'yes' | nm-fetch-ansible",
    "nm-fetch-ssh root@localhost ls",
  ]

  return run_commands(commands)


def test_nmPlaybook():
  commands = [
    "nmdb-playbook-insert-router --ip-addr 10.0.0.1",
    "nmdb-playbook-insert-source  --inter-network --interface test1 --ip-addr 10.0.0.10 --stage 1",
    "nmdb-playbook --inter-network",
    #"nmdb-playbook-nessus",
    "cisco-type7-decode --password 046E1803362E595C260E0B240619050A2D",
    "junos-type9-decode --password '$9$EeDcKWxNb4oGuOWxNd4oz36A01reW-VY5QclvM-daZUi.5/9p'",
  ]
  return run_commands(commands)


def main():
  #logging.basicConfig(level=logging.DEBUG)
  logging.basicConfig(level=logging.INFO)
  test_nmTools()
  test_nmDatalake()
  test_nmDatastore()
  test_nmFetchers()
  test_nmPlaybook()

  return 0


if __name__ == "__main__":
    ret_code = main()
    sys.exit(ret_code)
