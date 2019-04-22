#include "ada.h"
#include <SPI.h>
#include <string.h>

uint8_t *vram;

#define CS 10
#define DC 9
#define RES 8


/////////////////////////////////////////////////////////////////////////
void command(uint8_t c) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  SPI.transfer(c);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void sterge() {
  memset(vram, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/////////////////////////////////////////////////////////////////////////
void start() {

  if (vram)
    free(vram);
  vram = NULL;

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  if ((!vram) && !(vram = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    return false;

  sterge();

  digitalWrite(RES, HIGH);
  delay(1);                   // VDD goes high at start, pause for 1 ms
  digitalWrite(RES, LOW);  // Bring RESet low
  delay(10);                  // Wait 10 ms
  digitalWrite(RES, HIGH); // Bring out of RESet

  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  SPI.transfer(SSD1306_DISPLAYOFF);
  SPI.transfer(SSD1306_SETDISPLAYCLOCKDIV);
  SPI.transfer(0x80);
  SPI.transfer(SSD1306_SETMULTIPLEX);
  SPI.transfer(HEIGHT - 1);
  SPI.transfer(SSD1306_SETDISPLAYOFFSET);
  SPI.transfer(0x00);
  SPI.transfer(SSD1306_SETSTARTLINE | 0x0);
  SPI.transfer(SSD1306_CHARGEPUMP);
  SPI.transfer(0x14);
  SPI.transfer(SSD1306_MEMORYMODE);
  SPI.transfer(0x00);
  SPI.transfer(SSD1306_SEGREMAP | 0x1);
  SPI.transfer(SSD1306_COMSCANDEC);
  SPI.transfer(SSD1306_SETCOMPINS);
  SPI.transfer(0x12);
  SPI.transfer(SSD1306_SETCONTRAST);
  SPI.transfer(0xCF);
  SPI.transfer(SSD1306_SETPRECHARGE);
  SPI.transfer(0xF1);
  SPI.transfer(SSD1306_SETVCOMDETECT);
  SPI.transfer(0x40);
  SPI.transfer(SSD1306_DISPLAYALLON_RESUME);
  SPI.transfer(SSD1306_NORMALDISPLAY);
  SPI.transfer(SSD1306_DEACTIVATE_SCROLL);
  SPI.transfer(SSD1306_DISPLAYON);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void contrast(uint8_t contrast) {
  //default 0xCF
  //range from 0 to 255
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  SPI.transfer(SSD1306_SETCONTRAST);
  SPI.transfer(contrast);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void afisare(void) {

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);

  SPI.transfer(SSD1306_PAGEADDR);
  SPI.transfer(0);                    // Page start addRESs
  SPI.transfer(0xFF);                    // Page end (not really, but works here)
  SPI.transfer(SSD1306_COLUMNADDR);
  SPI.transfer(0);
  SPI.transfer(WIDTH - 1); // Column end addRESs

  uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
  uint8_t *ptr   = vram;

  digitalWrite(DC, HIGH);
  while (count--) {
  SPI.transfer(*ptr++);
  }

  digitalWrite(CS, HIGH);
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
  Serial.begin(9600);

  pinMode(DC, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(RES, OUTPUT);

  start();
  delay(500);
  pixel(0, 1, WHITE);
  pixel(1, 1, WHITE);
  afisare();
  delay(500);
}

void loop() {
  command(0xA5);
  delay(500);
  command(0xA4);
  delay(500);
}
