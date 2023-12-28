import socket
import threading
import logging
import logging.config
import os
import json

os.chdir(os.getcwd())

with open('loggingconfig.json','r') as f:
    logging_dictionary = json.load(f)
logging.config.dictConfig(logging_dictionary)
logger = logging.getLogger(__name__)

log_lock = threading.Lock()

# Log the initial welcome message
logger.info("Welcome to Logs")

def change_data(data):
    #Look for VIN:
    if data[4] == 0xEC and data[5] == 0xFE and data[6] == 0:
        fakeVIN = b'TotallyFakeVIN'
        dataout = data[:10] + fakeVIN
        return dataout
    # Look for DDEC Reports data
    elif data[4] == 0x80 and data[5] == 0xC6 and data[6] == 0x11 and data[7] == 0xB6 and data[8] > 10:
        fakedata = b'5'*15
        dataout = data[:9] + fakedata
        logger.debug(f'Changed data to {dataout}')
        return dataout
    else:
        return data #passthrough

def handle_RP1210ReadMessage():
    # Create a UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Client address and port
    client_address = ("127.0.0.1", 12354)

    # Bind the socket to the client address and port
    udp_socket.bind(client_address)

    logger.info(f"RP1210ReadMessage (RM) UDP Echo Client listening on {client_address}")


    while True:
        # Receive data from the socket
        data, addr = udp_socket.recvfrom(4400)  # Adjust the buffer size as needed

        # Print the received data and the sender's address
        hex_data = ' '.join(f'{byte:02x}' for byte in data)
        with log_lock:  # This lock ensures atomic writes in a multithreaded environment
            logger.info(f"RM {hex_data}")

        data = change_data(data)

        # Echo the received data back to the sender
        udp_socket.sendto(data, addr)

def handle_RP1210SendMessage():
    # Create a UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Client address and port
    client_address = ("127.0.0.1", 12352)

    # Bind the socket to the client address and port
    udp_socket.bind(client_address)

    logger.info(f"RP1210SendMessage (SM) UDP Echo Client listening on {client_address}")


    while True:
        # Receive data from the socket
        data, addr = udp_socket.recvfrom(4400)  # Adjust the buffer size as needed

        hex_data = ' '.join(f'{byte:02x}' for byte in data)
        with log_lock:  # This lock ensures atomic writes in a multithreaded environment
            logger.info(f"SM {hex_data}")

        # Echo the received data back to the sender
        udp_socket.sendto(data, addr)

def handle_RP1210Messages():
    # Create a UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Client address and port
    client_address = ("127.0.0.1", 12353)

    # Bind the socket to the client address and port
    udp_socket.bind(client_address)

    logger.info(f"RP1210Messages listenining on {client_address}")


    while True:
        # Receive data from the socket
        data, addr = udp_socket.recvfrom(4400)  # Adjust the buffer size as needed

        # Print the received data and the sender's address
        with log_lock:  # This lock ensures atomic writes in a multithreaded environment
            logger.info(f"Message: {data.decode('utf-8','ignore')}")
        
logger.info("Logs for RP1210 communications.")

# Create and start threads for each port
thread_Send = threading.Thread(target=handle_RP1210SendMessage, args=())
thread_Read = threading.Thread(target=handle_RP1210ReadMessage, args=())
thread_Msge = threading.Thread(target=handle_RP1210Messages, args=())

thread_Send.start()
thread_Read.start()
thread_Msge.start()

# Wait for both threads to finish (which will never happen in this example)
thread_Send.join()
thread_Read.join()
thread_Msge.join()