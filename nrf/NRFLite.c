#include "NRFLite.h"
#include"../lcd/lcd_i2c.h"
#include"../lcd/main_timer.h"
#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include<util/delay.h>
uint8_t radio_init(uint8_t radioId, uint8_t cePin, uint8_t csnPin, Bitrates bitrate, uint8_t channel, uint8_t callSpiBegin)
{
    lcd_print("began init");
    NRF_spi_settings.prescaler_divider = 4;
    NRF_spi_settings.MSB_first = 1;
    NRF_spi_settings.mode = 0;
    _cePin = cePin;
    _csnPin = csnPin;
    _useTwoPinSpiTransfer = 0;

    // Default states for the radio pins.  When CSN is LOW the radio listens to SPI communication,
    // so we operate most of the time with CSN HIGH.
    // currently connecting CSN AND CE TO PORT D .
    DDRD |= (1 << _cePin) | (1 << _csnPin);
    PORTD |= (1 << _csnPin);
        if (callSpiBegin)
        {
            // Arduino SPI makes SS (D10 on ATmega328) an output and sets it HIGH.  It must remain an output
            // for Master SPI operation, but in case it started as LOW, we'll set it back.
            uint8_t savedSS = PORTB & (1 << SS);
            spibegin();
            if (_csnPin != SS) {
                if(savedSS){
                    PORTB |= (1 << SS);
                }
                else{
                    PORTB &= (~(1 << SS));
                }
            }
        }
    lcd_clear();
    lcd_print("done pins!");
    // With the microcontroller's pins setup, we can now initialize the radio.
    uint8_t success = initRadio(radioId, bitrate, channel);
    return success;
}
uint8_t initRadio(uint8_t radioId, Bitrates bitrate, uint8_t channel)
{
    lcd_clear();
    lcd_print("radio work");
    _lastToRadioId = -1;
    _resetInterruptFlags = 1;
    _usingSeparateCeAndCsnPins = _cePin != _csnPin;

    _delay_ms(OFF_TO_POWERDOWN_MILLIS);

    // Valid channel range is 2400 - 2525 MHz, in 1 MHz increments.
    if (channel > MAX_NRF_CHANNEL) { channel = MAX_NRF_CHANNEL; }
    writeRegisterName(RF_CH, channel);

    // Transmission speed, retry times, and output power setup.
    // For 2 Mbps or 1 Mbps operation, a 500 uS retry time is necessary to support the max ACK packet size.
    // For 250 Kbps operation, a 1500 uS retry time is necessary.
    if (bitrate == BITRATE2MBPS)
    {
        writeRegisterName(RF_SETUP, 0b00001110);   // 2 Mbps, 0 dBm output power
        writeRegisterName(SETUP_RETR, 0b00011111); // 0001 =  500 uS between retries, 1111 = 15 retries
        _transmissionRetryWaitMicros = 600;   // 100 more than the retry delay
        _maxHasDataIntervalMicros = 1200;
    }
    else if (bitrate == BITRATE1MBPS)
    {
        writeRegisterName(RF_SETUP, 0b00000110);   // 1 Mbps, 0 dBm output power
        writeRegisterName(SETUP_RETR, 0b00011111); // 0001 =  500 uS between retries, 1111 = 15 retries
        _transmissionRetryWaitMicros = 600;   // 100 more than the retry delay
        _maxHasDataIntervalMicros = 1700;
    }
    else
    {
        writeRegisterName(RF_SETUP, 0b00100110);   // 250 Kbps, 0 dBm output power
        writeRegisterName(SETUP_RETR, 0b01011111); // 0101 = 1500 uS between retries, 1111 = 15 retries
        _transmissionRetryWaitMicros = 1600;  // 100 more than the retry delay
        _maxHasDataIntervalMicros = 5000;
    }

    // Assign this radio's address to RX pipe 1.  When another radio sends us data, this is the address
    // it will use.  We use RX pipe 1 to store our address since the address in RX pipe 0 is reserved
    // for use with auto-acknowledgment packets.
    uint8_t address[5] = { ADDRESS_PREFIX[0], ADDRESS_PREFIX[1], ADDRESS_PREFIX[2], ADDRESS_PREFIX[3], radioId };
    writeRegister(RX_ADDR_P1, &address, 5);

    // Enable dynamically sized packets on the 2 RX pipes we use, 0 and 1.
    // RX pipe address 1 is used to for normal packets from radios that send us data.
    // RX pipe address 0 is used to for auto-acknowledgment packets from radios we transmit to.
    writeRegisterName(DYNPD, (1 << DPL_P0) | (1 << DPL_P1));

    // Enable dynamically sized payloads, ACK payloads, and TX support with or without an ACK request.
    writeRegisterName(FEATURE, (1 << EN_DPL) | (1 << EN_ACK_PAY) | (1 << EN_DYN_ACK));

    // Ensure RX and TX buffers are empty.  Each buffer can hold 3 packets.
    spiTransfer(WRITE_OPERATION, FLUSH_RX, 0, 0);
    spiTransfer(WRITE_OPERATION, FLUSH_TX, 0, 0);

    // Clear any interrupts.
    writeRegisterName(STATUS_NRF, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    uint8_t success = startRx();
    lcd_clear();
    lcd_print(itoa(success));
    return success;
}
uint8_t startRx()
{
    waitForTxToComplete();

    // Put radio into Standby-I mode in order to transition into RX mode.
    PORTD &= (~(1 << _cePin));
    // Configure the radio for receiving.
    writeRegisterName(CONFIG, CONFIG_REG_SETTINGS_FOR_RX_MODE);

    // Put radio into RX mode.
    
    PORTD |= (1 << _cePin);
    // Wait for the transition into RX mode.
    _delay_ms(POWERDOWN_TO_RXTX_MODE_MILLIS);

    uint8_t inRxMode = readRegisterName(CONFIG) == CONFIG_REG_SETTINGS_FOR_RX_MODE;
    return inRxMode;
}
uint8_t waitForTxToComplete()
{
    _resetInterruptFlags = 0; // Disable interrupt flag reset logic in 'whatHappened'.

    uint8_t fifoReg, statusReg;
    uint8_t txBufferIsEmpty;
    uint8_t packetWasSent, packetCouldNotBeSent;
    uint8_t txAttemptCount = 0;
    uint8_t result = 0; // Default to indicating a failure.

    // TX buffer can store 3 packets, sends retry up to 15 times, and the retry wait time is about half
    // the time necessary to send a 32 byte packet and receive a 32 byte ACK response.  3 x 15 x 2 = 90
    const static uint8_t MAX_TX_ATTEMPT_COUNT = 90;

    while (txAttemptCount++ < MAX_TX_ATTEMPT_COUNT)
    {
        fifoReg = readRegisterName(FIFO_STATUS);
        txBufferIsEmpty = fifoReg & 1 << (TX_EMPTY);

        if (txBufferIsEmpty)
        {
            result = 1; // Indicate success.
            break;
        }

        // If we have separate pins for CE and CSN, CE will be LOW so we must toggle it to send a packet.
        if (_usingSeparateCeAndCsnPins)
        {
            PORTD |= (1 << _cePin);
            _delay_us(CE_TRANSMISSION_MICROS);
            PORTD &= (~(1 << _cePin));
        }

        _delay_us(600);
        
        statusReg = readRegisterName(STATUS_NRF);
        packetWasSent = statusReg & (1 << TX_DS);
        packetCouldNotBeSent = statusReg & (1 << MAX_RT);

        if (packetWasSent)
        {
            writeRegisterName(STATUS_NRF, (1 << TX_DS));   // Clear TX success flag.
        }
        else if (packetCouldNotBeSent)
        {
            spiTransfer(WRITE_OPERATION, FLUSH_TX, 0, 0); // Clear TX buffer.
            writeRegisterName(STATUS_NRF, (1 << MAX_RT));          // Clear max retry flag.
            break;
        }
    }

    _resetInterruptFlags = 1; // Re-enable interrupt reset logic in 'whatHappened'.

    return result;
}
void readData (void * data){
    // Determine length of data in the RX buffer and read it.
    uint8_t dataLength;
    spiTransfer(READ_OPERATION, R_RX_PL_WID, &dataLength, 1);
    spiTransfer(READ_OPERATION, R_RX_PAYLOAD, data, dataLength);
    
    // Clear data received flag.
    uint8_t statusReg = readRegisterName(STATUS_NRF);
    if (statusReg & 1 << (RX_DR))
    {
        writeRegisterName(STATUS_NRF, statusReg | _BV(RX_DR));
    }
}
void powerDown(){
    // IF we have separ
    if(_usingSeparateCeAndCsnPins){
        DDRD &= (~(1 << _cePin));
    }
    // Turn off the radio.
    writeRegisterName(CONFIG,readRegisterName(CONFIG) & (~(1 << PWR_UP)));
}
void printDetails(){
    printRegister("CONFIG", readRegisterName(CONFIG));
    printRegister("EN_AA", readRegisterName(EN_AA));
    printRegister("EN_RXADDR", readRegisterName(EN_RXADDR));
    printRegister("SETUP_AW", readRegisterName(SETUP_AW));
    printRegister("SETUP_RETR", readRegisterName(SETUP_RETR));
    printRegister("RF_CH", readRegisterName(RF_CH));
    printRegister("RF_SETUP", readRegisterName(RF_SETUP));
    printRegister("STATUS", readRegisterName(STATUS_NRF));
    printRegister("OBSERVE_TX", readRegisterName(OBSERVE_TX));
    printRegister("RX_PW_P0", readRegisterName(RX_PW_P0));
    printRegister("RX_PW_P1", readRegisterName(RX_PW_P1));
    printRegister("FIFO_STATUS", readRegisterName(FIFO_STATUS));
    printRegister("DYNPD", readRegisterName(DYNPD));
    printRegister("FEATURE", readRegisterName(FEATURE));
    
    uint8_t data[5];
    char msg[] = "TX_ADDR ";
    readRegister(TX_ADDR, &data, 5);
    
}
void printRegister(const char name[], uint8_t regName){
    //todo
}
uint8_t scanChannel(uint8_t channel, uint8_t measurementCount){
    uint8_t strength = 0;
    //Put radio into Standby-I mode.
    PORTD &= (~(1 << _cePin));
    writeRegisterName(RF_CH, channel);
    do{
        PORTD |= (1 << _cePin);
        _delay_us(400);
        PORTD &= (~(1 << _cePin));
        uint8_t signalWasReceived = readRegisterName(CD);
        if (signalWasReceived){
            strength ++;
        }
    
    } while(measurementCount--);    
    return strength;
}
uint8_t readRegisterName(uint8_t regName){
    uint8_t data;
    readRegister(regName, &data, 1);
    return data;
}
void readRegister(uint8_t regName, void* data, uint8_t length){
    
    spiTransfer(READ_OPERATION, (R_REGISTER | (REGISTER_MASK & regName)), data, length);
}
void writeRegisterName(uint8_t regName, uint8_t data){
     writeRegister(regName, &data, 1);
}
void writeRegister(uint8_t regName, void* data, uint8_t length){
    spiTransfer(WRITE_OPERATION, (W_REGISTER | (REGISTER_MASK & regName)), data, length);
}
void spiTransfer(SpiTransferType transferType, uint8_t regName, void* data, uint8_t length){
    uint8_t* intData = data;

    cli(); // Prevent an interrupt from interferring with the communication.
    {
        PORTD &= (~(1 << _csnPin));
        // Signal radio to listen to the SPI bus.
        // Transfer with the Arduino SPI library.
            spi_master_init(NRF_spi_settings);
            spi_transfer(regName);
            for (uint8_t i = 0; i < length; ++i) {
                uint8_t newData = spi_transfer(intData[i]);
                if (transferType == READ_OPERATION) { intData[i] = newData; }
            }
        
        DDRD |= (1 << _csnPin); // Stop radio from listening to the SPI bus.
    }

    sei();

}
