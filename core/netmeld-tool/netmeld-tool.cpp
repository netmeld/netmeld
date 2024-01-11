#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <regex>

// Function prototypes
void handleDatabase(const std::string& subcommand);
void handleDatalake(const std::string& subcommand);
void handleFetchers();
void handlePlaybook();
void handleTool();

// Structs
// This can be a dynamic struct with a cmake in the future

#include <unordered_map>
#include <string>

bool isValidFilePath(const std::string& path) {
    // Use a simple regex to check if the path looks like a file path
    // Adjust the regex based on your specific requirements
    std::regex filePathRegex(R"([/\\]([^/\\]+[/\\])+([^/\\]+))");
    return std::regex_match(path, filePathRegex);
}

bool isValidDeviceId(const std::string& deviceId) {
    // Use a regex to check if the device ID is a number
    return std::regex_match(deviceId, std::regex("\\d+"));
}

int main(int argc, char* argv[]) {
    //Customized helps for each level of argument missing
    // if (argc < 6) {
    //     fprintf(stderr,
	// 	"Usage: netmeld [ OPTIONS ] COMMAND { COMMAND | help }\n"
	// 	"where  COMMAND := { database | datalake | fetchers | playbook | tool }\n"
	// 	"       OPTIONS := { --device-id --data-path }|\n"
    //     );
    //     return 1;
    // }
    if (argc < 6) {
        fprintf(stderr,
		"Usage: netmeld [ OPTIONS ] COMMAND { COMMAND | help }\n"
		"where  COMMAND := { database | datalake | fetchers | playbook | tool }\n"
		"       OPTIONS := { --device-id --data-path }|\n"
        );
        return 1;
    }

    // Mappings
    std::map<std::string, std::string> importSubcommandMap = {
        {"apk", "/usr/local/bin/nmdb-import-apk"},
        {"aws-ec2-describe-instances", "/usr/local/bin/nmdb-import-aws-ec2-describe-instances"},
        {"aws-ec2-describe-network-acls", "/usr/local/bin/nmdb-import-aws-ec2-describe-network-acls"},
        {"aws-ec2-describe-network-interfaces", "/usr/local/bin/nmdb-import-aws-ec2-describe-network-interfaces"},
        {"aws-ec2-describe-route-tables", "/usr/local/bin/nmdb-import-aws-ec2-describe-route-tables"},
        {"aws-ec2-describe-security-groups", "/usr/local/bin/nmdb-import-aws-ec2-describe-security-groups"},
        {"aws-ec2-describe-subnets", "/usr/local/bin/nmdb-import-aws-ec2-describe-subnets"},
        {"aws-ec2-describe-transit-gateway-attachments", "/usr/local/bin/nmdb-import-aws-ec2-describe-transit-gateway-attachments"},
        {"aws-ec2-describe-vpc-peering-connections", "/usr/local/bin/nmdb-import-aws-ec2-describe-vpc-peering-connections"},
        {"aws-ec2-describe-vpcs", "/usr/local/bin/nmdb-import-aws-ec2-describe-vpcs"},
        {"brocade", "/usr/local/bin/nmdb-import-brocade"},
        {"cisco", "/usr/local/bin/nmdb-import-cisco"},
        {"cisco-show-ip-route", "/usr/local/bin/nmdb-import-cisco-show-ip-route"},
        {"cisco-wireless", "/usr/local/bin/nmdb-import-cisco-wireless"},
        {"clw", "/usr/local/bin/nmdb-import-clw"},
        {"dig", "/usr/local/bin/nmdb-import-dig"},
        {"dig", "/usr/local/bin/nmdb-import-dig"},
        {"dpkg", "/usr/local/bin/nmdb-import-dpkg"},
        {"f5-json", "/usr/local/bin/nmdb-import-f5-json"},
        {"hosts", "/usr/local/bin/nmdb-import-hosts"},
        {"hosts", "/usr/local/bin/nmdb-import-hosts"},
        {"ip-addr-show", "/usr/local/bin/nmdb-import-ip-addr-show"},
        {"juniper-set", "/usr/local/bin/nmdb-import-juniper-set"},
        {"juniper-show-route", "/usr/local/bin/nmdb-import-juniper-show-route"},
        {"nessus", "/usr/local/bin/nmdb-import-nessus"},
        {"nmap", "/usr/local/bin/nmdb-import-nmap"},
        {"paloalto-xml", "/usr/local/bin/nmdb-import-paloalto-xml"},
        {"pcap", "/usr/local/bin/nmdb-import-pcap"},
        {"ping", "/usr/local/bin/nmdb-import-ping"},
        {"ping", "/usr/local/bin/nmdb-import-ping"},
        {"powerconnect", "/usr/local/bin/nmdb-import-powerconnect"},
        {"prowler", "/usr/local/bin/nmdb-import-prowler"},
        {"rpm", "/usr/local/bin/nmdb-import-rpm"},
        {"rpm-query", "/usr/local/bin/nmdb-import-rpm-query"},
        {"show-cdp-neighbor", "/usr/local/bin/nmdb-import-show-cdp-neighbor"},
        {"show-mac-address-table", "/usr/local/bin/nmdb-import-show-mac-address-table"},
        {"show-neighbor", "/usr/local/bin/nmdb-import-show-neighbor"},
        {"systeminfo", "/usr/local/bin/nmdb-import-systeminfo"},
        {"tshark", "/usr/local/bin/nmdb-import-tshark"},
        {"traceroute", "/usr/local/bin/nmdb-import-traceroute"},
        {"vyos", "/usr/local/bin/nmdb-import-vyos"}
    };

    std::map<std::string, std::string> insertSubcommandMap = {
        {"ac", "/usr/local/bin/nmdb-insert-ac"},
        {"address", "/usr/local/bin/nmdb-insert-address"},
        {"device-hardware", "/usr/local/bin/nmdb-insert-device-hardware"},
        {"device", "/usr/local/bin/nmdb-insert-device"}
    };

    std::map<std::string, std::string> exportSubcommandMap = {
        {"port-list", "/usr/local/bin/nmdb-export-port-list"},
        {"query", "/usr/local/bin/nmdb-export-query"},
        {"scans", "/usr/local/bin/nmdb-export-scans"}
    };

    std::map<std::string, std::string> graphSubcommandMap = {
        {"ac", "/usr/local/bin/nmdb-graph-ac"},
        {"aws", "/usr/local/bin/nmdb-graph-aws"},
        {"network", "/usr/local/bin/nmdb-graph-network"}
    };

    std::map<std::string, std::string> dbSubCommandMap = {
        {"analyze-data", "/usr/local/bin/nmdb-analyze-data"},
        {"acls", "/usr/local/bin/nmdb-acls"},
        {"initialize", "/usr/local/bin/nmdb-initialize"},
        {"remove-tool-run", "/usr/local/bin/nmdb-remove-tool-run"}
    };

    std::string command = argv[1];
    std::string dataPath = argv[4];
    std::string deviceId = argv[6];

    // Validate data path
    if (!isValidFilePath(dataPath)) {
        std::cerr << "Invalid data path: " << dataPath << "\n";
        return 1;
    }

    // Validate device ID
    if (!isValidDeviceId(deviceId) || deviceId == "") {
        std::cerr << "Invalid device ID: " << deviceId << "\n";
        return 1;
    }
    if (command == "database") {
        if (argc < 3) {
            std::cerr << "Usage: netmeld database <subcommand>\n";
            return 1;
        }
        std::string subcommand = argv[2];
        if(subcommand == "import") {
            std::string subsubcommand = argv[3];
            auto it = importSubcommandMap.find(subsubcommand);
            if (it != importSubcommandMap.end()) {
                std::string command = it->second + " --data-path " + dataPath + " --device-id " + deviceId;
                std::cout << command;
                std::system(command.c_str());
            } else {
                std::cerr << "Unknown 'database import' subcommand: " << subsubcommand << "\n";
            }
        } else if (subcommand == "insert") {
            std::string subsubcommand = argv[3];
            auto it = insertSubcommandMap.find(subsubcommand);
            if (it != insertSubcommandMap.end()) {
                std::system(it->second.c_str());
            } else {
                std::cerr << "Unknown 'database insert' subcommand: " << subsubcommand << "\n";
            }
        } else if (subcommand == "export") {
            std::string subsubcommand = argv[3];
            auto it = exportSubcommandMap.find(subsubcommand);
            if (it != exportSubcommandMap.end()) {
                std::system(it->second.c_str());
            } else {
                std::cerr << "Unknown 'database export' subcommand: " << subsubcommand << "\n";
            }
        } else if (subcommand == "graph") {
            std::string subsubcommand = argv[3];
            auto it = graphSubcommandMap.find(subsubcommand);
            if (it != graphSubcommandMap.end()) {
                std::system(it->second.c_str());
            } else {
                std::cerr << "Unknown 'database graph' subcommand: " << subsubcommand << "\n";
            }
        } else if (subcommand == "analyze-data" || subcommand == "acls" || subcommand == "initialize" || subcommand == "remove-tool-run") {
            auto it = dbSubCommandMap.find(subcommand);
            if (it != dbSubCommandMap.end()) {
                std::system(it->second.c_str());
            } else {
                std::cerr << "Unknown 'database other' subcommand: " << subcommand << "\n";
            }
        } else {
            std::cerr << "Unknown 'database' subcommand: " << subcommand << "\n";
        }
    } else if (command == "datalake") {
        if (argc < 3) {
            std::cerr << "Usage: netmeld datalake <subcommand>\n";
            return 1;
        }
        // handleDatalake(argv[2]);
    } else if (command == "fetchers") {
        // handleFetchers();
    } else if (command == "playbook") {
        // handlePlaybook();
    } else if (command == "tool") {
        // handleTool();
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }


    return 0;
}
