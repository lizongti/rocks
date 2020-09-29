
#include <time.h>
#include <iostream>
#include "core.h"
#include "util.hpp"
using namespace core;
using namespace util;

//////////////////////////////////////////////////////////////////////
//								TEST
////
//////////////////////////////////////////////////////////////////////
int test1()  // 可见字符串编解码
{
  std::string in = "{\"a\":1,\"b\":2}", out, out2;
  util::base64_encode(in, out);
  util::base64_decode(out, out2);
  std::cout << in << std::endl << out << std::endl << out2 << std::endl;
  return 0;
}
int test2()  // 进制转换
{
  uint16_t in = 10, out;
  char buf[4];
  util::dec_to_hex(in, buf);
  out = util::hex_to_dec(buf);
  std::cout << buf[0] << buf[1] << buf[2] << buf[3] << std::endl
            << out << std::endl;
  return 0;
};
int test3()  //粘包和切包
{
  uint64_t s = core::get();
  core::message pi;
  std::string flow;
  pi.id = 10;
  pi.data = "{\"a\":1,\"b\":2}";
  int32_t state = core::send(s, pi, flow, false);
  /////////////////////////////////////
  printf("%d %s\n", state, flow.c_str());
  pi = {0, ""};
  flow =
      "002400000000000aeyJhIjoxLCJiIjoyfQ==002400000000000aeyJhIjoxLCJiIjoyfQ=="
      "002400";
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow =
      "000000000aeyJhIjoxLCJiIjoyfQ==002400000000000aeyJhIjoxLCJiIjoyfQ=="
      "002400000000000aeyJhIjoxLCJiIjoyfQ==002400000000000aey";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "JhIjoxLCJiIjoyfQ==002400000000000ae";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "yJhIjoxLCJiIjoyfQ==";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());
  flow = "";
  pi = {0, ""};
  state = core::recv(s, flow, pi, false);
  printf("%d %s\n", pi.id, pi.data.c_str());

  return 0;
}

int test4()  // 性能
{
  // [with validation] encode:4.00224; decode:0.013741
  // [no validation] encode:2.9868; decode:0.013794
  uint64_t s = core::get();
  core::message pi;
  std::string flow;
  pi.id = 10;
  pi.data = "{\"a\":1,\"b\":2,\"d\":3,\"c\":3,\"f\":3,\"h\":3,\"g\":3,}";
  time_t start = clock();
  int32_t state;
  for (size_t i = 0; i < 1000 * 1000; ++i) {
    state = core::send(s, pi, flow, false);
    state = core::recv(s, flow, pi, false);
  }
  time_t stop = clock();
  std::cout << (stop - start) / 1000000.0 << std::endl;
  return 0;
}

int test5()  // 二进制流编解码 base64 回转的字符串结尾会多出0
{
  uint64_t s = core::get();
  core::message pi;
  std::string flow;
  pi.id = 10;
  int32_t state;
  pi.data = std::string("{\"a\":1,") + '\0' + '\0' + '\0' + '\0' + '\0' +
            "\"b\":2" + '\0' + "}";
  std::cout << pi.id << " len=[" << pi.data.size() << "],data=[" << pi.data
            << "]" << std::endl;
  state = core::send(s, pi, flow, true);
  /////////////////////////////////////
  std::cout << state << " len=[" << flow.size() << "],data=[" << flow << "]"
            << std::endl;
  state = core::recv(s, flow, pi, true);
  std::cout << pi.id << " len=[" << pi.data.size() << "],data=[" << pi.data
            << "]" << std::endl;
  char buf[64];
  pi.data.copy(buf, pi.data.size());
  for (int i = 0; i < pi.data.size(); ++i) {
    std::cout << i << " " << int(buf[i]) << " " << buf[i] << std::endl;
  }
  return 0;
}

int test6()  // streambuf 的大小
{
  boost::asio::basic_streambuf<> buf;
  std::cout << buf.size() << std::endl;
  const char *s = "abcd";
  std::string str = s;
  char b[4];
  buf.sputn(s, 4);
  std::string sz_str(boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_begin(buf.data()) + 3);
  std::cout << sz_str << std::endl;
  std::cout << buf.size() << std::endl;
  buf.sgetn(b, 3);
  std::cout << buf.size() << std::endl;
  return 0;
}

int main() { return test3(); };