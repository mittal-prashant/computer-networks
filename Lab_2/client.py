import socket


def receive_ack(client_socket):
    try:
        data = client_socket.recv(1024).decode()  # receive NAK
        return data
    except socket.timeout:  # timeout exception
        print("Timeout Occurred at client side")
        return -1


def client_program():
    n = 10
    win_start = 0
    win_end = win_start + n - 1
    host = socket.gethostname()  # as both code is running on same pc
    port = 12344  # socket server port number
    sender = []
    flag = 0  # send whole sender list else 1 means send only win_start frame
    client_socket = socket.socket()  # instantiate
    client_socket.connect((host, port))  # connect to the server
    client_socket.settimeout(5)  # set timeout to 5 seconds
    print('Window Size is ', n)
    print('******** Enter "stop" to close connection ***************')
    message = input(
        "Hit any key to start sending frames -> ")  # take input

    while message.lower().strip() != 'stop':
        print("Sending frames...")
        if flag == 0:
            sender = [win_start + i for i in range(n)]
            for i in sender:
                print("Frame -> ", i)
        else:
            print("Frame -> ", win_start)
        msg = str(win_start)
        client_socket.send(msg.encode())  # send message
        data = receive_ack(client_socket)
        if data == -1:  # timeout occurred
            if flag == 0:
                sender = []  # remove all frames from sender buffer if flag = 0
            print("Resending frames...")
            continue
        msg = str(data)
        ack = int(msg)
        if ack not in sender:
            win_start = ack
            win_end = win_start + n - 1
            flag = 0  # send new frame
            sender = []
        else:
            win_start = int(msg)
            flag = 1  # send old frame

        print("************************************")
        print('Received ACK server: ' + data)  # show in terminal

        # again take input
        message = input(
            "Hit any key to start sending frames or enter stop to close connection -> ")

    client_socket.close()  # close the connection


if __name__ == '__main__':
    client_program()
