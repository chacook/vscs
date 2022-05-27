# very simple cache server
import socket
import hashlib
import time

SERVER_IP = "127.0.0.1"
SERVER_PORT = 5000
BACKLOG_LENGTH = 1000
MAX_KEY_SIZE = 32
MAX_DATA_SIZE = 1024
BUFFER_SIZE = MAX_KEY_SIZE + MAX_DATA_SIZE
DELIM = "!"

"""
get request: g<KEY>!
set request: s<KEY>!<DATA>

get response: <DATA> or </x00> if null
set response: !
"""

def start_server():
	 try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((SERVER_IP, SERVER_PORT))
        server_socket.listen(BACKLOG_LENGTH)
        server_socket.setblocking(False)  # necessary?
        print(f"Server listening at {SERVER_IP}:{SERVER_PORT}")
    except Exception as e:
        print(e)
        exit(1)

    clients = []
    cache = {}
    total_requests = 0
    total_time = 0.0
    start_time = 0.0
    end_time = 0.0

    while True:
        try:
            readable, writeable, exceptional = select.select([server_socket] + clients, [], [])

            for current_socket in readable:
                if current_socket is server_socket:
                    client_conn, client_addr = server_socket.accept()
                    clients.append(client_conn)
                else:
                    client_request_bin = current_socket.recv(BUFFER_SIZE)
                    start_time = time.time()

                    if client_request_bin is None:
                        current_socket.close()
                        clients.remove(current_socket)
                        continue

					client_request_decoded = client_request_bin.decode()

                    if client_request_decoded == "":
                        current_socket.close()
                        clients.remove(current_socket)
                        continue

                    key_and_data = client_request_decoded.split(DELIM, 1)
                    key = key_and_data[0]
                    key_length = len(key)
                    data = client_request_bin[key_length + 1:]

                    if key_length == 0 or key_length > MAX_KEY_SIZE or len(data) == 0:
                    	current_socket.close()
                        clients.remove(current_socket)
                        continue

                    command = key[0]
                    key = key[1:]

                    if command == "g":
                    	response = cache.get(key, b"\x00")
           				current_socket.sendall(response)
                    elif command == "s":
                    	cache[key] = data
                    	current_socket.sendall(b"!")
                   	else:
                   		current_socket.close()
                        clients.remove(current_socket)
                        continue

                    total_requests += 1
                	end_time = time.time()
       				total_time += end_time - start_time
        except KeyboardInterrupt:
            exit(0)
        except Exception as e:
            print(e)

if __name__ == "__main__":
	start_server()
