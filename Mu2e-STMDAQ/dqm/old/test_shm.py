import mmap
import os
import struct
import xml.etree.ElementTree as ET

# Load shared memory name and size from XML
def load_config():
    xmlFile = os.environ['STM_XML']
    tree = ET.parse(xmlFile) 
    root = tree.getroot()
    
    shm_element = root.find("./DQM")
    if shm_element is None:
        raise ValueError("Error: <shared_memory> section missing in config.xml")
    
    name_element = shm_element.find("name")
    size_element = shm_element.find("size")

    if name_element is None or size_element is None:
        raise ValueError("Error: Missing <name> or <size> in config.xml")

    shm_name = "/dev/shm/"+name_element.text
    shm_size = int(size_element.text)

    return shm_name, shm_size

# Read from shared memory and print contents
def read_shared_memory(shm_name, shm_size):
    if not os.path.exists(shm_name):
        print(f"Error: Shared memory '{shm_name}' not found.")
        return

    with open(shm_name, "r+b") as f:
        with mmap.mmap(f.fileno(), shm_size, mmap.MAP_SHARED, mmap.PROT_READ) as mm:
            raw_data = mm.read(shm_size)  # Read entire shared memory
            print("Raw Shared Memory Contents (First 64 bytes):")
            print(raw_data[:64])  # Print first 64 bytes for readability

            # Attempt structured unpacking (Modify format based on your data structure)
            try:
                header_format = "Q Q"  # Example: total_bytes (Q), active_threads (Q)
                header_size = struct.calcsize(header_format)
                total_bytes, active_threads = struct.unpack(header_format, raw_data[:header_size])

                print("\nParsed Shared Memory Data:")
                print(f"Total Bytes Processed: {total_bytes}")
                print(f"Active Threads: {active_threads}")

            except struct.error:
                print("Error: Could not unpack structured data. Check format.")

# Load config and run test
if __name__ == "__main__":
    try:
        SHM_NAME, SHM_SIZE = load_config()
        read_shared_memory(SHM_NAME, SHM_SIZE)
    except Exception as e:
        print(f"Error: {e}")
