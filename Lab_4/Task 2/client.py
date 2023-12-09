import sys
import select
import socket
import getpass
from datetime import datetime


RED = "\033[91m"
GREEN = "\033[92m"
YELLOW = "\033[93m"
BLUE = "\033[94m"
PURPLE = "\033[95m"
CYAN = "\033[96m"
RESET = "\033[0;0m"
BOLD = "\033[;1m"


class ChatClient():
    def __init__(self, host="localhost", port=5000):
        self.new_socket = None
        self.HOST = host
        self.PORT = port
        self.RECV_BUFFER = 4096
        self.username = ""
        self.group = "default"
        self.separator = "&&&"  # same as of server
        self.helpMsg = """{}\
            LIST  => to get list of people online\n\
            LEAVE  => to leave the chat room\n\
            LOGOUT  => logout.{}""".format(PURPLE, RESET)

    def prompt(self):
        # prompt for current active user to type message
        sys.stdout.flush()
        sys.stdout.write("\r\033[K")
        sys.stdout.write(BOLD + GREEN + self.username + "> " +
                         "[" + datetime.now().strftime("%H:%M:%S") + "] " + RESET)
        sys.stdout.flush()

    def init(self):
        self.new_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.new_socket.settimeout(2)

        try:
            self.new_socket.connect((self.HOST, self.PORT))
        except:
            print('Unable to connect')
            sys.exit()

        cmd = int(input("Press 1 to register\nPress 2 to login\nCommand: "))
        if(cmd == 1):
            # register user
            username = input("Enter username: ")
            password = getpass.getpass("Enter password: ")
            self.new_socket.send(("REGISTER"+self.separator +
                                  username+self.separator+password).encode())
            response = self.new_socket.recv(self.RECV_BUFFER).decode()
            if("SERVER_FAIL"+self.separator) in response:
                print("Username already exists.")
                sys.exit()
            else:
                self.username = username
                print("Registered successfully.")

        elif(cmd == 2):
            # login user
            username = input("Enter username: ")
            password = getpass.getpass("Enter password: ")
            self.new_socket.send(("LOGIN"+self.separator+username +
                                  self.separator+password).encode())
            response = self.new_socket.recv(self.RECV_BUFFER).decode()
            if("SERVER_FAIL"+self.separator) in response:
                print("Invalid username or password.")
                sys.exit()
            else:
                self.username = username
                print("Logged In successfully.")

        else:
            print("Error: unknown command!")
            sys.exit()

        self.joinroom()

    def printList(self, msg, response):
        sys.stdout.write("\r\033[K")
        sys.stdout.flush()
        print(YELLOW + "<---- " + msg + " ---->", RESET)
        for p in response.split("::"):
            print(GREEN + "*" + RESET, p)
        print()

    def joinroom(self):
        self.new_socket.send("okk".encode())
        # set chat room
        chat_rooms = self.new_socket.recv(self.RECV_BUFFER).decode()
        self.printList("CHAT ROOMS", chat_rooms)
        groupName = input("{}Join a Chat Room or Create New: {}".format(
            PURPLE, YELLOW)).replace(" ", "")
        if groupName != "":
            self.group = groupName
        self.new_socket.send((self.group).encode())
        if(groupName == "LOGOUT"):
            print("Bye,", self.username)
            sys.exit()

        print("{}Joined {}{}{} group as {}{}{}".format(PURPLE, YELLOW, self.group,
                                                       PURPLE, YELLOW, self.username, RESET))
        self.new_socket.send(self.username.encode())

        firstConnResponse = self.new_socket.recv(self.RECV_BUFFER)
        firstConnResponse = firstConnResponse.decode()
        print("{}{}INFO>{} Connected to host. Start sending messages.".format(
            BOLD, YELLOW, RESET))
        print(self.helpMsg)

        while True:
            self.prompt()
            socket_list = [sys.stdin, self.new_socket]

            # get the list sockets which are readable
            read_sockets, write_sockets, error_sockets = select.select(
                socket_list, [], [])

            for sock in read_sockets:
                # handle incoming message from remote server
                if sock == self.new_socket:
                    data = sock.recv(self.RECV_BUFFER).decode()
                    if not data:
                        sys.stdout.write(BOLD + RED)
                        sys.stdout.write('Disconnected from chat server')
                        sys.stdout.write(RESET)
                        sys.exit()
                    else:
                        # receive user messages
                        # clears self stdin (bug like thingy)
                        try:
                            dt = data.split(self.separator, 1)
                            name = dt[0]
                            msg = dt[1]
                            sys.stdout.write("\r\033[K")
                            sys.stdout.flush()
                            if name == "SERVER_INFO":
                                # information
                                sys.stdout.write(BOLD + YELLOW)
                                sys.stdout.write("INFO" + "> ")
                            else:
                                # normal message
                                sys.stdout.write(BOLD + CYAN)
                                sys.stdout.write(name + "> ")

                            sys.stdout.write(RESET)
                            sys.stdout.write(msg + "\n")
                        except:
                            # other wise show list of users online
                            # not to best way to handle responses
                            self.printList("PEOPLE ONLINE", data)
                # send message
                else:
                    msg = sys.stdin.readline().strip()
                    if msg == "LEAVE":
                        print("{}Left {}{}{} group {}".format(PURPLE, YELLOW, self.group,
                                                              PURPLE,  RESET))
                        self.new_socket.send(
                            (self.group + self.separator + msg).encode())
                        data = self.new_socket.recv(self.RECV_BUFFER).decode()
                        print(data)
                        self.joinroom()
                    elif len(msg) > 0:
                        self.new_socket.send(
                            (self.group + self.separator + "[" + datetime.now().strftime("%H:%M:%S") + "] "+msg).encode())


def main():
    client = ChatClient()
    client.init()


if __name__ == '__main__':
    main()
