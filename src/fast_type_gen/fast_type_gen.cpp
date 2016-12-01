// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include <iostream>
#include <string>

#include "hpp_gen.h"
#include "inl_gen.h"
#include "cpp_gen.h"
#include "mfast/coder/common/dictionary_builder.h"
#include "mfast/coder/common/template_repo.h"

#include <stdlib.h>
#include <fstream>
#include <iterator>

bool check_long_option(const std::string &command, int argc, const char **argv, int &i, const char *name, const char *&output, bool &bad) {
  const auto len = std::strlen(name);
  if (std::strncmp(argv[i], name, len) != 0) {
    return false;
  } else if (argv[i][len] == '=') {
    output = &argv[i][len + 1];
    return true;
  } else if (argv[i][len] == '\0') {
    if (++i < argc) {
      output = argv[i];
    } else {
      std::cerr << command << ": option requires and argument -- " << name << std::endl;
      bad = true;
    }
    return true;
  } else {
    return false;
  }
}

bool check_short_option(const std::string &command, int argc, const char **argv, int &i, const char *&arg, char flag, const char *&output, bool &bad)
{
  if (*arg == flag) {
    if (*++arg != '\0') {
      output = arg;
      arg += std::strlen(arg);
    } else if (++i < argc) {
      arg = output = argv[i];
      arg += std::strlen(arg);
    }
    else {
      std::cerr << command << ": option requires and argument -- " << flag << std::endl;
      bad = true;
    }
    return true;
  } else {
    return false;
  }
}

int main(int argc, const char **argv) {
  mfast::template_registry registry;

  try {
    int i = 1;

    std::string command_name = "fast_type_gen";

    const char *export_symbol = nullptr;
    const char *outer_namespace = nullptr;
    const char *header_extension = ".h";
    const char *inline_extension = ".inl";
    const char *source_extension = ".cpp";

    bool show_usage = false;
    bool bad_arguments = false;

    // handle all options: 
    // valid option formats:
    // -h or -help
    // -Xparam where X is a single character option
    // -X param where X is a single character option
    // -OPT=param where OPT is long option
    // -OPT param  where OPT is long option
    // --  Stops the scan for options.
    while (i < argc && argv[i][0] == '-' && std::strcmp(argv[i], "--") != 0 && !bad_arguments) {
      const char *flag = &argv[i][1];
      if (*flag == '-') {
        ++flag;
        if (std::strcmp(flag, "help") == 0) {
          show_usage = true;
        } else if (check_long_option(command_name, argc, argv, i, "--export-symbol", export_symbol, bad_arguments)) {
        } else if (check_long_option(command_name, argc, argv, i, "--namespace", outer_namespace, bad_arguments)) {
        } else if (check_long_option(command_name, argc, argv, i, "--header-extension", header_extension, bad_arguments)) {
        } else if (check_long_option(command_name, argc, argv, i, "--inline-extension", inline_extension, bad_arguments)) {
        } else if (check_long_option(command_name, argc, argv, i, "--source-extension", source_extension, bad_arguments)) {
        } else {
          std::cerr << command_name << ": illegal option -- " << flag << std::endl;
          bad_arguments = true;
        }
      } else {
        while ((i < argc) && *flag && !bad_arguments) {
          if (*flag == 'h') {
            show_usage = true;
            ++flag;
          } else if (check_short_option(command_name, argc, argv, i, flag, 'E', export_symbol, bad_arguments)) {
          } else if (check_short_option(command_name, argc, argv, i, flag, 'n', outer_namespace, bad_arguments)) {
          } else if (check_short_option(command_name, argc, argv, i, flag, 'H', header_extension, bad_arguments)) {
          } else if (check_short_option(command_name, argc, argv, i, flag, 'I', inline_extension, bad_arguments)) {
          } else if (check_short_option(command_name, argc, argv, i, flag, 'C', source_extension, bad_arguments)) {
          } else {
            std::cerr << command_name << ": illegal option -- " << *flag << std::endl;
            bad_arguments = true;
          }
        }
      }
      ++i;
    }

    // If a "--" stopped the scan, skip it.
    if(i < argc && std::strcmp(argv[i], "--") == 0){
      ++i;
    }

    if(i >= argc){
        std::cerr << "No template file(s) on command line." << std::endl;
        bad_arguments = true;
    }

    // Handle errors and help requests then exit
    if (show_usage || bad_arguments) {
      std::ostream &output = show_usage ? std::cout : std::cerr;
      output << "usage: " << command_name << " [-E symbol] [-H extension] [-I extension] [-C extension] template_file ...\n"
                "       generate C++ bindings for FAST types\n";
      if (show_usage) {
        output << "\n"
                  "Options and arguments:\n"
                  "  -h, --help                  show usage and exit\n"
                  "  -E, --export-symbol=SYM     qualifier for generated types\n"
                  "  -n, --namespace=NS          namespace for generated code\n"
                  "  -C, --source-extension=EXT  source filename extension (default .cpp)\n"
                  "  -H, --header-extension=EXT  header filename extension (default .h)\n"
                  "  -I, --inline-extension=EXT  inline function filename extension (default .inl)\n"
                  "  template_file ...           One or more XML FAST message template inputs\n";
      }
      return bad_arguments ? -1 : 0;
    }

    // remaining arguments must be template file names
    std::vector<std::string> filebases;

    std::vector<mfast::dynamic_templates_description> descriptions;
    for (; i < argc; ++i) {

      std::ifstream ifs(argv[i]);

      if (!ifs) {
        std::cerr << command_name << ": " << argv[i] << " load failed" << std::endl;
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

      descriptions.emplace_back(xml.c_str(), filebases.back().c_str(), &registry);
    }

    mfast::simple_template_repo_t repo;
    repo.build(descriptions.begin(), descriptions.end());

    for (std::size_t j = 0; j < filebases.size(); ++j) {
      mfast::dynamic_templates_description &desc = descriptions[j];
      const std::string &filebase = filebases[j];

      hpp_gen header_gen(filebase.c_str(), header_extension);
      if (outer_namespace)
        header_gen.set_outer_ns(outer_namespace);
      if (export_symbol)
        header_gen.set_export_symbol(export_symbol);
      header_gen.set_inl_fileext(inline_extension);
      header_gen.generate(desc);

      inl_gen inline_gen(filebase.c_str(), inline_extension);
      if (outer_namespace)
        inline_gen.set_outer_ns(outer_namespace);
      inline_gen.generate(desc);

      cpp_gen source_gen(filebase.c_str(), source_extension);
      if (outer_namespace)
        source_gen.set_outer_ns(outer_namespace);
      source_gen.set_hpp_fileext(header_extension);
      source_gen.generate(desc);
    }
  } catch (boost::exception &e) {
    std::cerr << diagnostic_information(e);
    return -1;
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return -1;
  }

  return 0;
}
