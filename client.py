import socket
import random

SERVER_IP = "127.0.0.1"
SERVER_PORT = 5000
MAX_KEY_SIZE = 32
MAX_DATA_SIZE = 1024
BUFFER_SIZE = MAX_KEY_SIZE + MAX_DATA_SIZE
DELIM = "!"

commands = ["g", "s", "d"]
chars = ["w", "x", "y", "z"]
data = [
    "jkgl;fbk;lfdkl;kfdl;k;vk  l;kl;fkgl;k;fl",
    "SkYcbPLkLsCtRDgO4iYV23mbMCaKXPpNLHB8XMIkfZEWAWaAeN6f7PcnNSaxM9jWLdH63Zw8NGSpv44VlOKWCeQ6ABj24yNlIxotdYpspZhY3WRQuQSj7WyvQfBTCpWNLd6NeCOOzNy3RkSYTUW5CyLwzTZRJZpM0RSIdqJVWV28v4W33C4y7AVRRb1r7k7xl6sQeRnO0Iw9eV76YujP0IMKGwfeB9pWWhvppoO2NM3ecXT6dK2XVnQpFoUbsfT1naVMb6CJds8VuAt7kuspjC1ZuUkvZhsx613FNujAYGsarnYJHzzYMFzaOh5mgn8ZfnYc6HaTgXmVrOwHCnsNuC3jLKF8bZG4BqpyFaOmDb0uQDRlcsYlu7InM7fL7Ri6XTOT18ePMINB4rNAr1qC2mQQ1jNxIdzCgSPv8nKubdc8vbjYFhy41EPNwZPtJHKAeqsm636nzlEIDU2XvqoJZIXYrJrFugsOOjVMYS3VxLqtswXaflJnOIEZrksbSB70wagrG2tvVvugrMD81cPFWkSP3B8ZJyjpPrCWli6z17nctxmm0KXcMKNr26ccTq4nSDPgMorMRA2ZhDC7UJEfpiIvThQVxcl92PghL1XV4B3KteoW4aAMDC9HHs3dLjIa8RFXK1OETyuHQy5lxm1eqnxabFxxXt3LEcbNzQoIWW5zOnGacWdIFXRCoUpsh3E4fCuUBTTe4YwxJtGbmq9vOJW964ZipYCXQXOXlxHrOgcI87kMjNl5s0lNQeNmEHEAb8r7fSVL0VFEELR9WF0L6KSyvJcLtBgSQgA7itlYwFspkDXd1EffM9VE5SS15s8Cd7z6wU3hQIX5i0RZ2X8xxPGmG8q3J8n1vSMcVXUv3dQB2RiEiIs6nCp1n5Y7J9Mm3ku77L35v9qAUaV3cmj7R2cqkZtwObDrmabYXQ6Od5GaAsdxZQfy662jh8qApxbLvUkN14xQZoIfElYZNpkc0VTDt6939B0dwFjGYv88tQke43nPH5SRvqI8mubwagZd",
    "121325644565656215564544g56f456fg5454f"
    ]

def get_random_command():
    command = random.choice(commands) + "".join(random.choice(chars) for _ in range(30)) + DELIM + random.choice(data)
    return command.encode()

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
        random_command = get_random_command()
        client_socket.sendall(random_command)
        client_socket.recv(BUFFER_SIZE)
        print("Completed", i)

    client_socket.close()

if __name__ == "__main__":
    run_client()
