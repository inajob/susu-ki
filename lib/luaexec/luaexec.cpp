#include <luaexec.h>
#include <search.h>
#include <Arduino.h>

extern "C" {
  const char *getF(lua_State *L, void *ud, size_t *size){
    struct LoadF *lf = (struct LoadF *)ud;
    (void)L; /* not used */
    char* ret = NULL;

    if(!lf->f.available()){
      *size = 0;
      return NULL;
    }

    lf->f.readStringUntil('\n').toCharArray(lf->buf, MAX_CHAR);
    ret = lf->buf;
    int len = strnlen(ret, MAX_CHAR);
    ret[len] = '\n'; // todo n, n+1 > MAX_CHAR ?
    ret[len + 1] = 0;
    Serial.println(ret);

    *size = len + 1;
    //Serial.print("");
    //Serial.println(ret);
    //Serial.println(*size);
    return ret;
  }
}

void LuaEngine::init(LGFX *plgfx){
  lgfx = plgfx;
  isTerminate = false;

  L = luaL_newstate();
  luaL_openlibs(L);

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_getFiles, 1);
  lua_setglobal(L, "getfiles");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_exists, 1);
  lua_setglobal(L, "exists");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_saveFile, 1);
  lua_setglobal(L, "savefile");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_readFile, 1);
  lua_setglobal(L, "readfile");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_text, 1);
  lua_setglobal(L, "text");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_clear, 1);
  lua_setglobal(L, "clear");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_fillRect, 1);
  lua_setglobal(L, "fillrect");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_getFreeHeap, 1);
  lua_setglobal(L, "getfreeheap");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_textWidth, 1);
  lua_setglobal(L, "textwidth");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_screenWidth, 1);
  lua_setglobal(L, "screenwidth");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_screenHeight, 1);
  lua_setglobal(L, "screenheight");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_color, 1);
  lua_setglobal(L, "color");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_debug, 1);
  lua_setglobal(L, "debug");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_exit, 1);
  lua_setglobal(L, "exit");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_run, 1);
  lua_setglobal(L, "run");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_require, 1);
  lua_setglobal(L, "require");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_ksearch, 1);
  lua_setglobal(L, "ksearch");

  File fp;
  if(isSD){
    fp = SD.open(fileName, FILE_READ);
    if(!fp){
      Serial.println("SD file open error");
      isTerminate = true;
      return;
    }
  }else{
    fp = SPIFFS.open(fileName, FILE_READ);
  }
  struct LoadF lf;
  lf.f = fp;
  char cFileName[32];
  bool runError = false;
  fileName.toCharArray(cFileName, 32);
  if(lua_load(L, getF, &lf, cFileName, NULL)){
    Serial.printf("error? %s\n", lua_tostring(L, -1));
    runError = true;
    errorString = lua_tostring(L, -1);
    isTerminate = true;
    return;
  }
  fp.close();

  if(runError == false){
    Serial.println("before pcall");
    if(lua_pcall(L, 0, 0,0)){
      Serial.printf("init error? %s\n", lua_tostring(L, -1));
      runError = true;
      errorString = lua_tostring(L, -1);
      isTerminate = true;
      return;
    }
    Serial.println("after pcall");
  }
}
void LuaEngine::eval(char* utf8LuaString){
  int result = luaL_loadstring(L, utf8LuaString);
  if(result != LUA_OK){
    Serial.println("lua error");
    Serial.printf("error? %s\n", lua_tostring(L, -1));
    char* err = (char*)lua_tostring(L, -1);
    lgfx->setCursor(0,0);
    lgfx->print((char*)lua_tostring(L, -1));

  }else{
    Serial.println("lua ok");

    if(lua_pcall(L,0,0,0) != LUA_OK) {
      Serial.println("lua error");
      Serial.println(lua_tostring(L, -1));
      //lua_close(L);
      //exit(EXIT_FAILURE);
    }
  }
}
void LuaEngine::keydown(char key, char c, bool ctrl){
  lua_getglobal(L, "keydown");
  lua_pushnumber(L, key);
  char s[2];
  s[0] = c;
  s[1] = 0;
  lua_pushstring(L, s);
  lua_pushboolean(L, ctrl);
  if(lua_pcall(L, 3, 0, 0)){
     Serial.printf("run error in keydown? %s\n", lua_tostring(L, -1));
     lgfx->setTextColor(TFT_WHITE, TFT_RED);
     lgfx->setCursor(0,0);
     lgfx->print((char*)lua_tostring(L, -1));
     runError = true;
     errorString = lua_tostring(L, -1);
     isTerminate = true;
     return;
   }
}

int LuaEngine::l_getFiles(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* path = "/";//lua_tostring(L, 1);

  lua_newtable(L);
  int i = 1;
  File dir = SD.open(path);
  File f = dir.openNextFile();
  while(f){
    lua_pushnumber(L, i);
    lua_pushstring(L, (char*)f.name());
    lua_settable(L, -3);
    f = dir.openNextFile();
    i ++;
  }
  return 1;
}

int LuaEngine::l_exists(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* fileName = lua_tostring(L, 1);

  bool exists = SD.exists(fileName);
  lua_pushboolean(L, exists);

  return 1;
}

int LuaEngine::l_saveFile(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* fileName = lua_tostring(L, 1);
  const char* body = lua_tostring(L, 2);

  File fp = SD.open(fileName, FILE_WRITE);
  fp.print(body);
  fp.close();

  return 0;
}

int LuaEngine::l_readFile(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* fileName = lua_tostring(L, 1);
  char buf[1024];

  File fp = SD.open(fileName, FILE_READ);
  lua_pushstring(L, "");
  while(fp.available()){
    int c = fp.readBytes((char*)buf, 1024);
    Serial.println(c);
    if(c == 0){
      break;
    }
    buf[c] = 0; // null terminate
    lua_pushstring(L, buf);
    lua_concat(L, 2);
    Serial.println(buf);
  }
  fp.close();

  return 1;
}

int LuaEngine::l_text(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  
  const char* text = lua_tostring(L, 1);
  const int col = lua_tointeger(L, 2);
  const int line = lua_tointeger(L, 3);

  self->lgfx->setCursor(col, line);
  self->lgfx->print(text);
  return 0;
}

int LuaEngine::l_clear(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));

  self->lgfx->fillScreen(self->color);
  return 0;
}
int LuaEngine::l_fillRect(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const int x = lua_tointeger(L, 1);
  const int y = lua_tointeger(L, 2);
  const int w = lua_tointeger(L, 3);
  const int h = lua_tointeger(L, 4);

  self->lgfx->fillRect(x, y, w, h, self->color);
  return 0;
}
int LuaEngine::l_getFreeHeap(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));

  lua_pushinteger(L, (lua_Integer)ESP.getFreeHeap());
  return 1;
}
int LuaEngine::l_textWidth(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* text = lua_tostring(L, 1);

  int width = self->lgfx->textWidth(text); // TODO: implement
  lua_pushinteger(L, (lua_Integer)width);
  return 1;
}
int LuaEngine::l_screenWidth(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));

  int width = 320; // TODO
  lua_pushinteger(L, (lua_Integer)width);
  return 1;
}
int LuaEngine::l_screenHeight(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));

  int height = 240; // TODO
  lua_pushinteger(L, (lua_Integer)height);
  return 1;
}

int LuaEngine::l_color(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  int r,g,b;
  r = lua_tointeger(L, 1);
  g = lua_tointeger(L, 2);
  b = lua_tointeger(L, 3);

  self->color = rgb24to16(r, g, b);
  self->lgfx->setTextColor(self->color);
  return 0;
}
int LuaEngine::l_debug(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* text = lua_tostring(L, 1);
  Serial.println(text);
  return 0;
}

int LuaEngine::l_run(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* fileName = lua_tostring(L, 1);

  self->fileNameStack.push_back(self->fileName);
  self->isTerminate = true;
  self->fileName = fileName;
  return 0;
}

int LuaEngine::l_exit(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  self->isTerminate = true;
  self->fileName = self->fileNameStack.back();
  self->fileNameStack.pop_back();
  return 0;
}

int LuaEngine::l_require(lua_State* L){
  bool loadError = false;
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  const char* fname = lua_tostring(L, 1);
  char fpath[128];
  sprintf(fpath, "/%s.lua", fname);

  File fp;
  if(self->isSD){
    fp = SD.open(fpath, FILE_READ);
  }else{
    fp = SPIFFS.open(fpath, FILE_READ);
  }

  struct LoadF lf;
  lf.f = fp;
  if(lua_load(L, getF, &lf, fname, NULL)){
    printf("error? %s\n", lua_tostring(L, -1));
    Serial.printf("error? %s\n", lua_tostring(L, -1));
    loadError = true;
  }
  fp.close();

  if(loadError == false){
    if(lua_pcall(L, 0, 1, 0)){
      Serial.printf("init error? %s\n", lua_tostring(L, -1));
    }
  }

  Serial.println("finish require");
  return 1;
}

void LuaEngine::exit(){
  Serial.println("CALL close!");
  Serial.println(runError);
  Serial.println(errorString);
  lua_close(L);
}

int LuaEngine::l_ksearch(lua_State* L){
  LuaEngine* self = (LuaEngine*)lua_touserdata(L, lua_upvalueindex(1));
  char* text = (char*)lua_tostring(L, 1);
  int len = strnlen(text,3);
  Serial.println(text);
  vector<string> kanjiList;

  if(len >= 3){
    Serial.printf("%02x%02x%02x\n", text[0], text[1], text[2]);
    char fname[64];
    sprintf(fname, "/dic/%02x%02x%02x.txt", text[0], text[1], text[2]);
    search(text, &kanjiList, fname);
  }
  int i = 1; // start from 1 in Lua
  lua_newtable(L);
  for(vector<string>::iterator itr = kanjiList.begin(); itr != kanjiList.end(); itr ++){
    lua_pushnumber(L, i);
    lua_pushstring(L, (*itr).c_str());
    lua_settable(L, -3);
    i ++;
  }

  return 1;
}

