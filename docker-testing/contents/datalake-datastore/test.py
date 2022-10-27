#!/usr/bin/python3

import os
import subprocess
import sys

sys.path.append("..")
from  run_command import run_command

def main():

    device_id = "--device-id=netmeld_test"
    
    blank_filename = "blank"

    commands = [
        {
            "args": ["nmdb-initialize"],
            "input": bytes("y\n", "ascii")
        }
    ]

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-brocade", device_id, "--pipe", blank_filename], 
            "input":  bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-brocade-show-ip-route", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-cisco", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    commands.append(
        {
            "args": ["nmdb-import-cisco-show-ip-route", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    
    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-cisco-wireless", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-clw", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-dig", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-f5-json", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-hosts", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-ipconfig", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    commands.append(
        {
            "args": ["nmdb-import-ip-addr-show", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-ip-route-show", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-iptables-save", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-juniper-conf", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-juniper-set", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-juniper-show-route", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-juniper-xml", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-nessus", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-nmap", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-paloalto-xml", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-pcap", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-ping", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-powerconnect", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-show-cdp-neighbor", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-show-inventory", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-show-mac-address-table", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-show-neighbor", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-traceroute", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-tshark", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-vyos", device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-insert-ac", device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-address", device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-device", device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-device-hardware", device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-network", device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdl-initialize", device_id], 
            "input": bytes("y", "ascii")
        }
    )

    commands.append(
        {
            "args": ["git", "config", "--global", "user.email", "test@test.test"]
        }
    )

    commands.append(
        {
            "args": ["git", "config", "--global", "user.name", "test"]
        }
    )

    commands.append(
        {
            "args": ["nmdl-insert", device_id, "--pipe", "--rename", "test"],
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdl-list", device_id]
        }
    )

    commands.append(
        {
            "args": ["nmdl-remove", device_id]
        }
    )

    commands.append(
        {
            "args": ["nmdb-export-port-list", device_id]
        }
    )

    commands.append(
        {
            "args": ["nmdb-export-query", device_id, "-q", "select * from ip_addrs"]
        }
    )

    commands.append(
        {
            "args": ["nmdb-graph-ac", device_id]
        }
    )

    commands.append(
        {
            "args": ["nmdb-graph-network", "--layer", "3", device_id]
        }
    )

    commands.append(
        {
            "args": ["nmdb-analyze-data", "--example"]
        }
    )


    commands.append(
        {
            "args": ["nmdb-convert-acls"]
        }
    )

    commands.append(
        {
            "args": ["nmdb-remove-tool-run", "12345678-1234-1234-1234-123456789012"]
        }
    )


    res = True

    for command in commands:
        print()
        res = run_command(**command) and res
        print()
        try:
            os.remove(blank_filename)
        except OSError as error:
            pass

    if res:
        print("All commands returned expected values")
        print()
        return 0
    print("A command returned an unexpected value")
    print()
    return 1

if __name__ == "__main__":
    ret_code = main()
    sys.exit(ret_code)