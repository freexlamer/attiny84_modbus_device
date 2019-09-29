
#include "tiny_modbus_rtu_slave.h"

unsigned char frame[BUFFER_SIZE+2];
unsigned char errorCount;
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

unsigned char pull_port(unsigned char c){

	if (!overflow) {
		if (buffer == BUFFER_SIZE) {
			overflow = true;
		}
		frame[buffer] = c;
		buffer++;
	}

	if (overflow) {
		buffer = 0;
		overflow = false;
    	return errorCount++;
	}

	// The minimum request packet is 8 bytes for function 3 & 16
	/*
	Вообще, надо сделать проверку. Если сейчас буфер меньше, чем должны получить данных -
	то пропускаем и принимаем дальше.
	*/
  	if (buffer > 7) {
  		unsigned char id = frame[0];
  		broadcastFlag = (id == MODBUS_BROADCAST_ID);
  		

  		if (id == slaveID || broadcastFlag) {
  			
  			// combine the crc Low & High bytes
  			unsigned int crc = ((frame[buffer - 2] << 8) | frame[buffer - 1]);
  			

  			if (calculateCRC(buffer - 2) == crc) {
  				(*modbus_led)(true);
  				function = frame[1];
  				unsigned int startingAddress = ((frame[2] << 8) | frame[3]);
  				unsigned int no_of_registers = ((frame[4] << 8) | frame[5]);
  				unsigned int crc16;

  				if (!broadcastFlag && (function == MODBUS_FUNCTION_READ_AO)) {
  					if (testAddress(startingAddress)) {
  						if (no_of_registers == 1) {
  							unsigned char noOfBytes = no_of_registers * 2;
  							unsigned char responseFrameSize = 5 + noOfBytes;
  							frame[0] = slaveID;
              				frame[1] = function;
              				frame[2] = noOfBytes;
  							unsigned int temp = (*modbus_read_reg)(startingAddress);
  							frame[3] = temp >> 8;
  							frame[4] = temp & 0x00FF;
  							crc16 = calculateCRC(responseFrameSize - 2);
  							frame[responseFrameSize - 2] = crc16 >> 8;
              				frame[responseFrameSize - 1] = crc16 & 0xFF;
  							sendPacket(responseFrameSize);
  							buffer = 0;
  						}
  						else
  							exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
  					}
  					else 
  						exceptionResponse(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);

  				}
  				else if (function == MODBUS_FUNCTION_WRITE_AO) {
  					if (testAddress(startingAddress)) {
  						unsigned int startingAddress = ((frame[2] << 8) | frame[3]);
              			unsigned int regStatus = ((frame[4] << 8) | frame[5]);

              			if ((*modbus_write_reg)(startingAddress,regStatus)){
              				sendPacket(MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE);
              				buffer = 0;
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
  				sendPacket(buffer);
  				exceptionResponse(MODBUS_ERROR_CRC);
  			}
  		}


  	} else {
  		return 0;
  	}

  	return 0;

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
  errorCount++; // each call to exceptionResponse() will increment the errorCount
  if (!broadcastFlag) // don't respond if its a broadcast message
  {
    frame[0] = slaveID;
    frame[1] = (function | MODBUS_ERROR_MARKER); // set the MSB bit high, informs the master of an exception
    frame[2] = exception;
    unsigned int crc16 = calculateCRC(3); // ID, function + 0x80, exception code == 3 bytes
    frame[3] = crc16 >> 8;
    frame[4] = crc16 & 0xFF;
    // exception response is always 5 bytes ID, function + 0x80, exception code, 2 bytes crc
    sendPacket(5); 
  }
  buffer = 0;
}

void sendPacket(unsigned char bufferSize)
{
  
  for (unsigned char i = 0; i < bufferSize; i++)
    (*modbus_uart_putc)(frame[i]);
 
  // allow a frame delay to indicate end of transmission
  _delay_ms(3);
  

}

bool testAddress(unsigned int address) {
	if ((address>=M90E26_START_ADDRESS) && (address<=M90E26_END_ADDRESS)) {
		return true;
	} 
	else if (address == RELAY_REG_ADDRESS) {
		return true;
	}
	else if ((address>=DS18B20_START_ADDRESS) && (address<=DS18B20_END_ADDRESS)) {
		return true;
	}
	else if ((address>=ERRORS_START_ADDRESS) && (address<=ERRORS_END_ADDRESS)) {
		return true;
	}
	else 
		return false;
}

