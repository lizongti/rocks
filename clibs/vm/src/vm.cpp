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
LUA_API int luaopen_vm(lua_State *l);
#ifdef __cplusplus
};
#endif

#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>

struct State {
  lua_State *l;
  std::atomic_bool lock;
};

static std::map<std::string, State *> storage;
static std::shared_mutex mutex;

int traceback(lua_State *L) {
  if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
    if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring") ||
        !lua_isstring(L, -1))
      return 1;       /* Return non-string error object. */
    lua_remove(L, 1); /* Replace object by result of __tostring metamethod. */
  }
  luaL_traceback(L, L, lua_tostring(L, 1), 1);
  return 1;
}

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

void across_vm(lua_State *src, lua_State *dst, int i) {
  switch (lua_type(src, i)) {
    case LUA_TNIL:
      lua_pushnil(dst);
      break;
    case LUA_TBOOLEAN:
      lua_pushboolean(dst, lua_toboolean(src, i));
      break;
    case LUA_TNUMBER:
      lua_pushnumber(dst, lua_tonumber(src, i));
      break;
    case LUA_TLIGHTUSERDATA:
      lua_pushlightuserdata(dst, lua_touserdata(src, i));
      break;
    case LUA_TSTRING:
      luaL_pushbstring(dst, luaL_getbstring(src, i));
      break;
    case LUA_TUSERDATA: {
      void *p = lua_touserdata(src, 1);
      void *pptr = lua_newuserdata(dst, sizeof(void *));
      memcpy(pptr, &p, sizeof(void *));
      break;
    }

    default:
      fprintf(stderr, "Error: arg not support type '%s' at %d\n",
              lua_typename(src, lua_type(src, i)), i);
      lua_pushnil(dst);
      break;
  }
}

int writer(lua_State *L, const void *p, size_t sz, void *B) {
  (void)L;
  luaL_addlstring((luaL_Buffer *)B, (const char *)p, sz);
  return 0;
}

const char *dump(lua_State *l, int idx, size_t *size) {
  if (lua_isstring(l, idx)) {
    return lua_tolstring(l, idx, size);
  } else {
    const char *buff = NULL;
    int top = lua_gettop(l);
    luaL_Buffer b;
    int test_lua_dump;
    luaL_checktype(l, idx, LUA_TFUNCTION);
    lua_pushvalue(l, idx);
    luaL_buffinit(l, &b);
    test_lua_dump = (lua_dump(l, writer, &b) == 0);
    if (test_lua_dump) {
      luaL_pushresult(&b);
      buff = lua_tolstring(l, -1, size);
    } else
      luaL_error(l, "Error: unable to dump given function");
    lua_settop(l, top);

    return buff;
  }
}

static int l_register(lua_State *l) {
  std::string recv;
  int n = lua_gettop(l);
  if (n >= 1) luaL_getbstring(l, 1, recv);

  std::unique_lock lock(mutex);

  auto it = storage.find(recv);
  if (it == storage.end()) {
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);
    storage[recv] = new State();
    storage[recv]->l = l;
    storage[recv]->lock.store(false);
  } else {
    fprintf(stderr, "repeated register\n");
  }

  return 0;
}

static int l_mono_register(lua_State *l) {
  std::string recv;
  int n = lua_gettop(l);
  if (n >= 1) luaL_getbstring(l, 1, recv);

  auto it = storage.find(recv);
  if (it == storage.end()) {
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);
    storage[recv] = new State();
    storage[recv]->l = l;
    storage[recv]->lock.store(false);
  } else {
    fprintf(stderr, "repeated register\n");
  }

  return 0;
};

static int l_unregister(lua_State *l) {
  std::string recv;
  int n = lua_gettop(l);
  if (n >= 1) luaL_getbstring(l, 1, recv);

  std::unique_lock lock(mutex);

  auto it = storage.find(recv);
  if (it != storage.end()) {
    lua_close(it->second->l);
    delete it->second;
    storage.erase(recv);
  }

  return 0;
};

static int l_mono_unregister(lua_State *l) {
  std::string recv;
  int n = lua_gettop(l);
  if (n >= 1) luaL_getbstring(l, 1, recv);

  auto it = storage.find(recv);
  if (it != storage.end()) {
    lua_close(it->second->l);
    delete it->second;
    storage.erase(recv);
  }

  return 0;
};

static int l_safe_call(lua_State *l) {
  // get(0)
  int l_n = lua_gettop(l);
  if (l_n < 3) {
    fprintf(stderr, "argc must reach 3\n");
    lua_pushboolean(l, true);
    return 1;
  }
  int argc = l_n - 3;

  // get(1)
  std::string recv;
  luaL_getbstring(l, 1, recv);

  State *s;
  {
    std::shared_lock shared_lock(mutex);
    auto it = storage.find(recv);
    if (it == storage.end()) {
      fprintf(stderr, "cannot find vm %s\n", recv.c_str());
      lua_pushboolean(l, true);
      return 1;
    }
    s = it->second;
  }

  auto &lock = s->lock;
  if (lock.exchange(true)) {
    lua_pushboolean(l, false);
    return 1;
  }

  lua_State *rl = s->l;
  int rl_n = lua_gettop(rl);
  int errfunc;

  lua_pushcfunction(rl, traceback);
  errfunc = lua_gettop(rl);

  // get(2)
  int rc = lua_tonumber(l, 2);

  // get(3)
  int t = lua_type(l, 3);
  if (t == LUA_TSTRING) {
    std::string method;
    luaL_getbstring(l, 3, method);
    lua_getglobal(rl, method.c_str());
  } else if (t == LUA_TFUNCTION) {
    size_t size;
    const char *buff = dump(l, 3, &size);
    std::string s(buff, size);

    if (luaL_loadbuffer(rl, s.c_str(), s.size(), recv.c_str()) != 0) {
      fprintf(stderr, "Uncaught Error : %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      lock.exchange(false);
      lua_pushboolean(l, true);
      return 1;
    }
  } else {
    fprintf(stderr, "Error type of arg 3\n");

    lock.exchange(false);
    lua_pushboolean(l, true);
    return 1;
  }

  // get(4~)
  if (l_n > 3) {
    for (auto i = 4; i <= l_n; ++i) {
      across_vm(l, rl, i);
    }
  }

  // call
  int ret = lua_pcall(rl, argc, rc, errfunc);
  switch (ret) {
    case 0:
      break;
    case LUA_ERRMEM:
      fprintf(stderr, "System Error: %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      lock.exchange(false);
      lua_pushboolean(l, true);
      return 1;
    case LUA_ERRRUN:
    case LUA_ERRSYNTAX:
    case LUA_ERRERR:
    default:
      fprintf(stderr, "Uncaught Error: %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      lock.exchange(false);
      lua_pushboolean(l, true);
      return 1;
  }

  lua_pushboolean(l, true);

  for (auto i = 1; i <= rc; ++i) {
    across_vm(rl, l, -1);
    lua_pop(rl, 1);
  }

  lua_pop(rl, 1);
  assert(rl_n == lua_gettop(rl));

  lock.exchange(false);
  return rc + 1;
}

static int l_call(lua_State *l) {
  // get(0)
  int l_n = lua_gettop(l);
  if (l_n < 3) {
    fprintf(stderr, "argc must reach 3\n");
    lua_pushboolean(l, true);
    return 1;
  }
  int argc = l_n - 3;

  // get(1)
  std::string recv;
  luaL_getbstring(l, 1, recv);

  State *s;
  {
    auto it = storage.find(recv);
    if (it == storage.end()) {
      fprintf(stderr, "cannot find vm %s\n", recv.c_str());
      lua_pushboolean(l, true);
      return 1;
    }
    s = it->second;
  }

  auto &lock = s->lock;
  if (lock.exchange(true)) {
    lua_pushboolean(l, false);
    return 1;
  }

  lua_State *rl = s->l;
  int rl_n = lua_gettop(rl);
  int errfunc;

  lua_pushcfunction(rl, traceback);
  errfunc = lua_gettop(rl);

  // get(2)
  int rc = lua_tonumber(l, 2);

  // get(3)
  int t = lua_type(l, 3);
  if (t == LUA_TSTRING) {
    std::string method;
    luaL_getbstring(l, 3, method);
    lua_getglobal(rl, method.c_str());
  } else if (t == LUA_TFUNCTION) {
    size_t size;
    const char *buff = dump(l, 3, &size);
    std::string s(buff, size);

    if (luaL_loadbuffer(rl, s.c_str(), s.size(), recv.c_str()) != 0) {
      fprintf(stderr, "Uncaught Error : %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      lock.exchange(false);
      lua_pushboolean(l, true);
      return 1;
    }
  } else {
    fprintf(stderr, "Error type of arg 3\n");

    lock.exchange(false);
    lua_pushboolean(l, true);
    return 1;
  }

  // get(4~)
  if (l_n > 3) {
    for (auto i = 4; i <= l_n; ++i) {
      across_vm(l, rl, i);
    }
  }

  // call
  int ret = lua_pcall(rl, argc, rc, errfunc);
  switch (ret) {
    case 0:
      break;
    case LUA_ERRMEM:
      fprintf(stderr, "System Error: %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      lock.exchange(false);
      lua_pushboolean(l, true);
      return 1;
    case LUA_ERRRUN:
    case LUA_ERRSYNTAX:
    case LUA_ERRERR:
    default:
      fprintf(stderr, "Uncaught Error: %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      lock.exchange(false);
      lua_pushboolean(l, true);
      return 1;
  }

  lua_pushboolean(l, true);

  for (auto i = 1; i <= rc; ++i) {
    across_vm(rl, l, -1);
    lua_pop(rl, 1);
  }

  lua_pop(rl, 1);
  assert(rl_n == lua_gettop(rl));

  lock.exchange(false);
  return rc + 1;
}

static int l_mono_call(lua_State *l) {
  // get(0)
  int l_n = lua_gettop(l);
  if (l_n < 3) {
    fprintf(stderr, "argc must reach 3\n");
    return 0;
  }
  int argc = l_n - 3;

  // get(1)
  std::string recv;
  luaL_getbstring(l, 1, recv);

  State *s;
  auto it = storage.find(recv);
  if (it == storage.end()) {
    fprintf(stderr, "cannot find vm %s\n", recv.c_str());
    return 0;
  }
  s = it->second;

  lua_State *rl = s->l;
  int rl_n = lua_gettop(rl);
  int errfunc;

  lua_pushcfunction(rl, traceback);
  errfunc = lua_gettop(rl);

  // get(2)
  int rc = lua_tonumber(l, 2);

  // get(3)
  int t = lua_type(l, 3);
  if (t == LUA_TSTRING) {
    std::string method;
    luaL_getbstring(l, 3, method);
    lua_getglobal(rl, method.c_str());
  } else if (t == LUA_TFUNCTION) {
    size_t size;
    const char *buff = dump(l, 3, &size);
    std::string s(buff, size);

    if (luaL_loadbuffer(rl, s.c_str(), s.size(), recv.c_str()) != 0) {
      fprintf(stderr, "Uncaught Error : %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);
      return 0;
    }
  } else {
    fprintf(stderr, "Error type of arg 3\n");
    return 0;
  }

  // get(4~)
  if (l_n > 3) {
    for (auto i = 4; i <= l_n; ++i) {
      across_vm(l, rl, i);
    }
  }

  // call
  int ret = lua_pcall(rl, argc, rc, errfunc);
  switch (ret) {
    case 0:
      break;
    case LUA_ERRMEM:
      fprintf(stderr, "System Error: %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);

      return 0;
    case LUA_ERRRUN:
    case LUA_ERRSYNTAX:
    case LUA_ERRERR:
    default:
      fprintf(stderr, "Uncaught Error: %s\n", lua_tostring(rl, -1));
      lua_pop(rl, 1);
      return 0;
  }

  for (auto i = 1; i <= rc; ++i) {
    across_vm(rl, l, -1);
    lua_pop(rl, 1);
  }

  lua_pop(rl, 1);
  assert(rl_n == lua_gettop(rl));

  return rc;
}

static const luaL_Reg vm_module[] = {{"register", l_register},
                                     {"unregister", l_unregister},
                                     {"call", l_call},
                                     {"safe_call", l_safe_call},
                                     {"mono_register", l_mono_register},
                                     {"mono_unregister", l_mono_unregister},
                                     {"mono_call", l_mono_call},
                                     {NULL, NULL}};

int luaopen_vm(lua_State *l) {
  luaL_register(l, "vm", vm_module);
  return 1;
}