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
#include "example.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <mfast.h>
#include <mfast/coder/fast_decoder_v2.h>
#include <mfast/coder/fast_encoder_v2.h>
#include <vector>

#include <boost/exception/diagnostic_information.hpp>
#include <chrono>

#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif

const char usage[] =
    "  -f file     : FAST Message file, default file=" DATA_FILE "\n"
    "  -head n     : process only the first 'n' messages\n"
    "  -c count    : repeat the test 'count' times\n"
    "  -r          : Toggle 'reset encoder on every message' (default false).\n"
    "  -hfix n     : Skip n byte header before each message, (default n=4)\n\n";

int read_file(const char *filename, std::vector<char> &contents)
{
  std::FILE *fp = std::fopen(filename, "rb");
  if (fp)
  {
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return 0;
  }
  std::cerr << "File read error : " << filename << "\n";
  return -1;
}

class ExampleDecoder
{
private:
  mfast::arena_allocator malloc_alloc;
  mfast::fast_decoder_v2<0> m_decoder;

public:
  ExampleDecoder() : m_decoder(&malloc_alloc, example::description())
  {
  }
  ~ExampleDecoder() {}
  void initialize()
  {
  }
  void decode(char *data, std::size_t skip_bytes, std::size_t size, bool force_reset)
  {
    const char *first = data + skip_bytes;
    const char *last = data + size;
    bool first_message = true;
    while (first < last)
    {
      m_decoder.decode(first, last, force_reset || first_message);
      first_message = false;
      first += skip_bytes;
    }
  }
};

int main(int argc, const char **argv)
{
  std::vector<char> message_contents;
  std::size_t head_n = (std::numeric_limits<std::size_t>::max)();
  std::size_t repeat_count = 1;
  bool force_reset = false;
  std::size_t skip_header_bytes = 4;
  ;
  const char *filename = DATA_FILE;

  int i = 1;
  int parse_status = 0;
  while (i < argc && parse_status == 0)
  {
    const char *arg = argv[i++];

    if (std::strcmp(arg, "-f") == 0)
    {
      filename = argv[i++];
    }
    else if (std::strcmp(arg, "-head") == 0)
    {
      head_n = atoi(argv[i++]);
      if (head_n == 0)
      {
        std::cerr << "Invalid argument for '-head'\n";
        parse_status = -1;
      }
    }
    else if (std::strcmp(arg, "-c") == 0)
    {
      repeat_count = atoi(argv[i++]);
      if (repeat_count == 0)
      {
        std::cerr << "Invalid argument for '-c'\n";
        parse_status = -1;
      }
    }
    else if (std::strcmp(arg, "-r") == 0)
    {
      force_reset = true;
    }
    else if (std::strcmp(arg, "-hfix") == 0)
    {
      skip_header_bytes = atoi(argv[i++]);
    }
  }

  parse_status = read_file(filename, message_contents);

  if (parse_status != 0 || message_contents.size() == 0)
  {
    std::cout << '\n'
              << usage;
    return -1;
  }

  try
  {

    // mfast::fast_decoder_v2<0> decoder(example::description());

#ifdef WITH_ENCODE
    mfast::fast_encoder_v2 encoder(example::description());
    std::vector<char> buffer;
    buffer.resize(message_contents.size());

#endif

    mfast::message_type msg_value;

    // boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();

    ExampleDecoder ex_decoder;
    ex_decoder.initialize();
    typedef std::chrono::high_resolution_clock clock;
    clock::time_point start = clock::now();
    {

      for (std::size_t j = 0; j < repeat_count; ++j)
      {
        ex_decoder.decode(&message_contents[0], skip_header_bytes, message_contents.size(), force_reset);
      }
    }

    typedef std::chrono::milliseconds ms;
    std::cout << "time spent " << std::chrono::duration_cast<ms>(clock::now() - start).count() << " ms\n";
  }
  catch (boost::exception &e)
  {
    std::cerr << boost::diagnostic_information(e);
    return -1;
  }

  return 0;
}
