import mmap
import os
import struct

class SharedMemoryReader:
    def __init__(self, shm_path, shm_size):
        self.shm_path = shm_path
        self.shm_size = shm_size

    def read(self, schema):
        values = {}
        offset = 0
        with open(self.shm_path, "rb") as f:
            mm = mmap.mmap(f.fileno(), self.shm_size, access=mmap.ACCESS_READ)
            for name, fmt in schema:
                size = struct.calcsize(fmt)
                raw = mm[offset:offset+size]
                unpacked = struct.unpack(fmt, raw)
                values[name] = unpacked if len(unpacked) > 1 else unpacked[0]
                offset += size
        return values
