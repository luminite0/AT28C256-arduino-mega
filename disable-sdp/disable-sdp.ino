// A modified version of https://github.com/TomNisbet/TommyPROM/blob/master/unlock-ben-eater-hardware/unlock-ben-eater-hardware.ino 
//
// Utility to unlock 28C256 Software Data Protection (SDP) for the Ben Eater EEPROM
// programmer design.  This hardware is similar to the TommyPROM hardware, but it uses
// different shift register chips and different pin assignments.
//
// To meet the timing requirements of the SDP unlock, this code uses direct port writes
// to set and read values on the data bus.  It will work Arduino Uno and Nano hardware,
// but would require changes for other platforms.


#define WRITE_EN 2
#define OUTPUT_EN 3

int address_bus_pins[] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52 };
int data_bus_pins[] = { 31, 33, 35, 37, 39, 41, 43, 45 };

// 32 byte test pattern to verify the EEPROM device.  The pattern includes a walking one
// and a walking zero, which may help to detect pins that are tied together or swapped.
byte test_data[] = {
    'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe,
    0x00, 0xff, 0x55, 0xaa, '0',  '1',  '2',  '3'
};


void enableWrite() {
    digitalWrite(WRITE_EN, LOW);
}

void disableWrite() {
    digitalWrite(WRITE_EN, HIGH);
}


byte readDataBus();
void writeDataBus(byte data);
void setByte(byte value, word address);


void setup() {
    pinMode(7, OUTPUT);
    digitalWrite(7, LOW);
    disableWrite();
    pinMode(WRITE_EN, OUTPUT);
    Serial.begin(57600);

    digitalWrite(OUTPUT_EN, HIGH);
    pinMode(OUTPUT_EN, HIGH);

    Serial.print("\nDisabling EEPROM Software Data Protection(SDP)...");
    disableSoftwareWriteProtect();
    Serial.println(" done\n");

    // Program a test pattern and fill the remainder of the first block with 0xff
    Serial.print("Writing test pattern to EEPROM... ");

    for (int i = 0; i < 10; i++) {
        writeEEPROM(i, test_data[i]);
        // writeEEPROM(i, 0xae);
    }
    Serial.println("Done writing");
    // Read and print out the contents of the EERPROM
    Serial.println("Reading EEPROM...");
    printContents();
 
    Serial.println();
    Serial.println("Done reading");
}


void loop() {
}


void setAddress(int addr, bool outputEnable) {
    if (outputEnable) {
        digitalWrite(OUTPUT_EN, LOW);
    } else {
        digitalWrite(OUTPUT_EN, HIGH);
    }
    for (int i = 0; i < 15; i++)
    {
        digitalWrite(address_bus_pins[i], addr & 1);
        addr >>= 1;
    }
}

// Read a byte from the EEPROM at the specified address.
byte readEEPROM(int address) {
    setDataBusMode(INPUT);
    setAddress(address, /*outputEnable*/ true);
    return readDataBus();
}

// Write a byte to the EEPROM at the specified address.
void writeEEPROM(int address, byte data) {
    setAddress(address, /*outputEnable*/ false);
    setDataBusMode(OUTPUT);
    writeDataBus(data);
    enableWrite();
    delayMicroseconds(1);
    disableWrite();
    delay(10);
}

// Read the first 256 byte block of the EEPROM and dump it to the serial monitor.
void printContents() {
    for (int base = 0; base < 256; base += 16) {
        byte data[16];
        for (int offset = 0; offset <= 15; offset += 1) {
            data[offset] = readEEPROM(base + offset);
        }

        char buf[90];
        sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        Serial.println(buf);
    }
}

// Write the special six-byte code to turn off Software Data Protection.
void disableSoftwareWriteProtect() {
    disableWrite();
    setDataBusMode(OUTPUT);

    setByte(0xaa, 0x5555);
    setByte(0x55, 0x2aaa);
    setByte(0x80, 0x5555);
    setByte(0xaa, 0x5555);
    setByte(0x55, 0x2aaa);
    setByte(0x20, 0x5555);

    setDataBusMode(INPUT);
    delay(10);
}

// Write the special three-byte code to turn on Software Data Protection.
void enableSoftwareWriteProtect() {
    disableWrite();
    setDataBusMode(OUTPUT);

    setByte(0xaa, 0x5555);
    setByte(0x55, 0x2aaa);
    setByte(0xa0, 0x5555);

    setDataBusMode(INPUT);
    delay(10);
}

// Set the I/O state of the data bus.
// The 8 bits data bus are defined in the "data_bus_pins" array.
void setDataBusMode(uint8_t mode) {
    if (mode == OUTPUT) {
        for (int i = 0; i < 8; i++) {
            pinMode(data_bus_pins[i], OUTPUT);
        }
    } else {
        for (int i = 0; i <  8; i++) {
            pinMode(data_bus_pins[i], INPUT);
        }
    }
}        


// Read a byte from the data bus.  The caller must set the bus to input_mode
// before calling this or no useful data will be returned.
byte readDataBus() {
    digitalWrite(OUTPUT_EN, LOW);
    delayMicroseconds(1);
    byte data = 0;
    for (int i = 7; i >= 0; i--) {
        data = (data << 1) + digitalRead(data_bus_pins[i]);
    }
    digitalWrite(OUTPUT_EN, HIGH);
    return data;
}

// Write a byte to the data bus.  The caller must set the bus to output_mode
// before calling this or no data will be written.
void writeDataBus(byte data) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(data_bus_pins[i], data & 1);
        data >>= 1;
    }
}

// Set an address and data value and toggle the write control.  This is used
// to write control sequences, like the software write protect.  This is not a
// complete byte write function because it does not set the chip enable or the
// mode of the data bus.
void setByte(byte value, word address) {
    setAddress(address, false);
    writeDataBus(value);

    delayMicroseconds(1);
    enableWrite();
    delayMicroseconds(1);
    disableWrite();
}
