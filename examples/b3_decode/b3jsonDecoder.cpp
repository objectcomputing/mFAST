#include <endian.h>
#include <fstream>
#include <iostream>
#include <string>

#include <mfast.h>
#include <mfast/coder/fast_decoder.h>
#include <mfast/json/json.h>
#include <mfast/xml_parser/dynamic_templates_description.h>

#include "b3_template.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;

using mfast::ascii_string_cref;
using mfast::dynamic_templates_description;
using mfast::fast_decoder;
using mfast::message_cref;
using mfast::templates_description;
using mfast::json::encode;

fast_decoder decoder;
static const templates_description *descriptions[] = {b3_template::templates_description::instance()};
int parse_one_packet(const char *fast_message, int msg_len, std::ofstream &out_file)
{

  const char *start = fast_message;  // .c_str();
  const char *end = start + msg_len; //fast_message.length();

  message_cref msg = decoder.decode(start, end, true);
  out_file << "Template id: " << msg.id() << " Size: " << msg_len << endl;

  ostringstream json_message;
  bool result = encode(json_message, msg, 0);
  if (result)
    out_file << json_message.str() << endl;

  return 0;
}

int main(int, char *argv[])
{
  std::ifstream inp_file;
  std::ofstream out_file;
  char buffer[1000000];
  uint32_t pkt_size;
  uint16_t chunk_idx, num_chunks, chunk_size;
  uint32_t data_size = 0;
  int ts_count;
  uint64_t ts;
  char tech_header[10];
  uint16_t last_chunk = 0;

  inp_file.open(argv[1], std::ios::in | std::ios::binary);
  out_file.open(argv[2], std::ios::out);
  decoder.include(descriptions);
  while (inp_file.tellg() >= 0)
  {
    inp_file.read(reinterpret_cast<char *>(&ts_count), 1);
    //cout << ts_count << endl;
    for (int i = 0; i < ts_count; ++i)
    {
      inp_file.read(reinterpret_cast<char *>(&ts), 8);
      //cout << ts << endl;
    }

    inp_file.read(reinterpret_cast<char *>(&pkt_size), 4);
    //cout << pkt_size << endl;
    if (pkt_size == 1)
    {
      inp_file.read(reinterpret_cast<char *>(&pkt_size), 1);
      continue;
    }

    uint16_t cur_packet_parsed = 0;
    while ((cur_packet_parsed < pkt_size) && (inp_file.tellg() >= 0))
    {
      //cout << "Starting packet at: " << inp_file.tellg() << endl;
      inp_file.read(&tech_header[0], 10);
      auto msg_seq_num = *reinterpret_cast<uint32_t *>(&tech_header[0]);
      msg_seq_num = be32toh(msg_seq_num);
      num_chunks = *reinterpret_cast<uint16_t *>(&tech_header[4]);
      num_chunks = be16toh(num_chunks);
      chunk_idx = *reinterpret_cast<uint16_t *>(&tech_header[6]);
      chunk_idx = be16toh(chunk_idx);
      chunk_size = *reinterpret_cast<uint16_t *>(&tech_header[8]);
      chunk_size = be16toh(chunk_size);

      if (chunk_size != pkt_size - 10)
      {
        //cout << "Multiple chunks in same packet" << endl;
      }
      inp_file.read(&buffer[data_size], chunk_size);
      cur_packet_parsed += 10 + chunk_size;
      data_size += chunk_size;
      out_file << msg_seq_num << " " << num_chunks << " " << chunk_idx << " "
               << "MsgLen: " << chunk_size << " " << inp_file.tellg() << " " << data_size << " " << last_chunk << endl;
      if (chunk_idx != num_chunks)
      {
        //cout << num_chunks << " " << chunk_idx << endl;
        last_chunk = chunk_idx;
        continue;
      }
      if (chunk_idx > 1)
      {
        if (last_chunk + 1 != chunk_idx)
        {
          cout << "Dropping unexpected chunk at " << static_cast<uint32_t>(inp_file.tellg()) - data_size << endl;
          data_size = 0;
          last_chunk = 0;
          continue;
        }
      }
      parse_one_packet(buffer, data_size, out_file);
      data_size = 0;
      last_chunk = 0;
      //cout << "File pos: " << inp_file.tellg() << endl;
      if (inp_file.tellg() < 0)
        break;
    }
  }
  out_file.close();
  inp_file.close();
}
