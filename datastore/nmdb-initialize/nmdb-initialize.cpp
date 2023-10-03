// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#include <regex>

#include <pqxx/pqxx>

#include <netmeld/datastore/tools/AbstractDatastoreTool.hpp>

namespace nmdt = netmeld::datastore::tools;
namespace nmcu = netmeld::core::utils;


class Tool : public nmdt::AbstractDatastoreTool
{
  private:
    nmcu::FileManager& nmfm {nmcu::FileManager::getInstance()};

    const std::string POSTGRES_DB_NAME {"postgres"};
    const std::string MAC_PREFIX_FILE  {"/usr/share/nmap/nmap-mac-prefixes"};
    const std::string SCHEMA_PATH      {nmfm.getConfPath().string()
                                       + '/' + NETMELD_SCHEMA_DIR};

  public:
    Tool() : nmdt::AbstractDatastoreTool
      ("(Re-)Initialize database to an empty state",
       PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      opts.addRequiredOption("schema-dir", std::make_tuple(
            "schema-dir",
            po::value<std::string>()->default_value(SCHEMA_PATH),
            "Directory of Netmeld .sql files to use as the database schema")
          );
      opts.addRequiredOption("mac-prefix-file", std::make_tuple(
            "mac-prefix-file",
            po::value<std::string>()->default_value(MAC_PREFIX_FILE),
            "Location of mac prefix file")
          );

      opts.addOptionalOption("delete", std::make_tuple(
            "delete,d",
            NULL_SEMANTIC,
            "Delete all existing data in DB, not a full drop and recreate")
          );
      opts.addOptionalOption("extra-schema", std::make_tuple(
            "extra-schema",
            po::value<std::vector<std::string>>()->multitoken(),
            "Additional .sql files to populate the database with")
          );
    }

    int
    runTool() override
    {
      const auto& dbConnectString {getDbConnectString()};
      const auto shouldDelete     {opts.exists("delete")};

      // Initialize DB to consistent state
      initDbState(dbConnectString, shouldDelete);

      LOG_DEBUG << "(runTool) dbConnectString: " << dbConnectString
                << std::endl;
      pqxx::connection db {dbConnectString};
      pqxx::work work     {db};

      // Populate target DB with schema(s)
      if (!shouldDelete) {
        loadSchema(work);
      }

      // Load targeted MAC prefixes
      loadMacPrefixes(work);

      work.commit();

      return nmcu::Exit::SUCCESS;
    }

    void
    initDbState(const std::string& dbConnectString, bool shouldDelete)
    {
      const auto& dbName  {getDbName()};

      const auto& pgDbConnectString
        {"dbname=" + POSTGRES_DB_NAME + ' ' + getDbArgs()};

      LOG_DEBUG << "postgresDb: " << pgDbConnectString << std::endl;
      pqxx::connection postgresDb {pgDbConnectString};

      try {
        // If DB already exists, either quick or full erase
        LOG_DEBUG << "existsDb: " << dbConnectString << std::endl;
        pqxx::connection existsDb {dbConnectString};
        existsDb.close();

        LOG_INFO << "Database '" << dbName << "' already exists.\n"
                 << "Re-initialize the database [y/n]?";
        char response;
        std::cin >> response;
        if (response != 'y' && response != 'Y') {
          LOG_INFO << "Database NOT re-initialized by user\n";
          std::exit(nmcu::Exit::USER_ABORTED);
        }

        if (shouldDelete) {
          LOG_DEBUG << "db: " << dbConnectString << std::endl;
          pqxx::connection db         {dbConnectString};
          pqxx::nontransaction ntWork {db};

          LOG_INFO << "Cleaning tool_runs\n";
          ntWork.exec("TRUNCATE FROM tool_runs RESTART IDENTITY CASCADE");

          pqxx::result tables = ntWork.exec(
              "SELECT relname AS populated_table"
              " FROM pg_catalog.pg_stat_user_tables"
              " WHERE n_live_tup > 0"
              );
          for (const auto& tableRow : tables) {
            std::string populatedTable;
            tableRow.at("populated_table").to(populatedTable);
            LOG_DEBUG << "Cleaning " << populatedTable << std::endl;
            ntWork.exec("TRUNCATE FROM "
                       + populatedTable
                       + " RESTART IDENTITY CASCADE"
                       );
          }
          return; // short circuit, easier logic

        } else {
          pqxx::nontransaction ntWork{postgresDb};
          LOG_INFO << "Dropping database '" << dbName << "'..." << std::endl;
          try {
            ntWork.exec("DROP DATABASE IF EXISTS " + dbName);
          } catch (std::exception& e) {
            LOG_ERROR << "Failed to drop database: " << dbName
                      << '\n' << e.what()
                      << std::endl;
            std::exit(nmcu::Exit::FAILURE); // Can't delete DB, probaly open
          }
        }
      }
      // If DB doesn't exists, nothing special
      catch (const std::exception& e) { }

      pqxx::nontransaction ntWork{postgresDb};
      LOG_INFO << "Creating database '" << dbName << "'..." << std::endl;
      ntWork.exec("CREATE DATABASE " + dbName);

      postgresDb.close();
    }

    void
    loadSchema(pqxx::work& work)
    {
      std::vector<std::string> schemas;

      auto schemaDir {opts.getValue("schema-dir")};

      if (!schemaDir.empty()) {
        for (auto& pathIter : sfs::recursive_directory_iterator(schemaDir)) {
          auto& path {pathIter.path()};
          if (sfs::is_regular_file(path) && ".sql" == path.extension()) {
            schemas.push_back(path.string());
          }
        }
      }
      auto const& extra {opts.getValues("extra-schema")};
      schemas.insert(schemas.end(), extra.begin(), extra.end());

      LOG_DEBUG << "Sorting schema imports" << std::endl;
      std::sort(schemas.begin(), schemas.end());

      LOG_INFO << "Importing schema(s)" << std::endl;
      for (const auto& schema : schemas) {
        importFile(work, schema);
      }
    }

    void
    importFile(pqxx::work& work, std::string filename)
    {
      if (!sfs::exists(sfs::path(filename))) {
        LOG_WARN << "File not found: " << filename << std::endl;
        return;
      }

      LOG_DEBUG << "Processing file: " << filename << std::endl;
      std::ifstream sqlfile(filename);
      std::ostringstream statement;
      statement << sqlfile.rdbuf();
      work.exec(statement.str());
    }

    void
    loadMacPrefixes(pqxx::work& work)
    {
      const auto macPrefixPath = opts.getValue("mac-prefix-file");

      if (!sfs::exists(sfs::path(macPrefixPath))) {
        LOG_WARN << "MAC prefix file not found: " << macPrefixPath << std::endl;
        return;
      }

      std::map<std::string, std::string> macs;
      std::ifstream macPrefixStream(macPrefixPath);
      std::string line;

      LOG_DEBUG << "Parsing MAC prefixes" << std::endl;

      std::regex macRegex("^[0-9A-F]{6} .*");
      std::regex trim("^[ ]+|[ ]+$");
      while(std::getline(macPrefixStream, line)) {
        // Only process MAC Vendor lines
        if (!std::regex_match(line, macRegex)) { continue; }

        std::istringstream splitter(line);
        std::string mac, vendor;

        splitter >> mac;
        std::getline(splitter, vendor);
        vendor = std::regex_replace(vendor, trim, ""); // trim whitespace

        if (macs.count(mac)) {
          macs[mac] = macs[mac] + " | " + vendor;
        }
        else {
          macs[mac] = vendor;
        }
      }

      LOG_INFO << "Inserting MAC prefixes" << std::endl;
      auto stream = pqxx::stream_to::table(
          work
        , "vendor_mac_prefixes"
        , std::vector<std::string>{"mac_prefix", "vendor_name"}
        );
      for (const auto& entry : macs) {
        stream << std::make_tuple(entry.first + ":000000", entry.second);
      }
      stream.complete();
    }
};

int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
