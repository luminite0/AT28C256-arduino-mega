rom = bytearray([0xea] * 32768)
with open("rom.bin", "wb") as f:
    unused = f.write(rom)
