// address to write data byte to
int address = 0x00;

int address_bus_pins[] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52 };
int data_bus_pins[] = { 31, 33, 35, 37, 39, 41, 43, 45 };
int WRITE_ENABLE = 2;
int OUTPUT_ENABLE = 3;
byte command;

byte read_data_buff = 0;

void setup() {
    // address bus pins
    for (int i = 0; i < 16; i++) {
        pinMode(address_bus_pins[i], OUTPUT);
    }

    // writing first before pinmode ensures that WE
    // doesn't go low before we want it to
    digitalWrite(WRITE_ENABLE, HIGH);
    pinMode(WRITE_ENABLE, OUTPUT);

    digitalWrite(OUTPUT_ENABLE, HIGH);
    pinMode(OUTPUT_ENABLE, OUTPUT);

    Serial.begin(115200);
}

void set_addr(int addr) {
    for (int i = 0; i < 16; i++) {
        // write the result from anding first bit of addr and 1
        digitalWrite(address_bus_pins[i], (addr & 1) ? 1 : 0);
        // right shift so the originally second bit is now the first bit
        addr >>= 1;
    }
}


void write_data(byte data, int addr) {
    set_addr(addr);
    // this works the same as set_addr but with only 8 bits instead of 16
    for (int i = 0; i < 8; i++) {
        pinMode(data_bus_pins[i], OUTPUT);
        digitalWrite(data_bus_pins[i], data & 1);
        data >>= 1;
    }
    // pulse write enable
    digitalWrite(WRITE_ENABLE, LOW);
    delayMicroseconds(1);
    digitalWrite(WRITE_ENABLE, HIGH);
    // give eeprom time to write data
    delay(5);
}


byte read_data(int addr) {
    digitalWrite(OUTPUT_ENABLE, LOW);
    set_addr(addr);
    delayMicroseconds(1);
    byte data = 0;
    for (int i = 7; i >= 0; i--) {
        pinMode(data_bus_pins[i], INPUT);
        // left shift existing data, then change final bit
        data = (data << 1) + digitalRead(data_bus_pins[i]);
    }
    digitalWrite(OUTPUT_ENABLE, HIGH);
    return data;
}


void flash_chip() {
    // reset address
    address = 0x00;
    while (true) {
        while (!Serial.available()); // wait until there's data to read
        byte buff = Serial.read();
        write_data(buff, address);
        address++;
    }
}

void dump_chip() {
    address = 0x00;
    read_data_buff = 0;
    while (true) {
        byte data = read_data(address);
        address++;
        Serial.print(data, HEX);
    }
}

void loop() {
    while (!Serial.available());
    byte command = Serial.read();
    if (command == 'w') {
        flash_chip();
    } else if (command == 'r') {
        dump_chip();
    }
}
