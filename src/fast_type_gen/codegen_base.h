// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <string>
#include <boost/exception/all.hpp>
#include <mfast.h>
#include <mfast/xml_parser/dynamic_templates_description.h>
#include <fstream>
#include <set>
#include <vector>

class file_open_error : public virtual boost::exception,
                        public virtual std::exception {
public:
  file_open_error() {}
  file_open_error(const std::string &filename) {
    *this << boost::errinfo_file_name(filename) << boost::errinfo_errno(errno);
  }
};

class codegen_base : public mfast::field_instruction_visitor {
protected:
  std::string filebase_;
  std::string ext_;
  std::vector<std::string> outer_ns_;
  std::string cpp_ns_;
  std::ofstream out_;
  std::stringstream cref_scope_;
  bool use_standard_comment_;

public:
  codegen_base(const char* filebase, const char* fileext);
  void set_outer_ns(const char *outer_ns);
  void set_generate_comment(bool generateComment);

  static std::string cpp_name(const mfast::field_instruction* inst);
  static std::string cpp_name(boost::string_ref n);
  static const  mfast::field_instruction* get_element_instruction(const mfast::sequence_field_instruction* inst);
protected:
  void generate_comment();
  void traverse(mfast::dynamic_templates_description &desc);
  virtual void traverse(const mfast::group_field_instruction *inst,
                        const char *name_suffix = "");

  void reset_scope(std::stringstream &strm, const std::string &str);

  std::string cpp_type_of(const mfast::field_instruction *inst,
                          std::set<std::string> *dependency = nullptr) const;

  bool is_const_field(const mfast::field_instruction *inst) const;

  bool
  contains_only_templateref(const mfast::group_field_instruction *inst) const;

  bool dont_generate(const mfast::field_instruction *inst) const;
};
