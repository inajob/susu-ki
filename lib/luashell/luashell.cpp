#include <luashell.h>
#include <SD.h>

// Define CS pin for the SD card module
#define SD_MISO     19
#define SD_MOSI     23
#define SD_SCLK     18
#define SD_CS       4
SPIClass sdSPI(VSPI);

void LuaShell::init(LGFX *lgfx){
  rawInputsItr = rawInputs.begin();
  isTerminate = false;
  lua.init(lgfx);
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS, sdSPI)){
    Serial.println("SD init failed");
  }
}
bool LuaShell::onkeydown(char key, char c, bool ctrl){
  lua.keydown(key, c, ctrl);

  isTerminate = lua.isTerminate;
  if(isTerminate == 1){
    lua.exit();
  }
  return true;
}