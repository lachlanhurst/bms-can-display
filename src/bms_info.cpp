
#include <SPI.h>
#include <mcp2515.h>

#ifndef H_A
#define H_A

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy

#endif

#ifndef H_CELL_DATA
#define H_CELL_DATA

#include "cell_data.cpp"

#endif

#define BMS_CAN_ID 10
#define BMS_DISPLAY_CAN_ID 3

// from https://github.com/SimosMCmuffin/FlexiBMS_Lite_FW/blob/master/Inc/datatypes.h#L23
// CAN commands (from VESC FW)
typedef enum {
  CAN_PACKET_SET_DUTY = 0,
  CAN_PACKET_SET_CURRENT,
  CAN_PACKET_SET_CURRENT_BRAKE,
  CAN_PACKET_SET_RPM,
  CAN_PACKET_SET_POS,
  CAN_PACKET_FILL_RX_BUFFER,
  CAN_PACKET_FILL_RX_BUFFER_LONG,
  CAN_PACKET_PROCESS_RX_BUFFER,
  CAN_PACKET_PROCESS_SHORT_BUFFER,
  CAN_PACKET_STATUS,
  CAN_PACKET_SET_CURRENT_REL,
  CAN_PACKET_SET_CURRENT_BRAKE_REL,
  CAN_PACKET_SET_CURRENT_HANDBRAKE,
  CAN_PACKET_SET_CURRENT_HANDBRAKE_REL,
  CAN_PACKET_STATUS_2,
  CAN_PACKET_STATUS_3,
  CAN_PACKET_STATUS_4,
  CAN_PACKET_PING,
  CAN_PACKET_PONG,
  CAN_PACKET_DETECT_APPLY_ALL_FOC,
  CAN_PACKET_DETECT_APPLY_ALL_FOC_RES,
  CAN_PACKET_CONF_CURRENT_LIMITS,
  CAN_PACKET_CONF_STORE_CURRENT_LIMITS,
  CAN_PACKET_CONF_CURRENT_LIMITS_IN,
  CAN_PACKET_CONF_STORE_CURRENT_LIMITS_IN,
  CAN_PACKET_CONF_FOC_ERPMS,
  CAN_PACKET_CONF_STORE_FOC_ERPMS,
  CAN_PACKET_STATUS_5
} CAN_PACKET_ID;


// from https://github.com/SimosMCmuffin/FlexiBMS_Lite_FW/blob/master/Inc/datatypes.h#L56
// Communication commands (from DieBieMS FW)
typedef enum {
  COMM_FW_VERSION = 0,
  COMM_JUMP_TO_BOOTLOADER,
  COMM_ERASE_NEW_APP,
  COMM_WRITE_NEW_APP_DATA,
  COMM_GET_VALUES,
  COMM_SET_DUTY,
  COMM_SET_CURRENT,
  COMM_SET_CURRENT_BRAKE,
  COMM_SET_RPM,
  COMM_SET_POS,
  COMM_SET_HANDBRAKE,
  COMM_SET_DETECT,
  COMM_SET_SERVO_POS,
  COMM_SET_BMS_CONF,
  COMM_GET_BMS_CONF,
  COMM_GET_MCCONF_DEFAULT,
  COMM_SET_APPCONF,
  COMM_GET_APPCONF,
  COMM_GET_APPCONF_DEFAULT,
  COMM_SAMPLE_PRINT,
  COMM_TERMINAL_CMD,
  COMM_PRINT,
  COMM_ROTOR_POSITION,
  COMM_EXPERIMENT_SAMPLE,
  COMM_DETECT_MOTOR_PARAM,
  COMM_DETECT_MOTOR_R_L,
  COMM_DETECT_MOTOR_FLUX_LINKAGE,
  COMM_DETECT_ENCODER,
  COMM_DETECT_HALL_FOC,
  COMM_REBOOT,
  COMM_ALIVE,
  COMM_GET_DECODED_PPM,
  COMM_GET_DECODED_ADC,
  COMM_GET_DECODED_CHUK,
  COMM_FORWARD_CAN,
  COMM_SET_CHUCK_DATA,
  COMM_CUSTOM_APP_DATA,
  COMM_NRF_START_PAIRING,
  COMM_STORE_BMS_CONF = 50,
  COMM_GET_BMS_CELLS
} COMM_PACKET_ID;


class BmsInfo {
  private:
    MCP2515 mcp2515;
  
    struct can_frame responses[10];
    int responsesLength = 0;

    uint8_t readBuffer[50];
    uint8_t readBufferLength = 0;
    uint8_t readBufferInfo[8];
    uint8_t readBufferInfoLength = 0;


  public:
    uint8_t fwVersionMajor;
    uint8_t fwVersionMinor;
    
    BmsInfo::BmsInfo() :
      mcp2515(10){
    }


  void setup(CellData *cellData){
    SPI.begin();
      
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();

//    cellData->voltages[0] = 1234;
//    cellData->voltages[1] = 4321;
//    cellData->voltages[2] = 3000;
//    cellData->voltages[3] = 2222;
//    cellData->voltages[4] = 3333;
//    cellData->voltages[5] = 4444;
//    cellData->voltages[6] = 5893;
//    cellData->voltages[7] = 3344;
//    cellData->voltages[8] = 1212;
//    cellData->voltages[9] = 2121;
//    cellData->voltages[10] = 2323;
//    cellData->voltages[11] = 3232;

    getFwVersion();
  }

  void loop(CellData *cellData){
    getCells(cellData);
    getValues(cellData);
  }


  void getFwVersion() {
    struct can_frame canMsg1;
    canMsg1.can_id  = (uint32_t (0x8000) << 16) + (uint16_t (CAN_PACKET_PROCESS_SHORT_BUFFER) << 8) + BMS_CAN_ID;
    canMsg1.can_dlc = 0x03;
    canMsg1.data[0] = BMS_DISPLAY_CAN_ID;
    canMsg1.data[1] = 0x00;
    canMsg1.data[2] = COMM_FW_VERSION;
    mcp2515.sendMessage(&canMsg1);
    
    batchRead();
    parseFwVersion();
  }

  bool parseFwVersion() {
    uint8_t comm_fw_version = readBuffer[0];
    fwVersionMajor = readBuffer[1];
    fwVersionMinor = readBuffer[2];
  }

  void getCells(CellData *cellData) {
    struct can_frame canMsg1;
    canMsg1.can_id  = (uint32_t (0x8000) << 16) + (uint16_t (CAN_PACKET_PROCESS_SHORT_BUFFER) << 8) + BMS_CAN_ID;
    canMsg1.can_dlc = 0x03;
    canMsg1.data[0] = BMS_DISPLAY_CAN_ID;
    canMsg1.data[1] = 0x00;
    canMsg1.data[2] = COMM_GET_BMS_CELLS;
    mcp2515.sendMessage(&canMsg1);
    
    batchRead();
    parseCells(cellData);
  }

  bool parseCells(CellData *cellData) {
    int32_t ind = 0;
    uint8_t commGetBmsCells = readBuffer[ind++];
    cellData->cellCount = readBuffer[ind++];

    for (int i = 0; i < cellData->cellCount ; i++) {
      int16_t cv = buffer_get_int16(readBuffer, &ind);
      cellData->voltages[i] = cv;
    }
  }

  void getValues(CellData *cellData) {
    struct can_frame canMsg1;
    canMsg1.can_id  = (uint32_t (0x8000) << 16) + (uint16_t (CAN_PACKET_PROCESS_SHORT_BUFFER) << 8) + BMS_CAN_ID;
    canMsg1.can_dlc = 0x03;
    canMsg1.data[0] = BMS_DISPLAY_CAN_ID;
    canMsg1.data[1] = 0x00;
    canMsg1.data[2] = COMM_GET_VALUES;
    mcp2515.sendMessage(&canMsg1);
    
    batchRead();
    parseValues(cellData);
  }

  bool parseValues(CellData *cellData) {
    int32_t ind = 0;
    uint8_t commGetValues = readBuffer[ind++];

    cellData->batteryVoltage = buffer_get_int32(readBuffer, &ind);
    cellData->chargeCurrent = buffer_get_int32(readBuffer, &ind);

    uint8_t soc = readBuffer[ind++];  // not implemented on FlexiBMS

    cellData->cellVoltageHigh = buffer_get_int32(readBuffer, &ind);
    cellData->cellVoltageAverage = buffer_get_int32(readBuffer, &ind);
    cellData->cellVoltageLow = buffer_get_int32(readBuffer, &ind);
    cellData->cellVoltageMisMatch = buffer_get_int32(readBuffer, &ind);

//    Serial.println("values");
//    Serial.println(cellData->batteryVoltage);
//    Serial.println(cellData->chargeCurrent);
//    Serial.println(soc);
//    Serial.println(cellData->cellVoltageHigh);
//    Serial.println(cellData->cellVoltageLow);

  }

  bool batchRead(){

    responsesLength = 0;
    for(int j = 0; j < 1000 && responsesLength < 10; j++){
      if (mcp2515.readMessage(&responses[responsesLength]) == MCP2515::ERROR_OK) {
        responsesLength++;
      }
    }

//    for(int i = 0; i < responsesLength; i++){
//      printFrame(&responses[i]);
//    }

    // Clear buffer
    readBufferLength = 0;
    for(int i = 0; i < 50; i++){
      readBuffer[i] = 0; //(Not really necessary ?)
    }
    readBufferInfoLength = 0;
    for(int i = 0; i < 8; i++){
      readBufferInfo[i] = 0; //(Not really necessary ?)
    }
    // Convert can frames to full buffer
    for(int i = 0; i < responsesLength; i++){
      if(responses[i].can_id == 0x80000000 + ((uint16_t)CAN_PACKET_FILL_RX_BUFFER << 8) + BMS_DISPLAY_CAN_ID){
        for(int j = 1; j < responses[i].can_dlc; j++){
          readBuffer[responses[i].data[0]+j-1] = responses[i].data[j];
          readBufferLength++;
        }
      }else if(responses[i].can_id == 0x80000000 + ((uint16_t)CAN_PACKET_PROCESS_RX_BUFFER << 8) + BMS_DISPLAY_CAN_ID){
        for(int j = 0; j < responses[i].can_dlc; j++){
          readBufferInfo[j] = responses[i].data[j];
          readBufferInfoLength++;
        }
      }
    }

//    for(int i = 0; i < readBufferLength; i++){
//      Serial.print(readBuffer[i],HEX);
//      Serial.print(" ");
//    }
//    Serial.println();
//
//    for(int i = 0; i < readBufferInfoLength; i++){
//      Serial.print(readBufferInfo[i],HEX);
//      Serial.print(" ");
//    }
//    Serial.println();

    // TODO: Proper read validation with checksum, for now we just compare lengths, can improove if data is error prone.
    uint16_t supposedLength = ((uint16_t)readBufferInfo[2] << 8) + readBufferInfo[3];
    if(readBufferLength != supposedLength){
      readBufferLength = 0;
      readBufferInfoLength = 0;
    }
  }


  void printFrame(struct can_frame *frame){
    Serial.print(frame->can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(frame->can_dlc, HEX); // print DLC
    Serial.print(" ");
    for (int i = 0; i < frame->can_dlc; i++)  {  // print the data
      Serial.print(frame->data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }


  int16_t buffer_get_int16(const uint8_t *buffer, int32_t *index) {
    int16_t res = ((uint16_t) buffer[*index]) << 8 | ((uint16_t) buffer[*index + 1]);
    *index += 2;
    return res;
  }

  int32_t buffer_get_int32(const uint8_t *buffer, int32_t *index) {
    int32_t res = ((uint32_t) buffer[*index]) << 24 |
            ((uint32_t) buffer[*index + 1]) << 16 |
            ((uint32_t) buffer[*index + 2]) << 8 |
            ((uint32_t) buffer[*index + 3]);
    *index += 4;
    return res;
  }

};
