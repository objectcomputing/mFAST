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
#include <mfast/xml_parser/dynamic_templates_description.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <limits>
#include <vector>

#include <boost/exception/diagnostic_information.hpp>

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
const char usage[] =
  "  -t file     : Template file (required)\n"
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
  std::vector<char> template_contents;
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

    if (std::strcmp(arg, "-t") == 0) {
      parse_status = read_file(argv[i++], template_contents);
    }
    else if (std::strcmp(arg, "-f") == 0) {
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

  if (parse_status != 0 || template_contents.size() == 0 || message_contents.size() == 0) {
    std::cout << '\n' << usage;
    return -1;
  }

  try {
    mfast::dynamic_templates_description description(&template_contents[0]);

    mfast::arena_allocator arena_alloc;
    mfast::malloc_allocator malloc_allc;
    mfast::allocator* alloc = &malloc_allc;
    if (use_arena)
      alloc = &arena_alloc;
    mfast::fast_decoder coder(alloc);

    const mfast::templates_description* descriptions[] = { &description };
    coder.include(descriptions);

#ifdef WITH_ENCODE
    mfast::fast_encoder encoder(alloc);
    encoder.include(descriptions);
    std::vector<char> buffer;
    buffer.reserve(message_contents.size());
#endif

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    {

      for (std::size_t j = 0; j < repeat_count; ++j) {

        const char *first = &message_contents[0] + skip_header_bytes;
        const char *last = &message_contents[0] + message_contents.size();
        bool first_message = true;
        while (first < last ) {
#ifdef WITH_ENCODE
          mfast::message_cref  msg =
#endif
            coder.decode(first, last, force_reset || first_message );

#ifdef WITH_ENCODE
          encoder.encode(msg, buffer, force_reset || first_message);
#endif
          first_message = false;
          first += skip_header_bytes;
        }
      }
    }
    boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
    std::cout << "time spent " <<  static_cast<unsigned long>((stop - start).total_milliseconds()) << " msec\n";
  }
  catch (boost::exception& e) {
    std::cerr << boost::diagnostic_information(e);
    return -1;
  }

  return 0;
}
