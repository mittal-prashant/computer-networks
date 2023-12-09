import socket

IP = socket.gethostbyname(socket.gethostname())
PORT = 4455
ADDR = (IP, PORT)
SIZE = 1024
FORMAT = "utf-8"


def main():

    # Create a TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        # Bind the socket to the IP and PORT
        server.bind(ADDR)
        # Listen for incoming connections
        server.listen()
        print("[LISTENING] Server is listening.")

        # Accept the incoming connection
        conn, addr = server.accept()
        print(f"[NEW CONNECTION] {addr} connected.")

        # Receive the filename from the client
        filename = conn.recv(SIZE).decode(FORMAT)
        print(f"[RECEIVING] Filename received: {filename}")

        # Create a file with the received filename
        with open(filename, "w") as file:
            # Send an acknowledgment to the client that filename is received
            conn.send("OK".encode(FORMAT))
            print("[SENDING] Filename received acknowledgment sent.")

            # Receive the file data from the client
            data = conn.recv(SIZE).decode(FORMAT)
            print("[RECEIVING] Receiving the file data.")
            # Write the received data to the file
            file.write(data)
            # Send an acknowledgment to the client that file data is received
            conn.send("OK".encode(FORMAT))
            print("[SENDING] File data received acknowledgment sent.")

        # Close the connection
        conn.close()
        print(f"[DISCONNECTED] {addr} disconnected.")


if __name__ == "__main__":
    main()
