#include <fuzzer/FuzzedDataProvider.h>

#include <sstream>
#include <string>
#include <tinyparser.hpp>

using namespace std;
using namespace tipa;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  FuzzedDataProvider fdp(data, size);

  std::string fuzz_s1 = fdp.ConsumeRandomLengthString();
  std::string fuzz_s2 = fdp.ConsumeRandomLengthString();
  std::string fuzz_s3 = fdp.ConsumeRandomLengthString();

  rule expr = rule(tk_int) >> rule('+') >> rule(tk_int) >> rule(tk_ident) >>
              rule("(") >> rule(tk_int) >> rule(")") >> rule(";") >>
              rule(fuzz_s2) >> rule(fuzz_s3);

  stringstream str(fuzz_s1);
  parser_context pc;
  pc.set_stream(str);

  parse_all(expr, pc);

  return 0;
}