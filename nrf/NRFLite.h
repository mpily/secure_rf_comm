#ifndef NRFLITE
#define NRFLITE
#include "nRF24L01.h"
#include "spilib.h"
const static uint8_t MAX_NRF_CHANNEL = 125;//MAX CHANNEL NUMBER
// Methods for both receivers and transmitters.
    // init       = Turns the radio on and puts it into receiving mode.  Returns 0 if it cannot communicate with the radio.
    //              Channel can be 0-125 and sets the exact frequency of the radio between 2400 - 2525 MHz.
    // initTwoPin = Same as init but with multiplexed MOSI/MISO and CE/CSN/SCK pins (only works on AVR architectures).
    //              Follow the 2-pin hookup schematic on https://github.com/dparson55/NRFLite (not implemented)
    // readData   = Loads a received data packet or acknowledgment data packet into the specified data parameter.
    // powerDown  = Power down the radio.  Turn the radio back on by calling one of the 'hasData' or 'send' methods.
    // printDetails = Prints many of the radio registers.  Requires a serial object in the constructor, e.g. NRFLite _radio(Serial);
    // scanChannel  = Returns 0-measurementCount to indicate the strength of any existing signal on a channel.  Radio communication will
    //                work best on channels with no existing signals, meaning a 0 is returned.
    SPI_Settings NRF_spi_settings;
    typedef enum  {BITRATE2MBPS, BITRATE1MBPS, BITRATE250KBPS}Bitrates;
    typedef enum  { REQUIRE_ACK, NO_ACK }SendType;
    uint8_t radio_init(uint8_t radioId, uint8_t cePin, uint8_t csnPin, Bitrates bitrate, uint8_t channel, uint8_t callSpiBegin);
    void readData(void *data);
    void powerDown();
    void printDetails();
    uint8_t scanChannel(uint8_t channel, uint8_t measurementCount);
    //default measurement count 255
     // Methods for transmitters.
    // send = Sends a data packet and waits for success or failure.  The default REQUIRE_ACK sendType causes the radio
    //        to attempt sending the packet up to 16 times.  If successful a 1 is returned.  Optionally the NO_ACK sendType
    //        can be used to transmit the packet a single time without any acknowledgement.
    // hasAckData = Checks to see if an ACK data packet was received and returns its length.
    uint8_t send(uint8_t toRadioId, void *data, uint8_t length, SendType sendType);
    //default sendType REQUIRE_ACK
    uint8_t hasAckData();

    // Methods for receivers.
    // hasData     = Checks to see if a data packet has been received and returns its length.  It also puts the radio in RX mode
    //               if it was previously in TX mode.
    // addAckData  = Enqueues an acknowledgment data packet for sending back to a transmitter.  Whenever the transmitter sends the 
    //               next data packet, it will get this ACK packet back in the response.  The radio will store up to 3 ACK packets
    //               and will not enqueue more if full, so you can clear any stale packets using the 'removeExistingAcks' parameter.
    // discardData = Removes the current received data packet.  Useful if a packet of an unexpected size is received.
    uint8_t hasData(uint8_t usingInterrupts);
    //default usingInterrupts 0
    void addAckData(void *data, uint8_t length, uint8_t removeExistingAcks);
    //default removeExistingAcks = 0
    void discardData(uint8_t unexpectedDataLength);
    
    // Methods when using the radio's IRQ pin for interrupts.
    // Note that if interrupts are used, do not use the send and hasData functions.  Instead the functions below should be used.
    // hasDataISR   = Same as hasData(1), it will greatly speed up the receive bitrate when CE and CSN share the same pin.
    // startRx      = Allows switching the radio into RX mode rather than calling 'hasData'.
    //                Returns 0 if it cannot communicate with the radio.
    // startSend    = Start sending a data packet without waiting for it to complete.
    // whatHappened = Use this inside the interrupt handler to see what caused the interrupt.
    uint8_t hasDataISR(); 
    uint8_t startRx();
    void startSend(uint8_t toRadioId, void *data, uint8_t length, SendType sendType);
    //default sendType REQUIRE_ACK 
    void whatHappened(uint8_t *txOk, uint8_t *txFail, uint8_t *rxReady);
    const static uint8_t ADDRESS_PREFIX[4] = { 1, 2, 3, 4 }; // 1st 4 bytes of addresses, 5th byte will be RadioId.
    const static uint8_t CONFIG_REG_SETTINGS_FOR_RX_MODE = 1 << (PWR_UP) | 1 << (PRIM_RX) | 1 << (EN_CRC);
    const static uint32_t NRF_SPICLOCK = 4000000; // Speed to use for SPI communication with the transceiver.
    // Delay used to discharge the radio's CSN pin when operating in 2-pin mode.
    // Determined by measuring time to discharge CSN on a 1MHz ATtiny using 0.1uF capacitor and 1K resistor.
    const static uint16_t CSN_DISCHARGE_MICROS = 500;

    const static uint8_t OFF_TO_POWERDOWN_MILLIS = 100;     // Vcc > 1.9V power on reset time.
    const static uint8_t POWERDOWN_TO_RXTX_MODE_MILLIS = 5; // 4500uS to Standby + 130uS to RX or TX mode, so 5ms is enough.
    const static uint8_t CE_TRANSMISSION_MICROS = 10;       // Time to initiate data transmission.

    typedef enum  { READ_OPERATION, WRITE_OPERATION }SpiTransferType;

    volatile uint8_t *_momi_PORT;
    volatile uint8_t *_momi_DDR;
    volatile uint8_t *_momi_PIN;
    volatile uint8_t *_sck_PORT;
    uint8_t _cePin, _csnPin, _momi_MASK, _sck_MASK;
    volatile uint8_t _resetInterruptFlags;
    uint8_t _useTwoPinSpiTransfer, _usingSeparateCeAndCsnPins;
    uint16_t _transmissionRetryWaitMicros, _maxHasDataIntervalMicros;
    int16_t _lastToRadioId = -1;
    uint64_t _microsSinceLastDataCheck;
    
    uint8_t getPipeOfFirstRxPacket();
    uint8_t getRxPacketLength();
    uint8_t initRadio(uint8_t radioId, Bitrates bitrate, uint8_t channel);
    void prepForTx(uint8_t toRadioId, SendType sendType);
    uint8_t waitForTxToComplete();
    uint8_t readRegisterName(uint8_t regName);
    void readRegister(uint8_t regName, void* data, uint8_t length);
    void writeRegisterName(uint8_t regName, uint8_t data);
    void writeRegister(uint8_t regName, void* data, uint8_t length);
    void spiTransfer(SpiTransferType transferType, uint8_t regName, void* data, uint8_t length);

    void printRegister(const char name[], uint8_t regName);
    //implementations
    
    
#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include<util/delay.h>
#include<stdlib.h>
#include"../lcd/lcd_i2c.h"
#include"../lcd/main_timer.h"
uint8_t radio_init(uint8_t radioId, uint8_t cePin, uint8_t csnPin, Bitrates bitrate, uint8_t channel, uint8_t callSpiBegin)
{
    NRF_spi_settings.prescaler_divider = 4;
    NRF_spi_settings.MSB_first = 1;
    NRF_spi_settings.mode = 0x00;
    _cePin = cePin;
    _csnPin = csnPin;
    _useTwoPinSpiTransfer = 0;

    // Default states for the radio pins.  When CSN is LOW the radio listens to SPI communication,
    // so we operate most of the time with CSN HIGH.
    // currently connecting CSN AND CE TO PORT D .
    DDRB |= (1 << _cePin) ;
    DDRB |= (1 << _csnPin);
    PORTB |= (1 << _csnPin);
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
    // With the microcontroller's pins setup, we can now initialize the radio.
    uint8_t success = initRadio(radioId, bitrate, channel);
    return success;
}
uint8_t initRadio(uint8_t radioId, Bitrates bitrate, uint8_t channel)
{
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
    //debug
    //lcd_clear();
    //printDetails();
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
    spiTransfer(WRITE_OPERATION, FLUSH_RX, NULL, 0);
    spiTransfer(WRITE_OPERATION, FLUSH_TX, NULL, 0);

    // Clear any interrupts.
    writeRegisterName(STATUS_NRF, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    //debug;
    uint8_t success = startRx();
    return success;
}
uint8_t startRx()
{
    waitForTxToComplete();

    // Put radio into Standby-I mode in order to transition into RX mode.
    PORTB &= (~(1 << _cePin));
    // Configure the radio for receiving.
    writeRegisterName(CONFIG, CONFIG_REG_SETTINGS_FOR_RX_MODE);

    // Put radio into RX mode.
    
    PORTB |= (1 << _cePin);
    // Wait for the transition into RX mode.
    _delay_ms(POWERDOWN_TO_RXTX_MODE_MILLIS);

    uint8_t inRxMode = (readRegisterName(CONFIG) == CONFIG_REG_SETTINGS_FOR_RX_MODE);
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
            PORTB |= (1 << _cePin);
            _delay_us(CE_TRANSMISSION_MICROS);
            PORTB &= (~(1 << _cePin));
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
        writeRegisterName(STATUS_NRF, statusReg | (1 << RX_DR));
    }
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
    char buffer[20];
    itoa(regName,buffer,16);
    lcd_print(name);
    _delay_ms(500);
    lcd_clear();
    lcd_print(buffer);
    _delay_ms(500);
    lcd_clear();
}
void assert(uint8_t fact){
	if(fact){
		return;
	}
	else{
		lcd_clear();
		lcd_print("not_true");
		while(1);
	}
}
uint8_t scanChannel(uint8_t channel, uint8_t measurementCount){
    uint8_t notInRxMode = readRegisterName(CONFIG) != CONFIG_REG_SETTINGS_FOR_RX_MODE;
    if(notInRxMode){
        startRx();
    }
    uint8_t strength = 0;
    uint8_t seen_something = 0;
    //Put radio into Standby-I mode.
    PORTB &= (~(1 << _cePin));
    writeRegisterName(RF_CH, channel);
    //assert(readRegisterName(RF_CH) == channel);
    do{
        PORTB |= (1 << _cePin);
        _delay_us(400);
        PORTB &= (~(1 << _cePin));
        uint8_t signalWasReceived = readRegisterName(CD);
        if (signalWasReceived){
            seen_something = 1;
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
    uint8_t* intData = (uint8_t *) data;

    cli(); // Prevent an interrupt from interferring with the communication.
    
    // Signal radio to listen to the SPI bus.
    // Transfer with the Arduino SPI library.
    //spi_master_init(NRF_spi_settings);
    PORTB &= (~(1 << _csnPin));
    spi_master_init(NRF_spi_settings);
    spi_transfer(regName);
    for (uint8_t i = 0; i < length; ++i) {
        uint8_t newData = spi_transfer(intData[i]);
        if (transferType == READ_OPERATION) { intData[i] = newData; }
    }
    spi_un_init();//Stop using SPI bus
    PORTB |= (1 << _csnPin); // Stop radio from listening to the SPI bus.
    
/*spi_master_init(MFRC522_spi_settings);	// Set the settings to work with SPI bus
	MFRC522_ENABLE_CS();		// Select slave
	spi_transceiver(reg);						// MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
	spi_transceiver(value);
	MFRC522_DISABLE_CS();		// Release slave again
	spi_un_init(); // Stop using the SPI bus*/
    sei();

}
void prepForTx(uint8_t toRadioId, SendType sendType){
    if (_lastToRadioId != toRadioId)
    {
        _lastToRadioId = toRadioId;

        // TX pipe address sets the destination radio for the data.
        // RX pipe 0 is special and needs the same address in order to receive ACK packets from the destination radio.
        uint8_t address[5] = { ADDRESS_PREFIX[0], ADDRESS_PREFIX[1], ADDRESS_PREFIX[2], ADDRESS_PREFIX[3], toRadioId };
        writeRegister(TX_ADDR, &address, 5);
        writeRegister(RX_ADDR_P0, &address, 5);
    }

    // Ensure radio is ready for TX operation.
    uint8_t configReg = readRegisterName(CONFIG);
    uint8_t readyForTx = configReg == (CONFIG_REG_SETTINGS_FOR_RX_MODE & ~(1 << PRIM_RX));
    if (!readyForTx)
    {
        // Put radio into Standby-I mode in order to transition into TX mode.
        PORTB &= (~(1 << _cePin));
        configReg = CONFIG_REG_SETTINGS_FOR_RX_MODE & ~(1 << PRIM_RX);
        writeRegisterName(CONFIG, configReg);
        _delay_ms(POWERDOWN_TO_RXTX_MODE_MILLIS);
    }

    uint8_t fifoReg = readRegisterName(FIFO_STATUS);

    // If RX buffer is full and we require an ACK, clear it so we can receive the ACK response.
    uint8_t rxBufferIsFull = fifoReg & (1 << RX_FULL);
    if (sendType == REQUIRE_ACK && rxBufferIsFull)
    {
        spiTransfer(WRITE_OPERATION, FLUSH_RX, NULL, 0);
    }
    
    // If TX buffer is full, wait for all queued packets to be sent.
    uint8_t txBufferIsFull = fifoReg & (1 << FIFO_FULL);
    if (txBufferIsFull)
    {
        waitForTxToComplete();
    }
}
 uint8_t getPipeOfFirstRxPacket(){
    // The pipe number is bits 3, 2, and 1.  So B1110 masks them and we shift right by 1 to get the pipe number.
    // 000-101 = Data Pipe Number
    //     110 = Not Used
    //     111 = RX FIFO Empty
    return (readRegisterName(STATUS_NRF) & 0b1110) >> 1;
 }
uint8_t getRxPacketLength(){
    // Read the length of the first data packet sitting in the RX buffer.
    uint8_t dataLength;
    spiTransfer(READ_OPERATION, R_RX_PL_WID, &dataLength, 1);

    // Verify the data length is valid (0 - 32 bytes).
    if (dataLength > 32)
    {
        spiTransfer(WRITE_OPERATION, FLUSH_RX, NULL, 0); // Clear invalid data in the RX buffer.
        writeRegisterName(STATUS_NRF, readRegisterName(STATUS_NRF) | (1 << TX_DS) | (1 << MAX_RT) | (1 << RX_DR));
        return 0;
    }
    else
    {
        return dataLength;
    }
}
void addAckData(void *data, uint8_t length, uint8_t removeExistingAcks){
    if (removeExistingAcks)
    {
        spiTransfer(WRITE_OPERATION, FLUSH_TX, NULL, 0); // Clear the TX buffer.
    }
    
    // Add the packet to the TX buffer for pipe 1, the pipe used to receive packets from radios that
    // send us data.  When we receive the next transmission from a radio, we'll provide this ACK data in the
    // auto-acknowledgment packet that goes back.
    spiTransfer(WRITE_OPERATION, (W_ACK_PAYLOAD | 1), data, length);
}
void discardData(uint8_t unexpectedDataLength){
    // Read data from the RX buffer.
    uint8_t data[unexpectedDataLength];
    spiTransfer(READ_OPERATION, R_RX_PAYLOAD, &data, unexpectedDataLength);

    // Clear data received flag.
    uint8_t statusReg = readRegisterName(STATUS_NRF);
    if (statusReg & (1 << RX_DR))
    {
        writeRegisterName(STATUS_NRF, statusReg | (1 << RX_DR));
    }
}
uint8_t hasAckData(){
    // If we have a pipe 0 packet sitting at the top of the RX buffer, we have auto-acknowledgment data.
    // We receive ACK data from other radios using the pipe 0 address.
    if (getPipeOfFirstRxPacket() == 0)
    {
        return getRxPacketLength(); // Return the length of the data packet in the RX buffer.
    }
    else
    {
        return 0;
    }
}
uint8_t hasData(uint8_t usingInterrupts){
    // If using the same pins for CE and CSN, we need to ensure CE is left HIGH long enough to receive data.
    // If we don't limit the calling program, CE could be LOW so often that no data packets can be received.
    if (!_usingSeparateCeAndCsnPins)
    {
        if (usingInterrupts)
        {
            // Since the calling program is using interrupts, we can trust that it only calls hasData when an
            // interrupt is received, meaning only when data is received.  So we don't need to limit it.
        }
        else
        {
            uint8_t giveRadioMoreTimeToReceive = main_timer_now() - _microsSinceLastDataCheck < _maxHasDataIntervalMicros;
            if (giveRadioMoreTimeToReceive)
            {
                return 0; // Prevent the calling program from forcing us to bring CE low, making the radio stop receiving.
            }
            else
            {
                _microsSinceLastDataCheck = main_timer_now();
            }
        }
    }
    uint8_t notInRxMode = readRegisterName(CONFIG) != CONFIG_REG_SETTINGS_FOR_RX_MODE;
    if (notInRxMode)
    {
        startRx();
    }

    // If we have a pipe 1 packet sitting at the top of the RX buffer, we have data.
    if (getPipeOfFirstRxPacket() == 1)
    {
        return getRxPacketLength(); // Return the length of the data packet in the RX buffer.
    }
    else
    {
        return 0;
    }
}
uint8_t hasDataISR(){
    // This method should be used inside an interrupt handler for the radio's IRQ pin to bypass
    // the limit on how often the radio is checked for data.  This optimization greatly increases
    // the receiving bitrate when CE and CSN share the same pin.
    return hasData(1);
}


uint8_t send(uint8_t toRadioId, void *data, uint8_t length, SendType sendType){
    prepForTx(toRadioId, sendType);

    // Clear any previously asserted TX success or max retries flags.
    writeRegisterName(STATUS_NRF, (1 << TX_DS) | (1 << MAX_RT));
    
    // Add data to the TX buffer, with or without an ACK request.
    if (sendType == NO_ACK) { spiTransfer(WRITE_OPERATION, W_TX_PAYLOAD_NO_ACK, data, length); }
    else                    { spiTransfer(WRITE_OPERATION, W_TX_PAYLOAD       , data, length); }

    uint8_t result = waitForTxToComplete();
    return result;
}
void startSend(uint8_t toRadioId, void *data, uint8_t length, SendType sendType){
    prepForTx(toRadioId, sendType);
    
    // Add data to the TX buffer, with or without an ACK request.
    if (sendType == NO_ACK) { spiTransfer(WRITE_OPERATION, W_TX_PAYLOAD_NO_ACK, data, length); }
    else                    { spiTransfer(WRITE_OPERATION, W_TX_PAYLOAD       , data, length); }
    
    // Start transmission.
    // If we have separate pins for CE and CSN, CE will be LOW and we must pulse it to send the packet.
    if (_usingSeparateCeAndCsnPins)
    {
        PORTB |= (1 << _cePin);
        _delay_us(CE_TRANSMISSION_MICROS);
        PORTB &= ~(1 << _cePin);
    }
}
void whatHappened(uint8_t *txOk, uint8_t *txFail, uint8_t *rxReady){
    uint8_t statusReg = readRegisterName(STATUS_NRF);
    
    *txOk = statusReg & (1 << TX_DS);
    *txFail = statusReg & (1 << MAX_RT);
    *rxReady = statusReg & (1 << RX_DR);
    
    // When we need to see interrupt flags, we disable the logic here which clears them.
    // Programs that have an interrupt handler for the radio's IRQ pin will use 'whatHappened'
    // and if we don't disable this logic, it's not possible for us to check these flags.
    if (_resetInterruptFlags)
    {
        writeRegisterName(STATUS_NRF, (1 << TX_DS) | (1 << MAX_RT) | (1 << RX_DR));
    }
}
void powerDown(){
    // If we have separate CE and CSN pins, we can gracefully transition into Power Down mode by first entering Standby-I mode.
    if (_usingSeparateCeAndCsnPins)
    {
        PORTB &= ~(1 << _cePin);
    }
    
    // Turn off the radio.
    writeRegisterName(CONFIG, readRegisterName(CONFIG) & ~(1 << PWR_UP));
}


#endif
