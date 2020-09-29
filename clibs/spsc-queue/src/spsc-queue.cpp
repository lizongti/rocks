#ifdef __cplusplus
extern "C"
{
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
extern "C"
{
#endif
  LUA_API int luaopen_spsc_queue(lua_State *L);
#ifdef __cplusplus
};
#endif

#include <boost/lockfree/spsc_queue.hpp>
#include <map>
#include <string>

typedef std::map<
    std::string,
    std::map<std::string, boost::lockfree::spsc_queue<
                              std::string, boost::lockfree::capacity<65536>>>>
    Queue;

static Queue queue;

std::string *luaL_getbstring(lua_State *l, size_t index)
{
  std::string *s = new std::string();
  size_t size;
  const char *data = luaL_checklstring(l, index, &size);
  s->assign(data, size);
  return s;
};

void luaL_getbstring(lua_State *l, size_t index, std::string &s)
{
  size_t size;
  const char *data = luaL_checklstring(l, index, &size);
  s.assign(data, size);
};

void luaL_pushbstring(lua_State *l, const std::string *s)
{
  luaL_Buffer b;
  luaL_buffinit(l, &b);
  luaL_addlstring(&b, s->c_str(), s->size());
  luaL_pushresult(&b);
  delete s;
};

void luaL_pushbstring(lua_State *l, const std::string &s)
{
  luaL_Buffer b;
  luaL_buffinit(l, &b);
  luaL_addlstring(&b, s.c_str(), s.size());
  luaL_pushresult(&b);
};

static int l_flush(lua_State *l)
{
  std::string recv, send;
  int n = lua_gettop(l);
  if (n >= 1)
    luaL_getbstring(l, 1, recv);
  if (n >= 2)
    luaL_getbstring(l, 2, send);

  if (recv.empty())
  {
    queue.erase(queue.begin(), queue.end());
  }
  else if (send.empty())
  {
    auto it = queue.find(recv);
    if (it != queue.end())
    {
      auto &receiver = it->second;
      receiver.erase(it->second.begin(), it->second.end());
    }
  }
  else
  {
    auto it = queue.find(recv);
    if (it != queue.end())
    {
      auto &receiver = it->second;
      auto it = receiver.find(send);
      if (it != receiver.end())
      {
        auto &sender = it->second;
        sender.reset();
      }
    }
  }

  return 0;
};

static int l_push(lua_State *l)
{
  std::string recv, send, content;
  int n = lua_gettop(l);
  if (n >= 1)
    luaL_getbstring(l, 1, recv);
  if (n >= 2)
    luaL_getbstring(l, 2, send);
  if (n >= 3)
    luaL_getbstring(l, 3, content);

  auto &q = queue[recv][send];
  bool is_empty = !q.read_available();
  bool ret = q.push(content);

  lua_pushboolean(l, ret);
  lua_pushboolean(l, is_empty);

  return 2;
};

static int l_pop(lua_State *l)
{
  std::string recv, send, content;
  int n = lua_gettop(l);
  if (n >= 1)
    luaL_getbstring(l, 1, recv);
  if (n >= 2)
    luaL_getbstring(l, 2, send);

  auto &q = queue[recv][send];

  bool is_full = !q.write_available();
  bool ret = q.pop(content);

  lua_pushboolean(l, ret);
  lua_pushboolean(l, is_full);
  if (ret)
  {
    luaL_pushbstring(l, content);
    return 3;
  }
  else
  {
    return 2;
  }
};

static const luaL_Reg spsc_queue_module[] = {
    {"flush", l_flush}, {"push", l_push}, {"pop", l_pop}, {NULL, NULL}};

int luaopen_spsc_queue(lua_State *L)
{
  luaL_register(L, "spsc_queue", spsc_queue_module);
  return 1;
}