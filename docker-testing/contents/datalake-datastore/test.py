#!/usr/bin/python

import os
import subprocess
import sys

sys.path.append("..")
from  run_command import run_command

def main():

    db_args = f'--db_args="host={os.environ["NETMELD_DB_HOST"]} user={os.environ["NETMELD_DB_USER"]} password={os.environ["NETMELD_DB_PASS"]}"'
    db_name = f'--db_name="{os.environ["NETMELD_DB"]}"'
    device_id = "--device-id=netmeld_test"

    subprocess.run(args=["service", "postgresql", "start" ])
    
    blank_filename = "blank"

    commands = [
        {
            "args": ["nmdb-initialize", db_args, db_name],
            "input": bytes("y\n", "ascii")
        }
    ]

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-brocade", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input":  bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-brocade-show-ip-route", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-cisco", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    commands.append(
        {
            "args": ["nmdb-import-cisco-show-ip-route", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    
    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-cisco-wireless", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-clw", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-dig", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-f5-json", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-hosts", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-ipconfig", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """
    
    commands.append(
        {
            "args": ["nmdb-import-ip-addr-show", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-ip-route-show", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-iptables-save", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-juniper-conf", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-juniper-set", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-juniper-show-route", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-juniper-xml", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-nessus", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-nmap", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-paloalto-xml", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-pcap", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-ping", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-powerconnect", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-show-cdp-neighbor", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-show-inventory", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-show-mac-address-table", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-show-neighbor", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-import-traceroute", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-tshark", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    """
    Does not accept empty input
    commands.append(
        {
            "args": ["nmdb-import-vyos", db_args, db_name, device_id, "--pipe", blank_filename], 
            "input": bytes("", "ascii")
        }
    )
    """

    commands.append(
        {
            "args": ["nmdb-insert-ac", db_args, db_name, device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-address", db_args, db_name, device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-device", db_args, db_name, device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-device-hardware", db_args, db_name, device_id], 
            "input": bytes("", "ascii")
        }
    )

    commands.append(
        {
            "args": ["nmdb-insert-network", db_args, db_name, device_id], 
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
            "args": ["nmdb-export-query", device_id, "-q", "\"select * from ip_addrs\""]
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