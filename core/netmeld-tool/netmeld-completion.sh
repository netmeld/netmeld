# _netmeld_completion Bash completion

_netmeld_completion() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    prev2="${COMP_WORDS[COMP_CWORD-2]}"

    if [[ ${prev} == *"netmeld-tool"* ]]; then
        opts="database playbook tool"
        COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
        return 0
    fi

    if [[ ${prev} == "database" ]]; then
        sub_opts="import insert export graph analyze-data acls initialize remove-tool-run"
        COMPREPLY=($(compgen -W "${sub_opts}" -- ${cur}))
        return 0
    fi

    if [[ ${prev} == "import" && ${prev2} == "database" ]]; then
        import_sub_opts="apk juniper-set aws-ec2-describe-instances juniper-show-route aws-ec2-describe-network-acls juniper-xml aws-ec2-describe-network-interfaces nessus aws-ec2-describe-route-tables nmap aws-ec2-describe-security-groups paloalto-xml aws-ec2-describe-subnets pcap aws-ec2-describe-transit-gateway-attachments ping aws-ec2-describe-vpc-peering-connections powerconnect aws-ec2-describe-vpcs prowler brocade brocade-show-ip-route cisco cisco-show-ip-route cisco-wireless clw dig dpkg f5-json hosts ip-addr-show rpm rpm-query show-cdp-neighbor show-inventory show-mac-address-table show-neighbor systeminfo traceroute vyos tshark"
        COMPREPLY=($(compgen -W "${import_sub_opts}" -- ${cur}))
        return 0
    fi
# Completion for after import <command>
    if [[ ${prev2} == "import" && ${prev} != "database" && ${prev} != "" ]]; then
        COMPREPLY=($(compgen -f -- ${cur}))
        return 0
    fi
    if [ -f "$(eval echo "$prev")" ]; then
        COMPREPLY=( $(compgen -W "--device-id" -- ${cur}) )
        return 0
    fi
# Completion for data-path flag
    if [[ ${prev} == "--data-path" ]]; then
        COMPREPLY=($(compgen -f -- ${cur}))
        return 0
    fi

    if [[ ${prev} == "insert" && ${prev2} == "database" ]]; then
        import_sub_opts="ac address device-hardware device network"
        COMPREPLY=($(compgen -W "${import_sub_opts}" -- ${cur}))
        return 0
    fi

    if [[ ${prev} == "export" && ${prev2} == "database" ]]; then
        import_sub_opts="port-list query scans"
        COMPREPLY=($(compgen -W "${import_sub_opts}" -- ${cur}))
        return 0
    fi

    if [[ ${prev} == "graph" && ${prev2} == "database" ]]; then
        import_sub_opts="ac aws network"
        COMPREPLY=($(compgen -W "${import_sub_opts}" -- ${cur}))
        return 0
    fi
    # Add more if statements for additional levels or commands

    # Default case for unrecognized commands
    return 0
}

complete -F _netmeld_completion netmeld-tool