from flask import Flask, render_template, jsonify
import struct
import mmap
import os
import time
import threading
import xml.etree.ElementTree as ET
from flask_cors import CORS

# Initialize Flask app
app = Flask(__name__)
CORS(app)  # Allow external access

# Load shared memory settings from XML
def load_config():

    xmlFile = os.environ['STM_XML']
    # if (xmlFile):
    #     print("Error: " + xmlFile + "does not exist!")
    #     exit()
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

    if shm_size < struct.calcsize("Q Q 16Q 1000f 10f 10i i f i"):  # Minimum required memory
        raise ValueError("Error: Shared memory size too small! Increase size in config.xml")

    print(f"Loaded Config: Shared Memory Name = {shm_name}, Size = {shm_size} bytes")
    return shm_name, shm_size

# Load shared memory settings
try:
    SHM_NAME, SHM_SIZE = load_config()
except ValueError as e:
    print(e)
    exit(1)

# Store last data and connection state
last_data_timestamp = 0
error_log = []  # Store last 10 errors for frontend (in memory)
shared_memory_connected = False  # Tracks shared memory connection state
last_valid_data = {
    "totalBytesProcessed": 0,
    "activeThreads": 0,
    "threadBytesProcessed": [0] * 16,
    "bufferData": [0] * (10 * 100),
    "bufferAverages": [0] * 10,
    "bufferCount": 0,
    "threshold": 0.0,
    "paused": False,
    "status": "No shared memory detected. Retrying..."
}


def log_event(message, level="info"):
    """ Stores an event in memory instead of writing to a file """
    global error_log
    log_entry = f"{time.strftime('%Y-%m-%d %H:%M:%S')} - {level.upper()} - {message}"

    error_log.append(log_entry)
    if len(error_log) > 10:  # Keep only last 10 errors
        error_log.pop(0)


def read_dqm_data():
    """ Reads DQM data from shared memory. Keeps last valid data if lost. """
    global last_data_timestamp, last_valid_data, shared_memory_connected
    print("Reading dqm data")
    reconnected = False  # Track if shared memory was just reconnected

    # If shared memory is missing, wait and return last known data
    while not os.path.exists(SHM_NAME):
        if shared_memory_connected:
            log_event("Lost connection to shared memory. Retrying...", "warning")
            shared_memory_connected = False
        time.sleep(2)  # Wait before retrying
        return {**last_valid_data, "reconnected": False}  # Show last valid data

    # Check if shared memory is empty before trying to read
    file_size = os.path.getsize(SHM_NAME)
    if file_size < SHM_SIZE:
        log_event(f"Shared memory file too small ({file_size} bytes). Retrying...", "warning")
        return {**last_valid_data, "reconnected": False}

    # If shared memory was lost and is now found, notify reconnection
    if not shared_memory_connected:
        log_event("Shared memory reconnected!", "info")
        shared_memory_connected = True
        reconnected = True  # Flag for frontend notification

    log_event("Shared memory coonnected!", "info")
        
    try:
        with open(SHM_NAME, "r+b") as f:
            with mmap.mmap(f.fileno(), SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ) as mm:
                # header_size = struct.calcsize("Q Q")
                # total_bytes, active_threads = struct.unpack("Q Q", mm[:header_size])

                header_format = "Q Q"  # Two uint64_t values (8 + 8 = 16 bytes)
                header_size = struct.calcsize(header_format)
                
                # Read the first 16 bytes (total_bytes_processed and active_threads)
                total_bytes, active_threads = struct.unpack(header_format, mm[:header_size])
                
                thread_bytes_size = struct.calcsize(f"{16}Q")
                thread_bytes = struct.unpack(f"{16}Q", mm[header_size:header_size + thread_bytes_size])

                buffer_raw_size = struct.calcsize(f"{10 * 100}h")
                buffer_raw = struct.unpack(f"{10 * 100}h", mm[header_size + thread_bytes_size:header_size + thread_bytes_size + buffer_raw_size])

                buffer_averages_size = struct.calcsize(f"{10}h")
                buffer_averages = struct.unpack(f"{10}h", mm[header_size + thread_bytes_size + buffer_raw_size:header_size + thread_bytes_size + buffer_raw_size + buffer_averages_size])

                remaining_size = struct.calcsize(f"{10}i i f i")
                 # buffer_metadata = struct.unpack(f"{10}i i f i", mm[header_size + thread_bytes_size + buffer_raw_size + buffer_averages_size:])

                buffer_count = 5 # buffer_metadata[10]
                threshold = 1000 # buffer_metadata[11]
                paused = False # buffer_metadata[12]

                last_data_timestamp = time.time()  # Update timestamp when new data is received

                last_valid_data = {  # Store latest valid data
                    "totalBytesProcessed": total_bytes,
                    "activeThreads": active_threads,
                    "threadBytesProcessed": thread_bytes[:active_threads],
                    "bufferData": buffer_raw,
                    "bufferAverages": buffer_averages[:buffer_count],
                    "bufferCount": buffer_count,
                    "threshold": threshold,
                    "paused": bool(paused),
                    "status": "Receiving data",
                    "reconnected": reconnected  # Notify frontend if reconnection happened
                }

                print(last_valid_data)
                # Only log if actual data was read
                log_event("Successfully read shared memory data.")
                return last_valid_data

    except Exception as e:
        log_event(f"Error reading shared memory: {str(e)}", "error")
        return {**last_valid_data, "reconnected": False}  # Keep showing last known data

@app.route("/")
def index():
    return render_template("index.html")


@app.route("/dqm_data")
def dqm_data():
    """ Returns DQM data, notifying if shared memory has reconnected. """
    return jsonify(read_dqm_data())


@app.route("/error_log")
def get_error_log():
    """ Returns the last 10 logged events. """
    return jsonify({"errors": error_log})


def run_flask():
    """ Starts the Flask server. """
    log_event("Starting Flask webserver...")
    app.run(host="127.0.0.1", port=5000, debug=True, use_reloader=False)


if __name__ == "__main__":
    threading.Thread(target=run_flask).start()

