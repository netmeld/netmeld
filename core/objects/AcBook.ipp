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


// ===========================================================================
// Constructors
// ===========================================================================
template<typename TData>
AcBook<TData>::AcBook()
{}


// ===========================================================================
// Methods
// ===========================================================================
template<typename TData>
const std::string&
AcBook<TData>::getName() const
{
  return name;
}

template<typename TData>
std::set<TData>
AcBook<TData>::getData() const
{
  return data;
}

template<typename TData>
void
AcBook<TData>::setId(const std::string& _id)
{
  id = _id;
}

template<typename TData>
void
AcBook<TData>::setName(const std::string& _name)
{
  name = _name;
}

template<typename TData>
void
AcBook<TData>::addData(const TData& _key)
{
  auto val = data.emplace(_key);

  if (!std::get<1>(val)) {
    LOG_DEBUG << "AcBook::addData: Insertion failed, duplicate key found: "
             << _key << std::endl;
  }
}

template<typename TData>
void
AcBook<TData>::addData(const std::set<TData>& _dataSet)
{
  data.insert(_dataSet.begin(), _dataSet.end());
}

template<typename TData>
void
AcBook<TData>::removeData(const TData& _key)
{
  if (!data.erase(_key)) {
    LOG_DEBUG << "AcBook::removeData: No such key to remove: "
             << _key << std::endl;
  }
}

template<typename TData>
bool
AcBook<TData>::isValid() const
{
  return !id.empty()
      && !name.empty();
}

template<typename TData>
void
AcBook<TData>::save(pqxx::transaction_base&, const Uuid&, const std::string&)
{
  if (!isValid()) {
    LOG_DEBUG << "AcBook object is not saving: " << toDebugString()
              << std::endl;
    return; // Always short circuit if invalid object
  }
}

template<typename TData>
// Utilized for full object data dump, for debug purposes
std::string
AcBook<TData>::toDebugString() const
{
  std::ostringstream oss;

  oss << "[ " // opening bracket
      << id << ", "
      << name << ", "
      ;

  oss << data
      ;

  oss << " ]"; // closing bracket

  return oss.str();
}

// ===========================================================================
// Friends
// ===========================================================================
