import struct
import mmap
import os
import time

class SharedMemoryReader:
    def __init__(self, shm_path, struct_format, max_retries=1, retry_delay=0.1):
        self.shm_path = "/dev/shm" + shm_path
        self.struct_format = struct_format
        self.struct_size = struct.calcsize(struct_format)
        self._last_timestamp_ns = 0
        self.connected = False            # Tracks whether shared memory is connected
        self.last_error_printed = False  # Avoids spamming error messages
        self.last_connected_printed = False  # Avoids spamming reconnection message
        self.mm = None
        self.fd = None

        self._try_connect()

    def _try_connect(self):
        try:
            if os.path.exists(self.shm_path):
                actual_size = os.stat(self.shm_path).st_size
                if actual_size >= self.struct_size:
                    self.fd = os.open(self.shm_path, os.O_RDONLY)
                    self.mm = mmap.mmap(self.fd, self.struct_size, mmap.MAP_SHARED, mmap.PROT_READ)
                    self.connected = True
                    if not self.last_connected_printed:
                        print(f"[✓] Connected to shared memory: {self.shm_path}")
                        self.last_connected_printed = True
                    self.last_error_printed = False
                else:
                    if not self.last_error_printed:
                        print(f"[✗] Shared memory file too small: {actual_size} < {self.struct_size}")
                        self.last_error_printed = True
            else:
                if not self.last_error_printed:
                    print(f"[✗] No shared memory connection for {self.shm_path}. Waiting for connection...")
                    self.last_error_printed = True
        except Exception as e:
            if not self.last_error_printed:
                print(f"[✗] Error initializing shared memory: {e}")
                self.last_error_printed = True
        return self.connected

    def read_atomic(self):

        if not os.path.exists(self.shm_path):
            self.connected = False
            return None
        
        if not self.connected:
            if not self._try_connect():
                return None

        try:
            self.mm.seek(0)
            gen_start = struct.unpack_from("<Q", self.mm)[0]
            data = struct.unpack_from(self.struct_format, self.mm)
            gen_end = data[-1]
            if gen_start == gen_end:
                return data
        except (OSError, ValueError) as e:
            print(f"[✗] Error reading memory from {self.shm_path}: {e}")
            self.connected = False
            self.mm = None  # Optional: release reference
            self.last_connected_printed = False            
        return None

    def read_updated(self):
        # Try reading a full consistent struct
        data = self.read_atomic()
        
        # Case 1: Not connected or can't read atomic block yet
        if not self.connected:
            return None, "waiting for shared memory"
        
        # Case 2: Connected, but couldn't read a stable struct (e.g. not written yet)
        if data is None:
            return None, "waiting for new data"
        
        # Case 3: Read same data again
        timestamp_ns = data[1]
        if timestamp_ns == self._last_timestamp_ns:
            return None, "waiting for new data"
        
        # Case 4: New data successfully read
        self._last_timestamp_ns = timestamp_ns

        return data, "new data"
        
    def close(self):
        if self.mm:
            self.mm.close()
        if self.fd:
            os.close(self.fd)

