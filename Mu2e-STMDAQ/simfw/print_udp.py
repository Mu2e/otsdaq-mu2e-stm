import socket
import struct
import numpy as np
import datetime

MAX_PACKET_LEN = 4099  # Number of int16 values
BYTES_PER_PACKET = MAX_PACKET_LEN * 2  # Each int16 is 2 bytes

UDP_IP = "192.168.34.12"
UDP_PORT = 51872


def receive_packets():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))

    print(f"Listening on {UDP_IP}:{UDP_PORT}...")

    packet_buffer = []
    packet_count = 0

    # Create a file with the current date and time
    filename = datetime.datetime.now().strftime("udp_packets_%Y%m%d_%H%M%S.txt")
    with open(filename, 'w') as outfile:
        try:
            while True:
                data, addr = sock.recvfrom(BYTES_PER_PACKET)

                if len(data) != BYTES_PER_PACKET:
                    print(f"Received malformed packet of size {len(data)} bytes")
                    continue

                # Unpack the data into int16 array
                packet = np.frombuffer(data, dtype=np.int16)
                packet_buffer.append(packet)
                packet_count += 1

                # Process full blocks of 10 packets
                while len(packet_buffer) >= 10:
                    print_header(packet_count - len(packet_buffer) + 1, packet_count - len(packet_buffer) + 10)
                    outfile.write(get_header_string(packet_count - len(packet_buffer) + 1, packet_count - len(packet_buffer) + 10))
                    print_packets(packet_buffer[:10], outfile)
                    packet_buffer = packet_buffer[10:]
        except KeyboardInterrupt:
            if packet_buffer:
                # Print remaining packets if any
                print_header(packet_count - len(packet_buffer) + 1, packet_count)
                outfile.write(get_header_string(packet_count - len(packet_buffer) + 1, packet_count))
                print_packets(packet_buffer, outfile)
            print("\nStopped by user.")

def print_header(start_packet, end_packet):
    header = f"\n--- Displaying packets {start_packet} to {end_packet} ---"
    print(header)

def get_header_string(start_packet, end_packet):
    return f"\n--- Displaying packets {start_packet} to {end_packet} ---\n"

def print_packets(packets, outfile):
    num_packets = len(packets)
    column_width = 10
    for i in range(MAX_PACKET_LEN):
        row = [f"{i:<{column_width}}"]
        for j in range(num_packets):
            row.append(f"{packets[j][i]:<{column_width}}")
        row_str = "".join(row)
        print(row_str)
        outfile.write(row_str + "\n")


if __name__ == "__main__":
    receive_packets()
