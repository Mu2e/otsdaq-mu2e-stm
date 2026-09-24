import socket
import pickle
import numpy as np
localIP     = "127.0.0.2"
localPort   = 10000
bufferSize  = 65536

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))

print("UDP server up and listening")


# Listen for incoming datagrams
while(True):
    byte_data,addr = UDPServerSocket.recvfrom(bufferSize)
    data = np.frombuffer(byte_data, dtype=np.int16)
    for i in range(len(data)):
        print(data[i])
    print(len(data))

    # Sending a reply to client
 #   UDPServerSocket.sendto(bytesToSend, address)
