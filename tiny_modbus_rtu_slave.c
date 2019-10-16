/*
Based on ideas of https://github.com/angeloc/simplemodbusng

The crc calculation is based on the work published 
 by jpmzometa at 
 http://sites.google.com/site/jpmzometa/arduino-mbrt
 
 By Juan Bester : bester.juan@gmail.com
*/

// (C) freexlamer@github.com


#include "tiny_modbus_rtu_slave.h"

unsigned char frame[MODBUS_BUFFER_SIZE];
unsigned char slaveID;
unsigned char function;
bool broadcastFlag;

unsigned char buffer = 0;
bool overflow = false;

uint64_t modbus_last_packet_time;

// function definitions
void exceptionResponse(unsigned char exception);
unsigned int calculate_CRC16(unsigned char start, unsigned char count);
void modbus_send_packet(unsigned char start, unsigned char count);
bool testAddress(unsigned int address);

void modbus_buffer_flush() {
  buffer = 0;
}

void modbus_init() {
  modbus_error_count = 0;
  modbus_crc_errors = 0;
  modbus_buffer_flush();
  modbus_last_packet_time = millis();
}

bool modbus_data_is_enough(unsigned char start) {
  unsigned char function_code_index = start + 1;
  if (function_code_index < buffer) {
    if ((frame[function_code_index]==MODBUS_FUNCTION_READ_AO) ||
        (frame[function_code_index]==MODBUS_FUNCTION_WRITE_AO) ||
        (frame[function_code_index]==MODBUS_FUNCTION_READ_DO) ||
        (frame[function_code_index]==MODBUS_FUNCTION_READ_DI) ||
        (frame[function_code_index]==MODBUS_FUNCTION_READ_AI) ||
        (frame[function_code_index]==MODBUS_FUNCTION_WRITE_DO)) {
      if ((function_code_index+MODBUS_4_BYTES_PDU_SIZE+MODBUS_CRC_SIZE) < buffer) {
        return true;
      }
    }
    else if ((frame[function_code_index]==MODBUS_FUNCTION_WRITE_MANY_DO) ||
             (frame[function_code_index]==MODBUS_FUNCTION_WRITE_MANY_AO)) {
      unsigned char data_bytes_count_index = function_code_index + MODBUS_4_BYTES_PDU_SIZE + 1;
      if (data_bytes_count_index < buffer) {
        unsigned char data_bytes_count = frame[data_bytes_count_index];
        if ((data_bytes_count_index + data_bytes_count + MODBUS_CRC_SIZE) < buffer) {
          return true;
        }
      }
    }
  }
  return false;
}

bool modbus_data_check_crc(unsigned char start) {
  unsigned int crc;
  unsigned char check_data_count;
  unsigned char data_bytes_count;

  unsigned char function_code_index = start + 1;

  if ((frame[function_code_index]==MODBUS_FUNCTION_READ_AO) ||
      (frame[function_code_index]==MODBUS_FUNCTION_WRITE_AO) ||
      (frame[function_code_index]==MODBUS_FUNCTION_READ_DO) ||
      (frame[function_code_index]==MODBUS_FUNCTION_READ_DI) ||
      (frame[function_code_index]==MODBUS_FUNCTION_READ_AI) ||
      (frame[function_code_index]==MODBUS_FUNCTION_WRITE_DO)) {
    check_data_count = MODBUS_HEADER_SIZE + MODBUS_4_BYTES_PDU_SIZE;
  }
  else if ((frame[function_code_index]==MODBUS_FUNCTION_WRITE_MANY_DO) ||
           (frame[function_code_index]==MODBUS_FUNCTION_WRITE_MANY_AO)) {
    data_bytes_count = frame[function_code_index + MODBUS_4_BYTES_PDU_SIZE + 1];
    check_data_count = MODBUS_HEADER_SIZE + MODBUS_4_BYTES_PDU_SIZE + 1 + data_bytes_count;
  }
  // combine the crc Low & High bytes
  crc = ((frame[start + check_data_count + 0] << 8) | frame[start + check_data_count + 1]);
  return (calculate_CRC16(start, check_data_count) == crc);
}

void decode_command(unsigned char start) {
  unsigned char id = frame[start];
  broadcastFlag = (id == MODBUS_BROADCAST_ID);
  unsigned int i;

  if (id == slaveID || broadcastFlag) {
    if (modbus_data_check_crc(0)) {
      function = frame[start+1];
      unsigned int crc16;

      if (!broadcastFlag && (function == MODBUS_FUNCTION_READ_AO)) {
        unsigned int starting_address = ((frame[start+2] << 8) | frame[start+3]);
        unsigned int count_of_registers = ((frame[start+4] << 8) | frame[start+5]);

        if (testAddress(starting_address)) {
            #ifdef MODBUS_FUNCTION_READ_AO_READ_MANY
              unsigned int temp = 0;
              unsigned char count_of_bytes = count_of_registers * 2;
              unsigned char responseFrameSize = MODBUS_HEADER_SIZE + 1 + count_of_bytes + MODBUS_CRC_SIZE;
              if (responseFrameSize <= MODBUS_BUFFER_SIZE) {
                bool succes = true;
                frame[0] = slaveID;
                frame[1] = function;
                frame[2] = count_of_bytes;
                unsigned char tmp_index = 3;
                for (i=0; i<count_of_registers; i++) {
                  if (testAddress(starting_address + i)) {
                    if ((*modbus_read_reg)(starting_address + i, &temp)) {
                      // or [3+i*2]
                      frame[tmp_index] = temp >> 8;
                      tmp_index++;
                      // or [3+i*2+1]
                      frame[tmp_index] = temp & 0x00FF;
                      tmp_index++;
                    } else {
                      exceptionResponse(MODBUS_ERROR_SLAVE_DEVICE_FAILURE);
                      succes = false;
                      break;
                    }
                  }
                  else {
                    exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
                    succes = false;
                    break;
                  }
                }

                if (succes) {
                  crc16 = calculate_CRC16(0, responseFrameSize - 2);
                  frame[responseFrameSize - 2] = crc16 >> 8;
                  frame[responseFrameSize - 1] = crc16 & 0xFF;
                  modbus_send_packet(0,responseFrameSize);
                  modbus_buffer_flush();
                }
              }
              else
                exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
            #else
              if (count_of_registers == 1) {
                unsigned int temp = 0;

                if ((*modbus_read_reg)(starting_address, &temp)) {
                  unsigned char count_of_bytes = count_of_registers * 2;
                  unsigned char responseFrameSize = 5 + count_of_bytes;
                  frame[0] = slaveID;
                  frame[1] = function;
                  frame[2] = count_of_bytes;
                  frame[3] = temp >> 8;
                  frame[4] = temp & 0x00FF;
                  crc16 = calculate_CRC16(0, responseFrameSize - 2);
                  frame[responseFrameSize - 2] = crc16 >> 8;
                  frame[responseFrameSize - 1] = crc16 & 0xFF;
                  modbus_send_packet(0,responseFrameSize);
                  buffer = 0;
                }
                else
                  exceptionResponse(MODBUS_ERROR_SLAVE_DEVICE_FAILURE);
              }
              else {
                  exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
              }
          #endif
        }
        else
          exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
      }
      else if (function == MODBUS_FUNCTION_WRITE_AO) {
        unsigned int starting_address = ((frame[start+2] << 8) | frame[start+3]);

        if (testAddress(starting_address)) {
          unsigned int regStatus = ((frame[start+4] << 8) | frame[start+5]);
          if ((*modbus_write_reg)(starting_address,regStatus)){
            modbus_send_packet(start, MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE);
            buffer = 0;
          }
          else
            exceptionResponse(MODBUS_ERROR_SLAVE_DEVICE_FAILURE);
        }
        else
          exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
      }
      #ifdef MODBUS_FUNCTION_WRITE_MANY_AO_ENABLED
      else if (function == MODBUS_FUNCTION_WRITE_MANY_AO) {
        bool succes = true;

        unsigned int starting_address = ((frame[start+2] << 8) | frame[start+3]);
        unsigned int count_of_registers = ((frame[start+4] << 8) | frame[start+5]);
        unsigned int tmp_index = 0;

        for (i=0; i<count_of_registers; i++) {
          if (!testAddress(starting_address+i)) {
            exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
            succes = false;
            break;
          }
        }

        if (succes) {
          succes = true;
          for (i=0; i<count_of_registers; i++) {
            unsigned int data = ((frame[start+7+tmp_index] << 8) | frame[start+7+tmp_index+1]);
            tmp_index = tmp_index + 2;
            if (!(*modbus_write_reg)(starting_address+i,data)){
              exceptionResponse(MODBUS_ERROR_SLAVE_DEVICE_FAILURE);
              succes = false;
              break;
            }
          }
          if (succes) {
            frame[0] = slaveID;
            frame[1] = function;
            frame[2] = starting_address >> 8;
            frame[3] = starting_address & 0x00FF;
            frame[4] = count_of_registers >> 8;
            frame[5] = count_of_registers & 0x00FF;
            crc16 = calculate_CRC16(0, 6);
            frame[6] = crc16 >> 8;
            frame[7] = crc16 & 0x00FF;
            modbus_send_packet(0, 8);
            modbus_buffer_flush();
          }
        }
      }
      #endif
      else
        exceptionResponse(MODBUS_ERROR_ILLEGAL_FUNCTION);

    }
    #ifdef MODBUS_ERROR_CRC_ENABLED
    else
    {
      //modbus_send_packet(buffer);
      exceptionResponse(MODBUS_ERROR_CRC);
    }
    #endif
  }

}

unsigned char pull_port(int c){

  if (c == -1) {
    return 0;
  }

  uint64_t current_time = millis();
  if ( (current_time - modbus_last_packet_time) > MODBUS_COMMAND_TIMEOUT) {
    modbus_buffer_flush();
  }

  modbus_last_packet_time = current_time;

  if (!overflow) {
    if (buffer == MODBUS_BUFFER_SIZE) {
      overflow = true;
    }

    frame[buffer] = (unsigned char)c;
    buffer++;
	}

  if (overflow) {
    overflow = false;

    #ifdef MODBUS_ERROR_BUFFER_OVERFLOW_ENABLED
      exceptionResponse(MODBUS_ERROR_BUFFER_OVERFLOW);
      return modbus_error_count;
    #else
      modbus_buffer_flush();
      return modbus_error_count++;
    #endif
  }

  if (modbus_data_is_enough(0)) {
    decode_command(0);
  }
  else
  {
    return 0;
  }
}


unsigned int calculate_CRC16(unsigned char start, unsigned char count)
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = start; i < (start+count); i++)
  {
    temp = temp ^ frame[i];
    for (unsigned char j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }
  // Reverse byte order. 
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  temp &= 0xFFFF;
  return temp; // the returned value is already swopped - crcLo byte is first & crcHi byte is last
}

void exceptionResponse(unsigned char exception)
{
  modbus_error_count++; // each call to exceptionResponse() will increment the modbus_error_count
  if (!broadcastFlag) // don't respond if its a broadcast message
  {
    frame[0] = slaveID;
    frame[1] = (function | MODBUS_ERROR_MARKER); // set the MSB bit high, informs the master of an exception
    frame[2] = exception;
    unsigned int crc16 = calculate_CRC16(0, 3); // ID, function + 0x80, exception code == 3 bytes
    frame[3] = crc16 >> 8;
    frame[4] = crc16 & 0xFF;
    // exception response is always 5 bytes ID, function + 0x80, exception code, 2 bytes crc
    modbus_send_packet(0, 5);
  }
  buffer = 0;

  #ifdef MODBUS_ERROR_CRC_ENABLED
  if (exception == MODBUS_ERROR_CRC)
    modbus_crc_errors++;
  #endif
}

void modbus_send_packet(unsigned char start, unsigned char count)
{
  
  for (unsigned char i = start; i < start+count; i++)
    (*modbus_SerialWrite)(frame[i], modbus_serial_port);
 
  // allow a frame delay to indicate end of transmission
  _delay_ms(3);
  

}

bool testAddress(unsigned int address) {
	if ((address>=M90E26_START_ADDRESS) && (address<=M90E26_END_ADDRESS)) {
		return true;
	} 
	else if ((address >= RELAY_REG_START_ADDRESS) && (address <= RELAY_REG_END_ADDRESS) ) {
		return true;
	}
	else if ((address>=DS18B20_START_ADDRESS) && (address<=DS18B20_END_ADDRESS)) {
		return true;
	}
	else if ((address>=ERRORS_START_ADDRESS) && (address<=ERRORS_END_ADDRESS)) {
		return true;
	}
	else if ((address>=DEBUG_REGS_START_ADDRESS) && (address<=DEBUG_REGS_END_ADDRESS)) {
		return true;
	}
	else 
		return false;
}

