// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef EXPORT_SCAN_SSH_ALGORITHMS_HPP
#define EXPORT_SCAN_SSH_ALGORITHMS_HPP

#include "ExportScan.hpp"

namespace netmeld::playbook::export_scans {
// ============================================================================
// Primary object
// ============================================================================
class SshAlgorithms : public ExportScan {
  // ========================================================================
  // Variables
  // ========================================================================
  private: // Variables should generally be private
    // colors
    const std::string good  {"darkgreen"};
    const std::string bad   {"red"};
    const std::string unk   {"black"};

    // algorithms: identified good/bad
    // NOTE: separately called out for easier tracking
    const std::map<std::string, std::string> algoComp
    {
    };
    const std::map<std::string, std::string> algoEnc
    {
      {"aes128-ctr", good},
      {"aes256-ctr", good},
      {"aes128-gcm@openssh.com", good},
      {"aes256-gcm@openssh.com", good},

      {"none", bad},
      {"3des-cbc", bad},
      {"blowfish-cbc", bad},
      {"twofish-cbc", bad},
      {"twofish128-cbc", bad},
      {"twofish256-cbc", bad},
      {"cast128-cbc", bad},
      {"arcfour", bad},
      {"arcfour128", bad},
      {"arcfour256", bad},
      {"aes128-cbc", bad},
      {"aes192-cbc", bad},
      {"aes256-cbc", bad},
      {"rijndael128-cbc", bad},
      {"rijndael192-cbc", bad},
      {"rijndael256-cbc", bad},
      {"rijndael-cbc@lysator.liu.se", bad}
    };
    const std::map<std::string, std::string> algoKex
    {
      {"curve25519-sha256", good},
      {"curve25519-sha256@libssh.org", good},
      {"diffie-hellman-group14-sha256", good},
      {"diffie-hellman-group16-sha512", good},
      {"diffie-hellman-group18-sha512", good},
      {"diffie-hellman-group-exchange-sha256", good},

      {"diffie-hellman-group1-sha1", bad},
      {"diffie-hellman-group-exchange-sha1", bad}
    };
    const std::map<std::string, std::string> algoMac
    {
      {"hmac-sha2-256-etm@openssh.com", good},
      {"hmac-sha2-512-etm@openssh.com", good},
      {"umac-128-etm@openssh.com", good},

      {"none", bad},
      {"hmac-sha1-96", bad},
      {"hmac-sha2-256-96", bad},
      {"hmac-sha2-512-96", bad},
      {"hmac-md5", bad},
      {"hmac-md5-96", bad},
      {"hmac-ripemd160", bad},
      {"hmac-ripemd160@openssh.com", bad},
      {"hmac-sha1-96-etm@openssh.com", bad},
      {"hmac-md5-etm@openssh.com", bad},
      {"hmac-md5-96-etm@openssh.com", bad},
      {"hmac-ripemd160-etm@openssh.com", bad}
    };
    const std::map<std::string, std::string> algoKey
    {
      {"ssh-ed25519-cert-v01@openssh.com", good},
      {"ssh-rsa-cert-v01@openssh.com", good},
      {"ssh-ed25519", good},
      {"ssh-rsa", good},

      {"ssh-dss", bad},
      {"ssh-rsa-cert-v00@openssh.com", bad},
      {"ssh-dss-cert-v00@openssh.com", bad},
      {"ssh-dss-cert-v01@openssh.com", bad}
    };

    // NOTE: container for easier searching
    const std::map<std::string, std::map<std::string, std::string>> algorithms
    {
      {"compression_algorithms", algoComp},
      {"encryption_algorithms", algoEnc},
      {"kex_algorithms", algoKex},
      {"mac_algorithms", algoMac},
      {"server_host_key_algorithms", algoKey},
    };

  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope

  // ========================================================================
  // Constructors
  // ========================================================================
  private: // Constructors which should be hidden from API users
  protected: // Constructors part of subclass API
  public: // Constructors part of public API
    SshAlgorithms() = delete;
    SshAlgorithms(const std::string&);

  // ========================================================================
  // Methods
  // ========================================================================
  private: // Methods which should be hidden from API users
    void exportTemplate(auto&);
    void exportFromDb(auto&, pqxx::result&);

  protected: // Methods part of subclass API
  public: // Methods part of public API
    void exportScan(std::unique_ptr<Writer>&);
};
}
#endif // EXPORT_SCAN_SSH_ALGORITHMS_HPP
