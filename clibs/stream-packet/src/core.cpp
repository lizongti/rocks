#include "core.h"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "util.hpp"

/*
To do :
1. add binary compatability
2. set base64 shuffled
*/

using namespace util;
using namespace core;

namespace core {
enum STATE {
  RECV_ONE_MESSAGE = 1,
  RECV_NO_MESSAGE = 0,
  RECV_SEQ_ERROR = -1,
  RECV_CRC_ERROR = -2,
  RECV_BASE64_ERROR = -3,

  SEND_ONE_MESSAGE = 1,
  SEND_NO_MESSAGE = 0,
  SEND_BASE64_ERROR = -1,
  SEND_SIZE_ERROR = -2,
};

static size_t packet_max_bytes = 64 * 1024;
static uint64_t index = 0;
static std::map<uint64_t, pipe *> pipes;

uint64_t get() {
  while (true) {
    auto iter = pipes.find(index);
    if (iter != pipes.end()) {
      ++index;
    } else {
      pipes[index] = new pipe();
      return index;
    }
  }
}

void put(uint64_t index) {
  auto iter = pipes.find(index);
  if (iter != pipes.end()) {
    delete iter->second;
    pipes.erase(iter);
  }
}

size_t size(uint64_t index) {
  auto iter = pipes.find(index);
  if (iter != pipes.end()) {
    return iter->second->buf.size();
  } else {
    pipes[index] = new pipe();
    return pipes[index]->buf.size();
  }
}

int32_t recv(uint64_t index, const std::string &flow, message &msg,
             bool validate) {
  pipe *pi;
  auto iter = pipes.find(index);
  if (iter != pipes.end()) {
    pi = iter->second;
  } else {
    pipes[index] = new pipe();
    pi = pipes[index];
  }

  auto &buf = pi->buf;
  buf.sputn(flow.c_str(), flow.size());

  if (buf.size() < 4) {
    return RECV_NO_MESSAGE;
  }

  std::string sz_str(boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_begin(buf.data()) + 4);
  char sz_buf[4];
  sz_str.copy(sz_buf, sz_str.size());
  size_t sz = hex_to_dec(sz_buf);

  if (buf.size() < sz) {
    return RECV_NO_MESSAGE;
  }

  char p_buf[sz + 1];
  packet *p = reinterpret_cast<packet *>(p_buf);
  p->sz[sz] = '\0';
  buf.sgetn((char *)p->sz, sz);

  if (validate) {
    if (hex_to_dec(p->seq) != pi->seq_recv + 1) {
      return RECV_SEQ_ERROR;
    }
    if (hex_to_dec(p->crc) != calc_crc(p->seq, sz - 8)) {
      return RECV_CRC_ERROR;
    }
  }

  std::string msg_b64 = p->msg;
  if (base64_decode(msg_b64, msg.data) < 0) {
    return RECV_BASE64_ERROR;
  }
  msg.id = hex_to_dec(p->id);
  ++pi->seq_recv;

  return RECV_ONE_MESSAGE;
};  // namespace core

int32_t send(uint64_t index, const message &msg, std::string &flow,
             bool validate) {
  pipe *pi;
  auto iter = pipes.find(index);
  if (iter != pipes.end()) {
    pi = iter->second;
  } else {
    pipes[index] = new pipe();
    pi = pipes[index];
  }

  if (msg.data.size() == 0) {
    return SEND_NO_MESSAGE;
  }
  std::string msg_b64;
  if (base64_encode(msg.data, msg_b64) < 0) {
    return SEND_BASE64_ERROR;
  }
  size_t sz = msg_b64.size() + 16;

  if (sz > packet_max_bytes) {
    return SEND_SIZE_ERROR;
  }

  char p_buf[sz + 1];
  packet *p = reinterpret_cast<packet *>(p_buf);
  p->sz[sz] = '\0';
  msg_b64.copy(p->msg, msg_b64.size());

  dec_to_hex(msg.id, p->id);
  dec_to_hex(sz, p->sz);

  ++pi->seq_send;

  if (validate) {
    dec_to_hex(pi->seq_send, p->seq);
    uint16_t crc = calc_crc(p->seq, sz - 8);
    dec_to_hex(crc, p->crc);
  } else {
    std::string("00000000").copy(p->crc, 8);
  }

  flow = p_buf;
  return SEND_ONE_MESSAGE;
};
};  // namespace core