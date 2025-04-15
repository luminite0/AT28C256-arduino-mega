# import math

rom = bytearray([0xea] * 32768)
with open("rom.bin", "wb") as f:
    unused = f.write(rom)
    # data = 0
    # num = 32768 / 255
    # for i in range(math.floor(num)):
    #     for j in range(255):
    #         f.write(j.to_bytes(1, byteorder="big"))
