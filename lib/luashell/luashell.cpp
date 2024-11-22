#include <luashell.h>
#include <SD.h>

// Define CS pin for the SD card module
#define SD_MISO     19
#define SD_MOSI     23
#define SD_SCLK     18
#define SD_CS       4
SPIClass sdSPI(VSPI);

void LuaShell::copyToSD(char* name){
  File fp = SPIFFS.open(name, FILE_READ);
  File fp2 = SD.open(name, FILE_WRITE);
  char buf[1024];

  while (fp.available()) {
    fp2.write(fp.read());
  }

  fp.close();
  fp2.close();
}
void LuaShell::init(LGFX *lgfx){
  rawInputsItr = rawInputs.begin();
  isTerminate = false;
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS, sdSPI)){
    Serial.println("SD init failed");
  }
  copyToSD("/shell.lua");
  copyToSD("/edit.lua");
  copyToSD("/skk.lua");
  copyToSD("/prompt.lua");
  copyToSD("/alert.lua");

  lua.init(lgfx);
}
bool LuaShell::onkeydown(char key, char c, bool ctrl){
  lua.keydown(key, c, ctrl);

  isTerminate = lua.isTerminate;
  if(isTerminate == 1){
    lua.exit();
    lua.init(lua.lgfx);
  }
  return true;
}