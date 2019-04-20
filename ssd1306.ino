#include "ada.h"
#include <SPI.h>
#include <string.h>

uint8_t *vram;
int8_t vccstate = SSD1306_SWITCHCAPVCC; //3.3v psu

#define cs 10
#define dc 9
#define res 8


/////////////////////////////////////////////////////////////////////////
void ssd1306_command1(uint8_t c) {
  digitalWrite(dc, LOW);
  SPI.transfer(c);
}

/////////////////////////////////////////////////////////////////////////
void ssd1306_commandList(const uint8_t *c, uint8_t n) {
  digitalWrite(dc, LOW);
  while (n--) SPI.transfer(pgm_read_byte(c++));
}

/////////////////////////////////////////////////////////////////////////
void ssd1306_command(uint8_t c) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  digitalWrite(dc, LOW);
  SPI.transfer(c);
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void clearDisplay() {
  memset(vram, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/////////////////////////////////////////////////////////////////////////
void start() {

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  if ((!vram) && !(vram = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    return false;

  clearDisplay();
  afisare();

  digitalWrite(res, HIGH);
  delay(1);                   // VDD goes high at start, pause for 1 ms
  digitalWrite(res, LOW);  // Bring reset low
  delay(10);                  // Wait 10 ms
  digitalWrite(res, HIGH); // Bring out of reset

  digitalWrite(cs, LOW);

  // Init sequence
  static const uint8_t PROGMEM init1[] = {
    SSD1306_DISPLAYOFF,                   // 0xAE
    SSD1306_SETDISPLAYCLOCKDIV,           // 0xD5
    0x80,                                 // the suggested ratio 0x80
    SSD1306_SETMULTIPLEX
  };               // 0xA8
  ssd1306_commandList(init1, sizeof(init1));
  ssd1306_command1(HEIGHT - 1);

  static const uint8_t PROGMEM init2[] = {
    SSD1306_SETDISPLAYOFFSET,             // 0xD3
    0x0,                                  // no offset
    SSD1306_SETSTARTLINE | 0x0,           // line #0
    SSD1306_CHARGEPUMP
  };                 // 0x8D
  ssd1306_commandList(init2, sizeof(init2));

  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

  static const uint8_t PROGMEM init3[] = {
    SSD1306_MEMORYMODE,                   // 0x20
    0x00,                                 // 0x0 act like ks0108
    SSD1306_SEGREMAP | 0x1,
    SSD1306_COMSCANDEC
  };        //0xC8
  ssd1306_commandList(init3, sizeof(init3));

  static const uint8_t PROGMEM init4b[] = {
    SSD1306_SETCOMPINS,                 // 0xDA
    0x12,
    SSD1306_SETCONTRAST
  };              // 0x81
  ssd1306_commandList(init4b, sizeof(init4b));
  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);

  ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
  static const uint8_t PROGMEM init5[] = {
    SSD1306_SETVCOMDETECT,               // 0xDB
    0x40,
    SSD1306_DISPLAYALLON_RESUME,         // 0xA4
    SSD1306_NORMALDISPLAY,               // 0xA6
    SSD1306_DEACTIVATE_SCROLL,
    SSD1306_DISPLAYON                    // Main screen turn on
  };
  ssd1306_commandList(init5, sizeof(init5));

  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void contrast(uint8_t contrast) {
  //default 0xCF
  //range from 0 to 255
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  ssd1306_command1(SSD1306_SETCONTRAST);
  ssd1306_command1(contrast);
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void afisare(void) {

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);

  static const uint8_t PROGMEM dlist1[] = {
    SSD1306_PAGEADDR,
    0,                         // Page start address
    0xFF,                      // Page end (not really, but works here)
    SSD1306_COLUMNADDR,
    0
  };                       // Column start address
  ssd1306_commandList(dlist1, sizeof(dlist1));
  ssd1306_command1(WIDTH - 1); // Column end address

  uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
  uint8_t *ptr   = vram;

  digitalWrite(dc, HIGH);
  while (count--) {
    SPI.transfer(*ptr);
  }

  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void pixel(int16_t x, int16_t y, uint16_t color) {
  switch (color) {
    case WHITE:   vram[x + (y / 8)*WIDTH] |=  (1 << (y & 7)); break;
    case BLACK:   vram[x + (y / 8)*WIDTH] &= ~(1 << (y & 7)); break;
    case INVERSE: vram[x + (y / 8)*WIDTH] ^=  (1 << (y & 7)); break;
  }
}

/////////////////////////////////////////////////////////////////////////
void setup() {
  
  if (vram) {
    free(vram);
    vram = NULL;
  }
  Serial.begin(9600);

  pinMode(dc, OUTPUT);
  pinMode(cs, OUTPUT);
  pinMode(res, OUTPUT);

  Serial.print("a");
  start();
  Serial.print("b");
  pixel(0, 0, WHITE);
  pixel(1, 0, WHITE);
  Serial.print("c");
  afisare();
  delay(500);
  contrast(0xcf);
  ssd1306_command(0xA5);
}

void loop() {
  for (uint8_t i = 0; i <= 255; i += 15) {
    contrast(i);
    delay(200);
  }
}
