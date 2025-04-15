int address_bus_pins[] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50 };
int data_bus_pins[] = { 31, 33, 35, 37, 39, 41, 43, 45 };
int WRITE_ENABLE = 2;
int OUTPUT_ENABLE = 3;




void set_addr(int addr) {
    for (int i = 0; i < 15; i++) {
        // write the result from anding first bit of addr and 1
        digitalWrite(address_bus_pins[i], addr & 1);
        // right shift so the originally second bit is now the first bit
        addr >>= 1;
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
    digitalWrite(WRITE_ENABLE, LOW);
    delayMicroseconds(1);
    digitalWrite(WRITE_ENABLE, HIGH);
    // give eeprom time to write data
    delay(10);
}

// Single byte read
byte read_data(int addr) {
    set_addr(addr);
    digitalWrite(OUTPUT_ENABLE, LOW);
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


bool is_page_write_done() {
    if (1) {
        return true;
    } else {
        return false;
    }
}


void write_page(byte data[], int start_addr) {
}

void disable_sdp() {
    write_data(0xaa, 0x5555);
    write_data(0x55, 0x2aaa);
    write_data(0x80, 0x5555);
    write_data(0xaa, 0x5555);
    write_data(0x55, 0x2aaa);
    write_data(0x20, 0x5555);
}

void flash_chip() {
    
    // reset address
    int address = 0x00;
    while (address < 32768) {
        while (!Serial.available()); // wait until there's data to read
        byte buff = Serial.read();
        write_data(buff, address);
        address++;
    }
}


void dump_chip() {

    int address = 0x00;
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

    Serial.begin(115200);
}

void loop() {
    
    while (!Serial.available());
    byte command = Serial.read();
    if (command == 'w') {
        // Temporary until page writing is implemented
        Serial.end();
        Serial.begin(800);
        flash_chip();
    } else if (command == 'r') {
        dump_chip();
    }
}
