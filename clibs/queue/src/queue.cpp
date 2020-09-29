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
#define LUA_API __declspec(dllexport)
#else
#define LUA_API __declspec(dllimport)
#endif
#else
#define LUA_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
LUA_API int luaopen_queue(lua_State *L);
#ifdef __cplusplus
};
#endif

#include <boost/lockfree/queue.hpp>
#include <map>
#include <string>

#define MAX_SIZE 65536

static std::map<std::string, boost::lockfree::queue<std::string *> *> queue;

std::string *luaL_getbstring(lua_State *l, size_t index) {
  std::string *s = new std::string();
  size_t size;
  const char *data = luaL_checklstring(l, index, &size);
  s->assign(data, size);
  return s;
};

void luaL_getbstring(lua_State *l, size_t index, std::string &s) {
  size_t size;
  const char *data = luaL_checklstring(l, index, &size);
  s.assign(data, size);
};

void luaL_pushbstring(lua_State *l, const std::string *s) {
  luaL_Buffer b;
  luaL_buffinit(l, &b);
  luaL_addlstring(&b, s->c_str(), s->size());
  luaL_pushresult(&b);
  delete s;
};

void luaL_pushbstring(lua_State *l, const std::string &s) {
  luaL_Buffer b;
  luaL_buffinit(l, &b);
  luaL_addlstring(&b, s.c_str(), s.size());
  luaL_pushresult(&b);
};

static int l_push(lua_State *l) {
  std::string key;
  std::string *content;
  int n = lua_gettop(l);
  if (n >= 1) luaL_getbstring(l, 1, key);
  if (n >= 2) content = luaL_getbstring(l, 2);

  if (queue.find(key) == queue.end()) {
    queue.insert(
        make_pair(key, new boost::lockfree::queue<std::string *>(MAX_SIZE)));
  }

  bool ret = queue[key]->push(content);

  lua_pushboolean(l, ret);

  return 1;
};

static int l_pop(lua_State *l) {
  std::string key;
  std::string *content;
  int n = lua_gettop(l);
  if (n >= 1) luaL_getbstring(l, 1, key);

  if (queue.find(key) == queue.end()) {
    queue.insert(
        make_pair(key, new boost::lockfree::queue<std::string *>(MAX_SIZE)));
  }
  
  bool ret = queue[key]->pop(content);

  lua_pushboolean(l, ret);
  if (ret) {
    luaL_pushbstring(l, content);
    return 2;
  } else {
    return 1;
  }
};

static const luaL_Reg queue_module[] = {
    {"push", l_push}, {"pop", l_pop}, {NULL, NULL}};

int luaopen_queue(lua_State *L) {
  luaL_register(L, "queue", queue_module);
  return 1;
}