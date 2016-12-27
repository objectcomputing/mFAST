#include <iostream>
#include <string>

#include <mfast.h>
#include <mfast/coder/fast_decoder.h>
#include <mfast/json/json.h>
#include <mfast/xml_parser/dynamic_templates_description.h>

using std::string;
using std::ostringstream;
using std::cout;
using std::endl;

using mfast::templates_description;
using mfast::dynamic_templates_description;
using mfast::fast_decoder;
using mfast::message_cref;
using mfast::ascii_string_cref;
using mfast::json::encode;

// example from http://jettekfix.com/node/36
static const string fast_template =
    "\
<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\
<templates xmlns=\"http://www.fixprotocol.org/ns/fast/td/1.1\">\
    <template dictionary=\"1\" id=\"1\" name=\"HelloWorld\">\
         <string id=\"58\" name=\"Text\">\
             <default value=\"\"></default>\
         </string>\
    </template>\
</templates>\
";

// 58=HelloWorld<SOH>
static const string fast_message =
    "\xE0\x81\x48\x65\x6C\x6C\x6F\x57\x6F\x72\x6C\xE4";

int main() {
  dynamic_templates_description description(fast_template);

  const templates_description* descriptions[] = {&description};

  fast_decoder decoder;
  decoder.include(descriptions);

  const char* start = fast_message.c_str();
  const char* end = start + fast_message.length();

  cout << "Decoding message \"58=HelloWorld<SOH>\":" << endl;
  cout << endl;

  message_cref msg = decoder.decode(start, end);

  cout << "Template id: " << msg.id() << endl;
  cout << "Template name: " << msg.name() << endl;
  cout << endl;

  ascii_string_cref field = static_cast<ascii_string_cref>((msg)[0]);

  cout << "Field id: " << field.id() << endl;
  cout << "Field name: " << field.name() << endl;
  cout << "Field content: " << field.c_str() << endl;
  cout << endl;

  cout << "Encoding message to JSON:" << endl;

  ostringstream json_message;
  bool result = encode(json_message, msg, 0);
  if (result) cout << "Success: " << json_message.str() << endl;

  return 0;
}
