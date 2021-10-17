#include <stdint.h>
#include <ss_oled.h>

#ifndef H_A
#define H_A

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy

#endif

#ifndef H_CELL_DATA
#define H_CELL_DATA

#include "cell_data.cpp"

#endif


// Not enough memory
//#define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

// Use -1 for the Wire library default pins
// or specify the pin numbers to use with the Wire library or bit banging on any GPIO pins
// These are the pin numbers for the M5Stack Atom default I2C
#define SDA_PIN A4
#define SCL_PIN A5
// Set this to -1 to disable or the GPIO pin number connected to the reset
// line of your display if it requires an external reset
#define RESET_PIN -1
// let ss_oled figure out the display address
#define OLED_ADDR -1
// don't rotate the display
#define FLIP180 0
// don't invert the display
#define INVERT 0
// Bit-Bang the I2C bus
#define USE_HW_I2C 0

// My oled
#define MY_OLED OLED_128x64
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

class CellDisplay {
  private:
    SSOLED ssoled;

  public:
    void setup(uint8_t fwVersionMajor, uint8_t fwVersionMinor){
      int rc;
      char szTemp[32];
      
      rc = oledInit(&ssoled, MY_OLED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L); // use standard I2C bus at 400Khz
        if (rc != OLED_NOT_FOUND){
          Serial.print("Display Found");
          Serial.println();
          char *msgs[] = {(char *)"SSD1306 @ 0x3C", (char *)"SSD1306 @ 0x3D",(char *)"SH1106 @ 0x3C",(char *)"SH1106 @ 0x3D"};
          oledFill(&ssoled, 0, 1);
          sprintf(szTemp, "FW %d.%d", fwVersionMajor, fwVersionMinor);
          oledWriteString(&ssoled, 0, 0, 0, szTemp, FONT_NORMAL, 0, 1);
          // oledWriteString(&ssoled, 0,0,0,msgs[rc], FONT_NORMAL, 0, 1);
          oledSetBackBuffer(&ssoled, ucBackBuffer);
          delay(1000);
        }else{
          Serial.print("Display Not Found");
          Serial.println();
        }
    }

    void loop(CellData cellData){
      int i, x, y;
      char szTemp[32];
      unsigned long ms;

      int16_t maxCellV = 0;
      int16_t minCellV = 6000;
      for (int i = 0; i < cellData.cellCount; i++) {
        if (cellData.voltages[i] < minCellV) {
          minCellV = cellData.voltages[i];
        }
        if (cellData.voltages[i] > maxCellV) {
          maxCellV = cellData.voltages[i];
        }
      }

      // clear screen
      oledFill(&ssoled, 0x0, 1);

      // draw individual cell voltages
      int cols = 3;
      int rows = 4;
      int dX = OLED_WIDTH / cols;
      int dY = 10;
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int index = i * cols + j;
          int x = dX * j;
          int y = dY * i;
          int16_t cv = cellData.voltages[index];
          sprintf(szTemp, "%d", cv);
          if (cv == minCellV || cv == maxCellV) {
            oledWriteString(&ssoled, 0,x,i,szTemp, FONT_NORMAL, 0, 1);
          } else {
            oledWriteString(&ssoled, 0,x,i,szTemp, FONT_SMALL, 0, 1);
          }
          
        }
      }

      // sprintf doesn't support %f formatter on this platform
      char strTemp[6];
      dtostrf((float)(cellData.batteryVoltage/1000.0), 4, 1, strTemp);

      // draw total pack voltage
      sprintf(szTemp, "%sV", strTemp);
      oledWriteString(&ssoled, 0, 0, rows+1, szTemp, FONT_STRETCHED, 0, 1);
        
    }
};
