#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/asio/basic_streambuf.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/crc.hpp>
#include <sstream>
#include <string>

namespace util {
static uint16_t hex_to_dec(char *hex_c) {
  char hex_value[5];
  hex_value[4] = '\0';
  memcpy(hex_value, hex_c, 4);

  uint16_t decimal_value;
  std::stringstream ss;
  ss << std::string(hex_value);
  ss >> std::hex >> decimal_value;
  return decimal_value;
};

static void dec_to_hex(uint16_t decimal, char *hex_c) {
  std::stringstream ss;
  std::string hex;
  ss << std::hex << decimal;
  ss >> hex;
  hex.copy(hex_c + 4 - hex.size(), hex.size());
  std::string("0000").copy(hex_c, 4 - hex.size());
}

static uint16_t calc_crc(char *data, size_t data_len) {
  boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false> crc_ccitt;
  crc_ccitt = std::for_each(data, data + data_len, crc_ccitt);
  return crc_ccitt();
};

static int base64_encode(const std::string &input, std::string &output) {
  using namespace boost::archive::iterators;
  std::stringstream result;
  try {
    copy(base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>(
             input.begin()),
         base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>(
             input.end()),
         std::ostream_iterator<char>(result));
  } catch (...) {
    return -1;
  }

  size_t num = (3 - input.length() % 3) % 3;
  for (size_t i = 0; i < num; i++) {
    result.put('=');
  }
  output = result.str();
  return 0;
};

static int base64_decode(const std::string &input, std::string &output) {
  using namespace boost::archive::iterators;
  std::stringstream result;
  try {
    copy(transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>(
             input.begin()),
         transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>(
             input.end()),
         std::ostream_iterator<char>(result));
  } catch (...) {
    return -1;
  }
  output = result.str();
  return 0;
};
};  // namespace util