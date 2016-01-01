// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
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

#include "hpp_gen.h"
#include "inl_gen.h"
#include "cpp_gen.h"
#include "mfast/coder/common/dictionary_builder.h"
#include "mfast/coder/common/template_repo.h"

// #include <boost/filesystem.hpp>
#include <stdlib.h>
#include <fstream>
#include <iterator>

// using namespace boost::filesystem;

int main(int argc, const char** argv)
{
  mfast::template_registry registry;

  try {
    int i = 1;
    const char* export_symbol = nullptr;

    if (std::strcmp(argv[1], "-E") == 0) {
      export_symbol = argv[2];
      i = 3;
    }


    std::vector<mfast::dynamic_templates_description> descriptions;

    std::vector<std::string> filebases;

    mfast::simple_template_repo_t repo;


    for (int j = 0; i < argc; ++i, ++j) {

      std::ifstream ifs(argv[i]);

      if (!ifs) {
        std::cerr << argv[i] << " load failed\n";
        return -1;
      }

      std::string xml((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
      
#ifdef _WINDOWS
      char filebase_buf[_MAX_FNAME];
      _splitpath(argv[i], NULL, NULL, filebase_buf, NULL);
      boost::string_ref filebase = static_cast<const char*>(filebase_buf);
#else
      boost::string_ref filebase(argv[i]);
      filebase = filebase.substr(filebase.find_last_of('/') + 1);
      filebase = filebase.substr(0, filebase.find_last_of('.'));
#endif
      filebases.push_back(codegen_base::cpp_name(filebase));

      descriptions.emplace_back(xml.c_str(),
                                filebases[j].c_str(),
                                &registry);
    }

    repo.build(descriptions.begin(), descriptions.end());


    for (std::size_t j = 0; j < filebases.size(); ++j)
    {
      mfast::dynamic_templates_description& desc = descriptions[j];
      const std::string& filebase = filebases[j];

      hpp_gen header_gen(filebase.c_str());
      if (export_symbol)
        header_gen.set_export_symbol(export_symbol);
      header_gen.generate(desc);

      inl_gen inline_gen(filebase.c_str());
      inline_gen.generate(desc);

      cpp_gen source_gen(filebase.c_str());
      source_gen.generate(desc);
    }
  }
  catch( boost::exception & e ) {
    std::cerr << diagnostic_information(e);
    return -1;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return -1;
  }

  return 0;
}
