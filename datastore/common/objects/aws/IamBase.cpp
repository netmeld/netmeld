// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/IamBase.hpp>


namespace netmeld::datastore::objects::aws {

  IamBase::IamBase()
  {}

  IamBase::IamBase(const std::string& _id,
                            const std::string& _arn,
                            const std::string& _name,
                            const std::string& _createDate,
                            const std::string& _path,
                            const json& _tags) :
    id(_id),
    arn(_arn),
    name(_name),
    createDate(_createDate),
    path(_path),
    tags(_tags)
  {}

  void
  IamBase::setId(const std::string& _id)
  {
      id = _id;
  }

  void
  IamBase::setArn(const std::string& _arn)
  {
      arn = _arn;
  }

  void
  IamBase::setName(const std::string& _name)
  {
      name = _name;
  }

  void
  IamBase::setCreateDate(const std::string& _createDate)
  {
      createDate = _createDate;
  }

  void
  IamBase::setPath(const std::string& _path)
  {
      path = _path;
  }

  void
  IamBase::setTags(const json& _tags)
  {
      tags = _tags;
  }

  std::string
  IamBase::getId() const
  {
      return id;
  }

  std::string
  IamBase::getArn() const
  {
      return arn;
  }

  std::string
  IamBase::getName() const
  {
      return name;
  }

  std::string
  IamBase::getCreateDate() const
  {
      return createDate;
  }

  std::string
  IamBase::getPath() const
  {
      return path;
  }

  json
  IamBase::getTags() const
  {
      return tags;
  }

  bool
  IamBase::isValid() const
  {
    return false;
  }

  std::string
  IamBase::toDebugString() const
  {
    std::ostringstream oss;

    oss << "[";
    oss << "id: " << id << ", "
        << "arn: " << arn << ", "
        << "name: " << name << ", "
        << "createDate: " << createDate << ","
        << "path: " << path << ","
        << "tags: " << tags.dump();
    oss << "]";

    return oss.str();
  }

  auto
  IamBase::operator<=>(const IamBase& rhs) const
  {
    return std::tie( id
                   , arn
                   , name
                   , createDate
                   , path
                   , tags
                   )
       <=> std::tie( rhs.id
                   , rhs.arn
                   , rhs.name
                   , rhs.createDate
                   , rhs.path
                   , rhs.tags
                   )
      ;
  }

  bool
  IamBase::operator==(const IamBase& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
