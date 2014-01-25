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
#include <mfast.h>
#include <mfast/coder/fast_decoder.h>
#include <mfast/coder/fast_encoder.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <limits>
#include <vector>
#include "example.h"

#include <boost/exception/diagnostic_information.hpp>
// #include <boost/chrono/chrono.hpp>

// Although chrono is better for performance measurement in theory, I choose to sue
// boost data_time because QuickFAST uses it and I want to compare the result with
// QuickFAST PerformanceTest directly.

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

const char usage[] =
  "  -f file     : FAST Message file (required)\n"
  "  -head n     : process only the first 'n' messages\n"
  "  -c count    : repeat the test 'count' times\n"
  "  -r          : Toggle 'reset encoder on every message' (default false).\n"
  "  -hfix n     : Skip n byte header before each message\n"
  "  -arena      : Use arena_allocator\n\n";

int read_file(const char* filename, std::vector<char>& contents)
{
  std::FILE*fp = std::fopen(filename, "rb");
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

int main(int argc, const char** argv)
{
  std::vector<char> message_contents;
  std::size_t head_n = (std::numeric_limits<std::size_t>::max)();
  std::size_t repeat_count = 1;
  bool force_reset = false;
  std::size_t skip_header_bytes = 0;
  bool use_arena = false;

  int i = 1;
  int parse_status = 0;
  while (i < argc && parse_status == 0) {
    const char* arg = argv[i++];

    if (std::strcmp(arg, "-f") == 0) {
      parse_status = read_file(argv[i++], message_contents);
    }
    else if (std::strcmp(arg, "-head") == 0) {
      head_n = atoi(argv[i++]);
      if (head_n == 0) {
        std::cerr << "Invalid argument for '-head'\n";
        parse_status = -1;
      }
    }
    else if (std::strcmp(arg, "-c") == 0) {
      repeat_count = atoi(argv[i++]);
      if (repeat_count == 0) {
        std::cerr << "Invalid argument for '-c'\n";
        parse_status = -1;
      }
    }
    else if (std::strcmp(arg, "-r") == 0) {
      force_reset = true;
    }
    else if (std::strcmp(arg, "-hfix") == 0) {
      skip_header_bytes = atoi(argv[i++]);
    }
    else if (std::strcmp(arg, "-arena") == 0) {
      use_arena = true;
    }
  }

  if (parse_status != 0 || message_contents.size() == 0) {
    std::cout << '\n' << usage;
    return -1;
  }

  try {

    mfast::arena_allocator arena_alloc;
    mfast::malloc_allocator malloc_allc;
    mfast::allocator* alloc = &malloc_allc;
    if (use_arena)
      alloc = &arena_alloc;

    const mfast::templates_description* descriptions[] = { example::description() };

    mfast::fast_decoder decoder(alloc);
    decoder.include(descriptions);

#ifdef WITH_ENCODE
    mfast::fast_encoder encoder(alloc);
    encoder.include(descriptions);
    std::vector<char> buffer;
    buffer.reserve(message_contents.size());
#endif

    mfast::message_type msg_value;

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();

    // typedef boost::chrono::high_resolution_clock clock;
    // clock::time_point start=clock::now();
    {

      for (std::size_t j = 0; j < repeat_count; ++j) {

        const char *first = &message_contents[0] + skip_header_bytes;
        const char *last = &message_contents[0] + message_contents.size();
        bool first_message = true;
        while (first < last ) {
          mfast::message_cref msg = decoder.decode(first, last, force_reset || first_message );

#ifdef WITH_ENCODE
          encoder.encode(msg, buffer, force_reset || first_message);
#endif
#ifdef WITH_MESSAGE_COPY
          msg_value = mfast::message_type(msg, &malloc_allc);
#endif
          first_message = false;
          first += skip_header_bytes;
        }
      }
    }

    boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
    std::cout << "time spent " <<  static_cast<unsigned long>((stop - start).total_milliseconds()) << " msec\n";

    // typedef boost::chrono::milliseconds ms;
    // std::cout << "time spent " << boost::chrono::duration_cast<ms>(clock::now() - start).count() << " ms\n";
  }
  catch (boost::exception& e) {
    std::cerr << boost::diagnostic_information(e);
    return -1;
  }

  return 0;
}
