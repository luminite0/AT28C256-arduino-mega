rom = bytearray([0xea] * 32768)
with open("rom.bin", "wb") as f:
    f.write(rom)
