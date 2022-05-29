import socket

SERVER_IP = "127.0.0.1"
SERVER_PORT = 5000

def run_client():
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        client_socket.connect((SERVER_IP, SERVER_PORT))
        print(f"Connected to server at {SERVER_IP}:{SERVER_PORT}")
    except Exception as e:
        print(e)
        exit(1)

    for i in range(1_000_000):
        client_socket.sendall(b"hi")
        r = client_socket.recv(1024)
        #print(r.decode())

if __name__ == "__main__":
    run_client()