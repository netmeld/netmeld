// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <pqxx/pqxx>

#include <fstream>
#include <map>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::regex;
using std::regex_match;
using std::string;
using std::map;
using std::vector;

namespace program_options = boost::program_options;
namespace fs = boost::filesystem;

bool verbose = false;


bool check_exists(string db_name);

bool import_dir(pqxx::work& db, string path);

bool import_file(pqxx::work& db, string filename);

void insert_mac_prefixes(pqxx::work& db, string filename);

void log(string message);


int main(int argc, char** argv)
{
  try {
    program_options::options_description opts_desc("Options");
    opts_desc.add_options()
      ("db-name",
       program_options::value<string>()->required()->
       default_value(DEFAULT_DB_NAME),
       "Database to create or (re)initialize.")
      ("quick,q",
       program_options::bool_switch()->default_value(false),
       "Delete DB tables of existing data instead of full drop and recreate")
      ("schema-dir",
       program_options::value<string>()->required()->
       default_value(INSTALL_DIR "netmeld/schema/"),
       "Directory of Netmeld .sql files to use as the database schema.")
      ("extra-schema",
       program_options::value<vector<string> >()->multitoken(),
       "Additional .sql files to populate the database with.")
      ("mac-prefix-file",
       program_options::value<string>()->required()->
       default_value("/usr/share/nmap/nmap-mac-prefixes"),
       "Location of mac prefix file")
      ("help,h",
       "Show this help message, then exit")
      ("version,v",
       "Show version information, then exit")
      ("verbose",
       program_options::bool_switch(),
       "Verbose mode.")
      ;

    program_options::variables_map opts;
    program_options::store
      (program_options::parse_command_line(argc,argv, opts_desc), opts);

    if (opts.count("help")) {
      cerr << "Initialize a database to a default Netmeld schema." << endl
           << endl
           << "Usage: " << PROGRAM_NAME << " [options] {input_file}" << endl
           << opts_desc << endl;
      return 0;
    }
    if (opts.count("version")) {
      cerr << "nmdb-initialize (Netmeld)" << endl;
      return 0;
    }

    program_options::notify(opts);

    verbose = opts.at("verbose").as<bool>();
    string const db_name = opts.at("db-name").as<string>();
    string const schema_dir = opts.at("schema-dir").as<string>();

    
    // Determine if DB exists already and init appropriately

    pqxx::connection postgres_db("dbname=postgres");

    bool is_full = !(opts.at("quick").as<bool>());
    bool db_exists = check_exists(db_name);

    if (db_exists) {
      cout << "Database '" << db_name << "' already exists." << endl;
      cout << "Re-initialize the database [y/n]?";
      char response;
      cin >> response;
      if (response != 'y' && response != 'Y') {
        cerr << "Database NOT re-initialized" << endl;
        return 1; // User aborted
      }

      if (is_full) {
        pqxx::nontransaction nt_work{postgres_db};
        log("Dropping database '" + db_name + "'...");
        nt_work.exec("DROP DATABASE IF EXISTS " + db_name);
        db_exists = !db_exists;
      }
      else {
        pqxx::connection db{"dbname=" + db_name};
        pqxx::nontransaction nt_work{db};

        log("Cleaning tool_runs");
        nt_work.exec("DELETE FROM tool_runs");

        pqxx::result tables = nt_work.exec(
            "SELECT relname AS populated_table"
            " FROM pg_catalog.pg_stat_user_tables"
            " WHERE n_live_tup > 0"
            );
        for (auto const& table_row : tables) {
          string populated_table;
          table_row.at("populated_table").to(populated_table);
          log("Cleaning " + populated_table);
          nt_work.exec("DELETE FROM " + populated_table);
        }
      }
    }

    if (!db_exists) {
      pqxx::nontransaction nt_work{postgres_db};
      log("Creating database '" + db_name + "'...");
      nt_work.exec("CREATE DATABASE " + db_name);
    }
    postgres_db.disconnect();


    // Initialize target DB
    
    pqxx::connection db("dbname=" + db_name);
    pqxx::work work(db);

    if (is_full) {
      import_dir(work, schema_dir);

      if (opts.count("extra-schema")) {
        vector<string> extras = opts.at("extra-schema").as<vector<string>>();
        for (string extra : extras) {
          import_file(work, extra);
        }
      }
    }

    string const prefix_fn = opts.at("mac-prefix-file").as<string>();
    if (exists(fs::path(prefix_fn))) {
      insert_mac_prefixes(work, prefix_fn);
    }
    else {
      cerr << "WARNING: " << prefix_fn
           << " not found, no mac prefixes have been loaded" << endl;
    }

    work.commit();
  }
  catch (std::exception& e) {
    cerr << e.what() << endl;
    return -1;
  }
  return 0;
}

bool check_exists(string db_name)
{
  try {
    pqxx::connection exists_db("dbname=" + db_name);
    exists_db.disconnect();
    return true;
  }
  catch (std::exception& e) {
    return false;
  }
}

void log(string message) 
{
  if (verbose) {
    cout << message << endl;
  }
}

bool import_file(pqxx::work& work, string filename)
{
  if (fs::exists(fs::path(filename))) {
    log("Processing file: " + filename);
    ifstream sqlfile(filename);
    std::stringstream statement;
    statement << sqlfile.rdbuf();
    work.exec(statement.str());
    return true;
  }
  else {
    cerr << "No file: " << filename << " found." << endl;
    return false;
  }
}

bool import_dir(pqxx::work& work, string dir)
{
  vector<string> sortfd;
  fs::directory_iterator itr(dir), end_itr;

  for (; itr != end_itr; ++itr) {
    if (fs::is_regular_file(*itr) &&
        ".sql" == itr->path().extension()) {
      sortfd.push_back(itr->path().string());
    }
  }

  log("Sorting schema imports");
  std::sort(sortfd.begin(), sortfd.end());

  for (auto& entry : sortfd) {
    import_file(work, entry);
  }

  return true;
}

void insert_mac_prefixes(pqxx::work& work, string filename)
{
  map<string, string> macs;
  ifstream prefix_fd(filename);
  string line;

  log("Parsing MAC prefixes");

  regex mac_reg("^[0-9A-F]{6} .*");
  regex trim("^[ ]+|[ ]+$");
  while(getline(prefix_fd, line)) {
    if (regex_match(line, mac_reg)) { // Only get mac addr lines
      istringstream splitter(line);
      string mac;
      string vendor;

      splitter >> mac;
      getline(splitter, vendor);
      vendor = regex_replace(vendor, trim, ""); // Trim line whitespace

      if (macs.count(mac)) {
        macs[mac] = macs[mac] + " | " + vendor;
      }
      else {
        macs[mac] = vendor;
      }
    }
  }

  log("Inserting MAC prefixes");
  pqxx::stream_to st(work, "vendor_mac_prefixes");
  for(auto const& entry : macs) {
    st << std::make_tuple(entry.first + ":000000", entry.second);
  }
  st.complete();
}

