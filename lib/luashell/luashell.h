using namespace std;
#include <vector>
#include <string>

#include <LovyanGFX.hpp>
#include <luaexec.h>

#ifndef LUA_SHELL_H
#define LUA_SHELL_H

class LuaShell{
  public:
  int x = 0;
  int y = 0;
  lua_State* L;
  const int16_t TFT_WHITE = 0xffff;
  const int16_t TFT_BLACK = 0x0000;

  LuaEngine lua;
  vector<wchar_t> rawInputs;
  vector<wchar_t> ::iterator rawInputsItr;
  bool isTerminate = false;

  virtual void init(LGFX *lgfx);
  virtual bool onkeydown(char key, char c, bool ctrl);
};
#endif


