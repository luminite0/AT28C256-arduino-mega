int address_bus_pins[] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50 };
int data_bus_pins[] = { 31, 33, 35, 37, 39, 41, 43, 45 };
int WRITE_ENABLE = 2;
int OUTPUT_ENABLE = 3;


void pulse_write_en() {
    digitalWrite(WRITE_ENABLE, LOW);
    delayMicroseconds(1);
    digitalWrite(WRITE_ENABLE, HIGH);
}

void set_addr(int addr) {
    for (int i = 0; i < 15; i++) {
        // write the result from anding first bit of addr and 1
        digitalWrite(address_bus_pins[i], addr & 1);
        // right shift so the originally second bit is now the first bit
        addr >>= 1;
    }
}

void set_page_addr(int addr) {
    for (int i = 6; i < 15; i++) {
        pinMode(address_bus_pins[i], OUTPUT);
    }
    for (int i = 6; i < 15; i++) {
        digitalWrite(address_bus_pins[i], addr & 1);
        addr >>= 1;
    }
}

void set_byte_within_page(int which_byte) {
    for (int i = 0; i < 6; i++) {
        pinMode(address_bus_pins[i], OUTPUT);
    }
    // Set which byte of the page is to be written.
    // Byte will be in between 0 and 63 as pages are 64 bytes long.
    for (int i = 0; i < 6; i++) {
        digitalWrite(address_bus_pins[i], which_byte & 1);
        which_byte >>= 1;
    }
}


// Single byte write
void write_data(byte data, int addr) {
    set_addr(addr); 
    for (int i = 0; i < 8; i++) {
        pinMode(data_bus_pins[i], OUTPUT);
    }
    // this works the same as set_addr but with only 8 bits instead of 15
    for (int i = 0; i < 8; i++) {
        digitalWrite(data_bus_pins[i], data & 1);
        data >>= 1;
    }
    // pulse write enable
    pulse_write_en();
    // Give eeprom time to write data.
    // Optionally change this to delay(3) if using the AT28C256F for
    // slightly faster write times.
    delay(10);
}

// Single byte read
byte read_data(int addr) {
    set_addr(addr);
    digitalWrite(OUTPUT_ENABLE, LOW);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++) {
        pinMode(data_bus_pins[i], INPUT);
    }

    byte data = 0;
    for (int i = 7; i >= 0; i--) {
        // left shift existing data, then change final bit
        data = (data << 1) + digitalRead(data_bus_pins[i]);
    }

    digitalWrite(OUTPUT_ENABLE, HIGH);
    return data;
}

// Like write_data except it doesn't set an address.
// This is necessary for page write mode.
void set_data_bus(int data) {
    for (int i = 0; i < 8; i++) {
        pinMode(data_bus_pins[i], OUTPUT);
    }
    for (int i = 0; i < 8; i++) {
        digitalWrite(data_bus_pins[i], data & 1);
        data >>= 1;
    }
}

void write_page(int start_addr) {
    set_page_addr(start_addr);
    byte page_from_serial[64] = { 0 };
    for (int i = 0; i < 64; i++) {
        while (!Serial.available());
        page_from_serial[i] = Serial.read();
    }

    // Write the 64 bytes
    for (int i = 0; i < 64; i++) {
        set_byte_within_page(i);
        set_data_bus(page_from_serial[i]); //0xea); //page_from_serial[i]);
        pulse_write_en();
    }
    // Time out byte load cycle time
    delayMicroseconds(150);
    // Wait for the data to actually be written
    delay(10);

}

void test_write_page() {
    for (int i = 0; i < 64; i++) {
    }
    write_page(0x55);
}

void disable_sdp() {
    write_data(0xaa, 0x5555);
    write_data(0x55, 0x2aaa);
    write_data(0x80, 0x5555);
    write_data(0xaa, 0x5555);
    write_data(0x55, 0x2aaa);
    write_data(0x20, 0x5555);
}

// int file_size is in bytes
void flash_chip() {

    // Binary file is always 32768 bytes in size.
    // A page is 64 bytes, so there are 512 write operations.
    // Page address goes from 0 to 511.
    unsigned int current_page_address = 0;
    
    while (current_page_address < 512) {
        while (!Serial.available());
        // Write the page of 64 bytes starting at the address
        write_page(current_page_address);
        current_page_address++;
    }
}

void dump_chip() {

    unsigned int address = 0x00;
    while (address < 32768) {
        byte data = read_data(address);
        address++;
        Serial.print(data, HEX);
    }
}


void time_write() {
    int before_time = millis();
    write_data(0x48, 0);
    int after_time = millis();
    int difference = after_time - before_time;
    Serial.print("Time in between is: ");
    Serial.print(difference);
    Serial.println();
}

void write_single_value(byte val) {
    int address = 0x00;
    while (address < 32768) {
        write_data(val, address++);
    }
}



void setup() {
    // address bus pins
    for (int i = 0; i < 15; i++) {
        pinMode(address_bus_pins[i], OUTPUT);
    }

    // writing first before pinmode ensures that WE and OE
    // don't go low before we want them to
    digitalWrite(WRITE_ENABLE, HIGH);
    pinMode(WRITE_ENABLE, OUTPUT);

    digitalWrite(OUTPUT_ENABLE, HIGH);
    pinMode(OUTPUT_ENABLE, OUTPUT);

    Serial.begin(51200);
}

void loop() {
    while (!Serial.available());
    byte command = Serial.read();

    // test_write_page();
    if (command == 'w') {
        flash_chip();
    } else if (command == 'r') {
        dump_chip();
    }
}
