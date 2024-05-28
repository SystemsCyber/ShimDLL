import socket
import threading
import queue
import logging
import logging.config
import os
import json
import sqlite3
import struct
import time

os.chdir(os.getcwd())
db_path = 'RP1210Message.db'

with open('SocketShim\loggingconfig.json','r') as f:
    logging_dictionary = json.load(f)
logging.config.dictConfig(logging_dictionary)
logger = logging.getLogger(__name__)

# Define database schema
create_table_sql = """
CREATE TABLE IF NOT EXISTS RP1210_messages (
    id INTEGER PRIMARY KEY,
    direction TEXT,
    timestamp INTEGER,
    pgn INTEGER,
    pgn_hex TEXT,
    priority INTEGER,
    sa INTEGER,
    da INTEGER,
    can_bytes BLOB,
    can_text TEXT
);
"""


# Function to initialize the database
def init_db(db_path):
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute(create_table_sql)
    conn.commit()
    return conn

# Function to insert CAN message into the database
def insert_RP1210_J1939_message(conn,direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text):
    cursor = conn.cursor()
    cursor.execute(
        "INSERT INTO RP1210_messages (direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text) VALUES (?,?,?,?,?,?,?,?,?)",
        (direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text)
    )
    conn.commit()

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

def handle_RP1210ReadMessage(message_queue):
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
        direction = "RM"
        timestamp = struct.unpack("<L",data[0:4])[0]
        pgn = struct.unpack("<L",data[0:3]+b'\x00')[0]
        pgn_hex="{:X}".format(pgn)
        priority = data[3]
        sa = data[4]
        da = data[5]
        can_bytes= data[6:]
        can_text = ' '.join(f'{b:02x}' for b in data[6:])
        message_queue.put((direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text))
        

        # Echo the received data back to the sender
        udp_socket.sendto(data, addr)

def handle_RP1210SendMessage(message_queue):
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
        
        
        direction = "SM"
        timestamp = time.time()
        pgn = struct.unpack("<L",data[4:7]+b'\x00')[0]
        pgn_hex="{:X}".format(pgn)
        priority = data[7]
        sa = data[8]
        da = data[9]
        can_bytes= data[10:]
        can_text = ' '.join(f'{b:02x}' for b in data[10:])
        message_queue.put((direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text))
        

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
        
        # Echo the received data back to the sender
        udp_socket.sendto(data, addr)

def handle_Database(message_queue):
    # Initialize the database
    conn = init_db(db_path)
    while True:
        direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text = message_queue.get()
        insert_RP1210_J1939_message(conn,direction,timestamp,pgn,pgn_hex,priority,sa,da,can_bytes,can_text)

logger.info("Logs for RP1210 communications.")


# Create a queue
message_queue = queue.Queue()

# Create and start threads for each port
thread_Send = threading.Thread(target=handle_RP1210SendMessage, args=(message_queue,))
thread_Read = threading.Thread(target=handle_RP1210ReadMessage, args=(message_queue,))
thread_Msge = threading.Thread(target=handle_RP1210Messages, args=())
thread_database = threading.Thread(target=handle_Database, args=(message_queue,))

thread_Send.start()
thread_Read.start()
thread_Msge.start()
thread_database.start()

# Wait for both threads to finish (which will never happen in this example)
thread_Send.join()
thread_Read.join()
thread_Msge.join()
thread_database.join()