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

#ifndef SERVICE_FACTORY_HPP
#define SERVICE_FACTORY_HPP

#include <netmeld/datastore/objects/Service.hpp>

namespace nmdo = netmeld::datastore::objects;


namespace netmeld::datastore::utils {

  class ServiceFactory {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
      ServiceFactory();
    protected:
    public:
      ServiceFactory(const ServiceFactory&) = delete;
      void operator=(const ServiceFactory&) = delete;

      static nmdo::Service makeDhcp();
      static nmdo::Service makeDns();
      static nmdo::Service makeNtp();
      static nmdo::Service makeRadius();
      static nmdo::Service makeSnmp();
      static nmdo::Service makeSyslog();


    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
  };
}
#endif // SERVICE_FACTORY_HPP
