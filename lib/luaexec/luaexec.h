#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#include <SD.h>
#include <SPIFFS.h>

#ifndef LUA_EXEC_H
#define LUA_EXEC_H

extern "C" {
#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
}

#define MAX_CHAR 256

struct LoadF{
  File f;
  char buf[MAX_CHAR];
};

inline uint16_t rgb24to16(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t tmp = ((r>>3) << 11) | ((g>>2) << 5) | (b>>3);
  return tmp; //(tmp >> 8) | (tmp << 8);
}

class LuaEngine{
  public:
  lua_State* L;
  const int16_t TFT_WHITE = 0xffff;
  const int16_t TFT_BLACK = 0x0000;
  const int16_t TFT_RED = 0xF800;
  int16_t fgColor = 0xffff;
  int16_t bgColor = 0x0000;
  LGFX *lgfx;
  bool isSD = false;
  String fileName = "/main.lua";
  String errorString;
  bool runError;

  bool isTerminate = false;
  void init(LGFX *plgfx);
  void eval(char* utf8LuaString);
  void keydown(char key, char c, bool ctrl);
  void exit();

  static int l_text(lua_State* L);
  static int l_fillRect(lua_State* L);
  static int l_color(lua_State* L);
  static int l_debug(lua_State* L);
  static int l_ksearch(lua_State* L);
  
  static int l_getFiles(lua_State* L);
  static int l_exists(lua_State* L);
  static int l_saveFile(lua_State* L);
  static int l_readFile(lua_State* L);
  static int l_clear(lua_State* L);
  static int l_getFreeHeap(lua_State* L);
  static int l_textWidth(lua_State* L);
  static int l_screenWidth(lua_State* L);
  static int l_screenHeight(lua_State* L);
  static int l_exit(lua_State* L);
  static int l_require(lua_State* L);
};

#endif
