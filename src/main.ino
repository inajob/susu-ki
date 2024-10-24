#include "efontEnableJa.h"
#include "efont.h"

#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
static LGFX lcd;

#include <hidboot.h>
#include <usbhub.h>

#include <luashell.h>
LuaShell luaShell;

QueueHandle_t keyQueue;

uint8_t global_key = 0;

struct KeyEvent {
  char keyCode;
  char keyChar;
  bool ctrl;
};

class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t mod, uint8_t key);

  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);

    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t c);
};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift  == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt    == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI    == 1) ? "G" : " ");

  Serial.print(" >");
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift  == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt    == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI    == 1) ? "G" : " ");
};


void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  Serial.print("DN ");
  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);
  global_key = c;
  uint8_t ctrl = (mod & 0x11);
  switch(key){
    case 40: key = 13; break; // Enter
    case 42: key = 8; break; // BS
    case 80: key = 37; break; // Letf
    case 79: key = 39; break; // Right
    case 82: key = 38; break; // Up
    case 81: key = 40; break; // Down
    case 44: key = 32; break; // space
    //case x: key = 9; break; // Tab
    default: key = 0;
  }
  struct KeyEvent event = {key, c, ctrl};
  xQueueSend( keyQueue, &event, 0 );
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {

  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");
  }

}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  Serial.print("UP ");
  PrintKey(mod, key);
}

void KbdRptParser::OnKeyPressed(uint8_t c)
{
 /* no use */
  Serial.print("ASCII: ");
  Serial.println((char)c);
};

USB     Usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;

TaskHandle_t thp[1];

void setup(){
  Serial.begin(115200);
  SPIFFS.begin();

  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  //lcd.setColorDepth(16);  // RGB565の16ビットに設定
  lcd.setColorDepth(24);  // RGB888の24ビットに設定(表示される色数はパネル性能によりRGB666の18ビットになります)
  lcd.setFont(&fonts::efont);

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );
  HidKeyboard.SetReportParser(0, &Prs);

  keyQueue = xQueueCreate( 10, sizeof(struct KeyEvent) );
  xTaskCreatePinnedToCore(Core0a, "Core0a", 4096, NULL, 3, &thp[0], 0);

  luaShell.init(&lcd);
}

int count1 = 0;
void loop(){
  char buf[256];
  delay(1);
  count1 ++;
  struct KeyEvent event;
  if(xQueueReceive( keyQueue, &event, 0 )){
    //sprintf(buf, "loop %d [%c][%d]", count1, event.keyChar, event.keyCode);
    //lcd.drawString(buf, 10, 200);
    Serial.printf("get key down [%c]\n", event.keyChar);
    luaShell.onkeydown(event.keyCode, event.keyChar, event.ctrl);
  }
}

// USB処理用コア
void Core0a(void *args) {
  while (1) {
    Usb.Task();
    delay(1);
  }
}
