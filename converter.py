assembly_big_endian = "\x8d\x30\x00\x14\x02\x0b\x50\x22\x10\x00\xff\xff\x00\x00\x00\x00"
for i in range(len(assembly_big_endian)/4):
    print(''.join([ "{:02x}".format(ord(i)) for i in assembly_big_endian[i*4:i*4+4] ]))

