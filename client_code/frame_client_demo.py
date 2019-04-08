#Based on a UDP echo client copied from https://pymotw.com/3/socket/udp.html

import socket
import sys

# for random message to send
import string
import random

# for drawing image
import matplotlib.pyplot as plt
import numpy as np

server_address = '192.168.0.16'
server_port = 44400

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server = (server_address, server_port)
sock.connect(server)
#message = b'This is the message.  It will be repeated.'

try:
    for x in range(0,3):
        # random message to send, as a bytes object
        message = bytes(''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(42)), 'utf-8')
        # Send data
        print('sending {!r}'.format(message))
        sent = sock.sendto(message, server)
        # Receive response
        print('waiting to receive')
        data, server_addr = sock.recvfrom(1024)

        data_str = data.decode("ascii")
        # for echo server: convert received string to 6x7 matrix
        #data_matrix = np.array(data_str, "c").view(np.uint8).reshape(6,7)
        # for image server: convert received string to 128x128 matrix
        data_matrix = np.array(data_str, "c").view(np.uint8).reshape(128,128)

        print('received data size = {!r}'.format(len(data_str)))
        plt.imshow(data_matrix, cmap="gray")
        plt.ion()
        plt.show()
        plt.pause(1)

        print('received {!r}'.format(data))
    ###

finally:
    print('closing socket')
    sock.close()
