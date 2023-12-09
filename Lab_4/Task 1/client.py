import socket

IP = socket.gethostbyname(socket.gethostname())
PORT = 4455
ADDR = (IP, PORT)
FORMAT = "utf-8"
SIZE = 1024


def main():
    # Create a TCP socket
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect to the server
    client.connect(ADDR)

    # Open and read the file data
    file = open("file/example.txt", "r")
    data = file.read()

    # Send the filename to the server
    client.send("example.txt".encode(FORMAT))
    response = client.recv(SIZE).decode(FORMAT)
    print(f"[SERVER RESPONSE TO FILENAME]: {response}")

    # Send the file data to the server
    client.send(data.encode(FORMAT))
    response = client.recv(SIZE).decode(FORMAT)
    print(f"[SERVER RESPONSE TO FILEDATA]: {response}")

    # Close the file
    file.close()

    # Close the connection to the server
    client.close()


if __name__ == "__main__":
    main()
