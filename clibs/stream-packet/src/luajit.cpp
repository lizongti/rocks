#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#ifdef __cplusplus
}
#endif

#if defined(LUA_BUILD_AS_DLL)
#if defined(LUA_CORE) || defined(LUA_LIB)
#define LUA_API __declspec(dllexfort)
#else
#define LUA_API __declspec(dllimfort)
#endif
#else
#define LUA_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
LUA_API int32_t luaopen_stream_packet(lua_State *L);
#ifdef __cplusplus
};
#endif

static int32_t get(lua_State *l) {
  lua_pushnumber(l, core::get());
  return 1;
};

static int32_t put(lua_State *l) {
  size_t n = lua_gettop(l);
  if (n < 1) {
    return 0;
  }
  core::put(lua_tonumber(l, 1));
  return 0;
};

static int32_t size(lua_State *l) {
  size_t n = lua_gettop(l);
  if (n < 1) {
    return 0;
  }

  lua_pushnumber(l, core::size(lua_tonumber(l, 1)));
  return 0;
};

static int32_t recv(lua_State *l) {
  size_t n = lua_gettop(l);
  if (n < 2) {
    return 0;
  }

  uint64_t index = lua_tonumber(l, 1);
  std::string flow;
  size_t data_len;
  const char *data = luaL_checklstring(l, 2, &data_len);
  flow.assign(data, data_len);

  bool validate = (n > 2) ? lua_toboolean(l, 3) : false;

  core::message msg;
  int32_t state = core::recv(index, flow, msg, validate);

  if (state <= 0) {
    lua_pushnumber(l, state);
    return 1;
  } else {
    lua_pushnumber(l, state);
    lua_pushnumber(l, msg.id);
    luaL_Buffer b;
    luaL_buffinit(l, &b);
    luaL_addlstring(&b, msg.data.c_str(), msg.data.size());
    luaL_pushresult(&b);
    return 3;
  }
};

static int32_t send(lua_State *l) {
  size_t n = lua_gettop(l);
  if (n < 3) {
    return 0;
  }

  uint64_t index = lua_tonumber(l, 1);

  int32_t id = lua_tonumber(l, 2);
  size_t data_len;
  const char *data = luaL_checklstring(l, 3, &data_len);
  core::message msg({id, std::string(data, data_len)});

  bool validate = (n > 3) ? lua_toboolean(l, 4) : false;

  std::string flow;
  int32_t state = core::send(index, msg, flow, validate);

  if (state <= 0) {
    lua_pushnumber(l, state);
    return 1;
  } else {
    lua_pushnumber(l, state);
    luaL_Buffer b;
    luaL_buffinit(l, &b);
    luaL_addlstring(&b, flow.c_str(), flow.size());
    luaL_pushresult(&b);

    return 2;
  }
}

static const luaL_Reg stream_packet_module[] = {{"get", get},   {"put", put},
                                                {"size", size}, {"recv", recv},
                                                {"send", send}, {NULL, NULL}};

int32_t luaopen_stream_packet(lua_State *L) {
  luaL_register(L, "stream_packet", stream_packet_module);
  return 1;
}