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

#ifndef AC_RULE_HPP
#define AC_RULE_HPP

#include <vector>

#include <netmeld/core/objects/AbstractDatastoreObject.hpp>


namespace netmeld::core::objects {

  class AcRule : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      size_t                    id {0};

      std::string               srcId;
      std::vector<std::string>  srcs;
      std::vector<std::string>  srcIfaces;

      std::string               dstId;
      std::vector<std::string>  dsts;
      std::vector<std::string>  dstIfaces;

      std::vector<std::string>  services;
      std::vector<std::string>  actions;

      std::string               description;

      bool enabled {true};

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      AcRule();

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
      void smartVectorAdd(const std::string&, std::vector<std::string>* const);

    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setRuleId(const size_t);
      void setRuleDescription(const std::string&);

      void setSrcId(const std::string&);
      void addSrc(const std::string&);
      void addSrcIface(const std::string&);

      void setDstId(const std::string&);
      void addDst(const std::string&);
      void addDstIface(const std::string&);

      void addAction(const std::string&);

      void addService(const std::string&);

      void enable();
      void disable();

      const std::string& getSrcId() const;
      const std::vector<std::string>& getSrcs() const;
      const std::vector<std::string>& getSrcIfaces() const;
      const std::string& getDstId() const;
      const std::vector<std::string>& getDsts() const;
      const std::vector<std::string>& getDstIfaces() const;
      const std::vector<std::string>& getServices() const;
      const std::vector<std::string>& getActions() const;

      // Inherited from AbstractDatastoreObject at this scope
        // virtual void saveAsMetadata(pqxx::transaction_base&, Uuid);
        // friend std::ostream& operator<<(std::ostream&, const AbstractDatastoreObject&);

      // Always overriden from AbstractDatastoreObject
      bool isValid() const override;
      void save(pqxx::transaction_base&, const Uuid&, const std::string&) override;
      std::string toDebugString() const override;

      friend bool operator<(const AcRule&, const AcRule&);
      friend bool operator==(const AcRule&, const AcRule&);
  };
}
#endif // AC_RULE_HPP
