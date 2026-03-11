import struct
import mmap
import os
import time
import numpy as np

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
                    self.mm = mmap.mmap(self.fd, actual_size, mmap.MAP_SHARED, mmap.PROT_READ)
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
            return None, None
        
        if not self.connected:
            if not self._try_connect():
                return None, None

        try:
            header = struct.unpack_from(self.struct_format, self.mm, 0)
            gen_start = header[0]
            timestamp_ns = header[1]
            header_size = struct.calcsize(self.struct_format)

            if len(self.mm) == header_size:
                # No data yet
                return None, None

            if "dqm_adc_baseline" in self.shm_path:
                baseline_nbins = header[-2]
                noise_len = header[-1]
                if baseline_nbins == 0 and noise_len == 0:
                    return None, None

                # Calculate offsets
                hist_offset = header_size
                noise_offset = hist_offset + (baseline_nbins * 8)
                payload_end = noise_offset + (noise_len * 2)
                gen_end_offset = (payload_end + 7) & ~7
        
                # Read gen_end
                gen_end = struct.unpack_from("<Q", self.mm, gen_end_offset)[0]

                # Get histograms
                baseline_hist = np.frombuffer(self.mm, dtype=np.uint64, count=baseline_nbins, offset=hist_offset)
                noise_samples = np.frombuffer(self.mm, dtype=np.int16, count=noise_len, offset=noise_offset)

                # Get relevant header info
                header_info = header[1:]

                data = header_info + (baseline_hist, noise_samples)
            
            elif "dqm_raw_data" in self.shm_path:
                raw_len = header[-1]
                if raw_len == 0:
                    return None, None

                # Raw starts ater header, ends after raw length
                payload_end = header_size + (raw_len * 2)
                gen_end_offset = (payload_end + 7) & ~7
                
                # Read gen_end
                gen_end = struct.unpack_from("<Q", self.mm, gen_end_offset)[0]

                raw_samples = np.frombuffer(self.mm, dtype=np.int16, count=raw_len, offset=header_size)

                data = (header[1], raw_len, raw_samples)

            elif "dqm_daq_data" in self.shm_path:
                num_ops = header[-1]

                # Op speed entries
                entry_dtype = np.dtype([('name', 'S32'), ('speed', 'f8')])

                payload_end = header_size + (num_ops * 40)
                gen_end_offset = (payload_end + 7) & ~7
                
                # Read gen_end
                gen_end = struct.unpack_from("<Q", self.mm, gen_end_offset)[0]

                op_speeds = np.array(np.frombuffer(self.mm, dtype=entry_dtype, count=num_ops, offset=header_size))

                # Get relevant header info
                header_info = header[1:]

                data = (header_info, op_speeds)

            elif "dqm_peak_data" in self.shm_path:
                peak_nbins = header[-1]

                all_hist_offset = header_size
                window_hist_offset = all_hist_offset + (peak_nbins * 8)
                payload_end = window_hist_offset + (peak_nbins * 8)

                # Read gen_end
                gen_end = struct.unpack_from("<Q", self.mm, payload_end)[0]

                # Get histograms
                all_peak_hist = np.frombuffer(self.mm, dtype=np.uint64, count=peak_nbins, offset=all_hist_offset)
                window_peak_hist = np.frombuffer(self.mm, dtype=np.uint64, count=peak_nbins, offset=window_hist_offset)

                # Get relevant header info
                header_info = header[1:]

                data = (header_info, all_peak_hist, window_peak_hist)

            elif "dqm_alarm_data" in self.shm_path:
                # To pass time logic below increment time 
                max_alarms = header[-3]
                num_alarms = header[-2]

                # Alarm entry type
                entry_dtype = np.dtype([('name', 'S128'), ('level', 'u8'), ('time','u8')])
                payload_end = header_size + (max_alarms * 144)

                # Get alarm info
                alarm_info = np.array(np.frombuffer(self.mm, dtype=entry_dtype, count=num_alarms, offset=header_size))

                # Read gen_end
                gen_end = struct.unpack_from("<Q", self.mm, payload_end)[0]

                # Get relevant header info
                header_info = header[1:]

                data = (header_info, alarm_info) 

            else:
                self.mm.seek(0)
                gen_start = struct.unpack_from("<Q", self.mm)[0]
                data = struct.unpack_from(self.struct_format, self.mm)
                gen_end = data[-1]

            if gen_start == gen_end:
                return data, timestamp_ns

            if gen_start != gen_end:
                return None, timestamp_ns

        except (OSError, ValueError) as e:
            print(f"[✗] Error reading memory from {self.shm_path}: {e}")
            self.connected = False
            self.mm = None  # Optional: release reference
            self.last_connected_printed = False            
        return None, None

    def read_updated(self):
        # Try reading a full consistent struct
        data, timestamp_ns = self.read_atomic()
        
        # Case 1: Not connected or can't read atomic block yet
        if not self.connected:
            return None, "waiting for shared memory"
        
        # Case 2: Connected, but couldn't read a stable struct (e.g. not written yet)
        if data is None:
            print("Connected, but couldn't read a stable struct")
            return None, "waiting for new data"
        
        # Case 3: Read same data again
        if timestamp_ns == self._last_timestamp_ns:
            return "no update", "waiting for new data"
        
        # Case 4: New data successfully read
        self._last_timestamp_ns = timestamp_ns

        return data, "new data"
        
    def close(self):
        if self.mm:
            self.mm.close()
        if self.fd:
            os.close(self.fd)

