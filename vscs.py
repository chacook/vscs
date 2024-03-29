import select
import socket
import time

SERVER_IP = "127.0.0.1"
SERVER_PORT = 5000
BACKLOG_LENGTH = 1000
DELIM = b"!"
MAX_KEY_SIZE = 63
MAX_DATA_SIZE = 1024
BUFFER_SIZE = MAX_KEY_SIZE + MAX_DATA_SIZE

"""
PROTOCOL:
get request: g<KEY>!
set request: s<KEY>!<DATA>
delete request: d<KEY>!
clear cache request: c!

get response: <DATA> or /x00 if null
set response: !
delete response: !
clear response: !
"""

def run_server():
	try:
		server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		server_socket.bind((SERVER_IP, SERVER_PORT))
		server_socket.listen(BACKLOG_LENGTH)
		print(f"Server listening at {SERVER_IP}:{SERVER_PORT}")
	except Exception as e:
		print(e)
		exit(0)

	clients = []
	cache = {}
	total_requests = 0
	total_time = 0.0

	while True:
		try:
			readable, _, _ = select.select([server_socket, *clients], [], [])

			for current_socket in readable:
				if current_socket is server_socket:
					client_conn, client_addr = server_socket.accept()
					clients.append(client_conn)
				else:
					start_time = time.time()
					request_binary = current_socket.recv(BUFFER_SIZE)

					if request_binary is None or len(request_binary) == 0:
						current_socket.close()
						clients.remove(current_socket)
						continue

					key_and_data = request_binary.split(DELIM, 1)
					key = key_and_data[0].decode()
					key_length = len(key)
					data = request_binary[key_length + 1:]

					if key_length == 0 or key_length > MAX_KEY_SIZE:
						current_socket.close()
						clients.remove(current_socket)
						continue

					command = key[0]
					key = key[1:]

					if command == "g":
						response = cache.get(key, b"\x00")
						current_socket.sendall(response)
					elif command == "s" and len(data) > 0:
						cache[key] = data
						current_socket.sendall(DELIM)
					elif command == "d":
						cache.pop(key, None)
						current_socket.sendall(DELIM)
					elif command == "c":
						cache.clear()
						current_socket.sendall(DELIM)
					else:
						current_socket.close()
						clients.remove(current_socket)
						continue

					end_time = time.time()
					total_time += end_time - start_time
					total_requests += 1
		except KeyboardInterrupt:
			if total_time > 0:
				print(f"\nCompleted {total_requests} requests in {total_time} seconds. \nAverage: {total_requests / total_time} per second.")
			exit(0)
		except Exception as e:
			print(e)

if __name__ == "__main__":
	run_server()
