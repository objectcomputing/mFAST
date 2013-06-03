// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//
#include <iostream>
#include <string>

#include "FastXML2Header.h"
#include "FastXML2Inline.h"
#include "FastXML2Source.h"
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

using namespace tinyxml2;

int main(int argc, const char** argv)
{
  templates_registry_t registry;

  try {
    for (int i = 1; i < argc; ++i) {
      XMLDocument doc;
      if (doc.LoadFile( argv[i] ) != 0)
      {
        std::cerr << argv[i] << " load failed\n";
        return doc.ErrorID();
      }

      path f(path(argv[i]).stem());
      const char* filebase = f.c_str();

      FastXML2Header header_producer(filebase,registry);
      doc.Accept(&header_producer);

      FastXML2Inline inline_producer(filebase,registry);
      doc.Accept(&inline_producer);

      FastXML2Source source_producer(filebase,registry);
      doc.Accept(&source_producer);
    }
  }
  catch (std::exception& ex) {
    std::cerr << ex.what() << "\n";
    return -1;
  }

  return 0;
}
