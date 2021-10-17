
#ifndef H_CELL_DATA
#define H_CELL_DATA

#include "cell_data.cpp"

#endif

#include "bms_info.cpp"
#include "cell_display.cpp"

CellData cellData;
BmsInfo bmsInfo;
CellDisplay cellDisplay;



void setup() {

  Serial.begin(115200);


  bmsInfo.setup(&cellData);
  cellDisplay.setup(bmsInfo.fwVersionMajor, bmsInfo.fwVersionMinor);

  Serial.print("BMS firmware version: ");
  Serial.print(bmsInfo.fwVersionMajor);
  Serial.print(".");
  Serial.print(bmsInfo.fwVersionMinor);
  Serial.println();

}




void loop() {

//  Serial.println("update");
//  for(int i = 0; i < 12; i++)
//  {
//    Serial.println(cellData.voltages[i]);
//  }
//  Serial.println();

  bmsInfo.loop(&cellData);
  cellDisplay.loop(cellData);

  delay(2000);
 
}
