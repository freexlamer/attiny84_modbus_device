/*
Based on ideas of https://github.com/angeloc/simplemodbusng

The crc calculation is based on the work published 
 by jpmzometa at 
 http://sites.google.com/site/jpmzometa/arduino-mbrt
 
 By Juan Bester : bester.juan@gmail.com
*/

// (C) freexlamer@github.com


#include "tiny_modbus_rtu_slave.h"

unsigned char frame[BUFFER_SIZE+2];
unsigned char out_frame[OUT_BUFFER_SIZE];
unsigned char slaveID;
unsigned char function;
bool broadcastFlag;

unsigned char buffer = 0;
bool overflow = false;

// function definitions
void exceptionResponse(unsigned char exception);
unsigned int calculateCRC(unsigned char bufferSize);
void sendPacket(unsigned char bufferSize);
bool testAddress(unsigned int address);

void modbus_init() {
	modbus_error_count = 0;
	modbus_crc_errors = 0;
}

void cut_frame(unsigned char count) {
  unsigned char i;
  for (i=count; i<buffer; i++) {
    frame[i-count] = frame[i];
  }
  buffer = buffer - count;
}

int find_command() {
  unsigned char command_start = 0;
  unsigned char cut_until;
  bool broadcast_frame;
  bool one_command_find = false;

  for (command_start = 0; command_start < buffer; command_start ++) {
    broadcast_frame = (frame[command_start] == MODBUS_BROADCAST_ID);

    if (broadcast_frame || (frame[command_start] == slaveID)) {
      unsigned char function_code_index = command_start + 1;
      if (function_code_index < buffer) {
        one_command_find = true;
        continue;
      } else {
        if ((frame[function_code_index]==MODBUS_FUNCTION_READ_AO) ||
            (frame[function_code_index]==MODBUS_FUNCTION_WRITE_AO) ||
            (frame[function_code_index]==MODBUS_FUNCTION_READ_DO) ||
            (frame[function_code_index]==MODBUS_FUNCTION_READ_DI) ||
            (frame[function_code_index]==MODBUS_FUNCTION_READ_AI) ||
            (frame[function_code_index]==MODBUS_FUNCTION_WRITE_DO)) {
          if (function_code_index+MOBUS_4_BYTES_PDU_SIZE+MODBUS_CRC_SIZE) {
            //calculate_crc?
            return command_start;
          }
          else {
            one_command_find = true;
            continue;
          }
        }
        else if ((frame[function_code_index]==MODBUS_FUNCTION_WRITE_MANY_DO) ||
                 (frame[function_code_index]==MODBUS_FUNCTION_WRITE_MANY_AO)) {
          unsigned char data_bytes_count_index = function_code_index + MOBUS_4_BYTES_PDU_SIZE + 1;
          if (data_bytes_count_index < buffer) {
            one_command_find = true;
            continue;
          }
          else {
            unsigned char data_bytes_count = frame[data_bytes_count_index];
            if ((data_bytes_count_index + data_bytes_count + MODBUS_CRC_SIZE) < buffer) {
              one_command_find = true;
              continue;
            }
            else {
              return command_start;
            }
          }

        }
      }
    }
    else if (!one_command_find) {
      // delete this and shift frame
      cut_until = command_start;
    }
  }

  if (one_command_find) {
    return -2;
  }
  else {
    // cut buffer
    cut_frame(cut_until+1);
    return -1;
  }
}

bool decode_command(unsigned char start) {
  unsigned char id = frame[start];
  broadcastFlag = (id == MODBUS_BROADCAST_ID);

  if (id == slaveID || broadcastFlag) {
    function = frame[start+1];

    /*
    // костыль!
    if ((frame[start] != MODBUS_FUNCTION_READ_AO) && (frame[start] != MODBUS_FUNCTION_WRITE_AO)) {
      exceptionResponse(MODBUS_ERROR_ILLEGAL_FUNCTION);
      return modbus_error_count;
    }
    */

    // combine the crc Low & High bytes
    // костыль
    unsigned int crc = ((frame[start + MOBUS_4_BYTES_PDU_SIZE + 1] << 8) | frame[start + MOBUS_4_BYTES_PDU_SIZE + 2]);
    

    if (calculateCRC(buffer - 2) == crc) {
      function = frame[start+1];
      unsigned int startingAddress = ((frame[start+2] << 8) | frame[start+3]);
      unsigned int no_of_registers = ((frame[start+4] << 8) | frame[start+5]);
      unsigned int crc16;

      if (!broadcastFlag && (function == MODBUS_FUNCTION_READ_AO)) {
        if (testAddress(startingAddress)) {
          if (no_of_registers == 1) {
            unsigned int temp = 0;

            if ((*modbus_read_reg)(startingAddress, &temp)) {
              // prepare answer
              unsigned char noOfBytes = no_of_registers * 2;
              unsigned char responseFrameSize = 5 + noOfBytes;
              out_frame[0] = slaveID;
              out_frame[1] = function;
              out_frame[2] = noOfBytes;
              out_frame[3] = temp >> 8;
              out_frame[4] = temp & 0x00FF;
              crc16 = calculateCRC(responseFrameSize - 2);
              out_frame[responseFrameSize - 2] = crc16 >> 8;
              out_frame[responseFrameSize - 1] = crc16 & 0xFF;
              // sending answer
              sendPacket(responseFrameSize);

              // reset frame !
              //buffer = 0;
              cut_frame(start + MOBUS_4_BYTES_PDU_SIZE + MODBUS_CRC_SIZE + 1);
            }
            else 
              exceptionResponse(MODBUS_ERROR_SLAVE_DEVICE_FAILURE);
          }
          else
            exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
        }
        else 
          exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
      }
      else if (function == MODBUS_FUNCTION_WRITE_AO) {
        if (testAddress(startingAddress)) {
          unsigned int startingAddress = ((frame[start + 2] << 8) | frame[start + 3]);
                unsigned int regStatus = ((frame[start + 4] << 8) | frame[start + 5]);

                if ((*modbus_write_reg)(startingAddress,regStatus)){
                  for (int i = start; i < start + MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE; i++ ) {
                    out_frame[i-start] = frame[i];
                  }
                  sendPacket(MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE);
                  //buffer = 0;
                  cut_frame(start + MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE + 1);
                }
                else 
                  exceptionResponse(MODBUS_ERROR_SLAVE_DEVICE_FAILURE);
        }
        else
          exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
      }
      else
            exceptionResponse(MODBUS_ERROR_ILLEGAL_FUNCTION);

    }
    else 
    { 
    //sendPacket(buffer);
      exceptionResponse(MODBUS_ERROR_CRC);
    }
  }


}

unsigned char pull_port(int c){
  int command_start;
	if (c == -1) {
		return 0;
	}

	if (!overflow) {
		if (buffer == BUFFER_SIZE) {
			overflow = true;
		}
		frame[buffer] = (unsigned char)c;
		buffer++;
	}

	if (overflow) {
		buffer = 0;
		overflow = false;
		return modbus_error_count++;
	}

	// The minimum request packet is 8 bytes for function 3 & 16
	/*
	Вообще, надо сделать проверку. Если сейчас буфер меньше, чем должны получить данных -
	то пропускаем и принимаем дальше.
	*/
    command_start = find_command();
    if (command_start >= 0) {
      decode_command(command_start);
    }
    else {
      return 0;
    }

    /*
  	if (buffer > 7) {
      decode_command(0);
  	} else {
  		return 0;
  	}

  	return 0;
    */

}


unsigned int calculateCRC(unsigned char bufferSize) 
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < bufferSize; i++)
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
    out_frame[0] = slaveID;
    out_frame[1] = (function | MODBUS_ERROR_MARKER); // set the MSB bit high, informs the master of an exception
    out_frame[2] = exception;
    unsigned int crc16 = calculateCRC(3); // ID, function + 0x80, exception code == 3 bytes
    out_frame[3] = crc16 >> 8;
    out_frame[4] = crc16 & 0xFF;
    // exception response is always 5 bytes ID, function + 0x80, exception code, 2 bytes crc
    sendPacket(5); 
  }
  buffer = 0;

  if (exception == MODBUS_ERROR_CRC)
    modbus_crc_errors++;
}

void sendPacket(unsigned char bufferSize)
{
  
  for (unsigned char i = 0; i < bufferSize; i++)
    (*modbus_SerialWrite)(out_frame[i], modbus_serial_port);
 
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

