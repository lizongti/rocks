#include <boost/asio/basic_streambuf.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <string>

namespace core {
#pragma pack(1)
struct packet {
  char sz[4];
  char crc[4];
  char seq[4];
  char id[4];
  char msg[];
};
#pragma pack()

struct pipe {
  boost::asio::basic_streambuf<> buf;
  uint16_t seq_recv;
  uint16_t seq_send;
};

struct message {
  int32_t id;
  std::string data;
};

uint64_t get();

void put(uint64_t index);

size_t size(uint64_t index);

int32_t recv(uint64_t index, const std::string& flow, message& msg,
             bool validate);

int32_t send(uint64_t index, const message& msg, std::string& flow,
             bool validate);
};  // namespace core