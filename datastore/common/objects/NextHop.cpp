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

#include <netmeld/datastore/objects/NextHop.hpp>


namespace netmeld::datastore::objects {

  NextHop::NextHop() : NextHop(0)
  {}

  NextHop::NextHop(unsigned int hopNumber) : hopNumber(hopNumber)
  {}

  void
  NextHop::setHopNumber(unsigned int hopNumber) {
    this->hopNumber = hopNumber;
  }

  void
  NextHop::setOrigin(const std::string& orig) {
    this->orig.setAddress(orig);
  }

  void
  NextHop::setDestination(const std::string& dest) {
    this->dest.setAddress(dest);
  }

  void
  NextHop::setOrigin(const IpAddress& orig) {
    this->orig = orig;
  }

  void
  NextHop::setDestination(const IpAddress& dest) {
    this->dest = dest;
  }

  unsigned int
  NextHop::getHopNumber() const
  {
    return this->hopNumber;
  }

  const IpAddress&
  NextHop::getOrigin() const
  {
    return this->orig;
  }

  const IpAddress&
  NextHop::getDestination() const
  {
    return this->dest;
  }
  
  bool
  NextHop::isValid() const
  {
    return hopNumber > 0 && orig.isValid() && dest.isValid();
  }

  void
  NextHop::save(pqxx::transaction_base&,
                                const nmco::Uuid&, const std::string&)
  {
    LOG_WARN << "NextHop::save called, nothing done"
             << std::endl;
  }
  
}
