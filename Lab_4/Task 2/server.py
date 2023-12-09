import socket
import select


class ChatServer():
    def __init__(self, host="localhost", port=5000):
        self.network = {
            "default": {
                "connections": [],
                "names": []
            }
        }
        self.users = {}
        self.RECV_BUFFER = 4096
        self.HOST = host
        self.PORT = port
        self.server_socket = 0
        self.separator = "&&&"  # same as client

    # initialize server and handle requests
    def init(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(
            socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.HOST, self.PORT))
        self.server_socket.listen(2)

        # add server socket to the list of readable connections
        self.network["default"]["connections"].append(self.server_socket)
        self.network["default"]["names"].append("<server>")

        print("Running chat server at {}:{}".format(self.HOST, self.PORT))

        while True:
            # get the list all sockets which are ready to be read through select
            all_connections = []
            for key in self.network:
                all_connections += self.network[key]["connections"]
            read_sockets, write_sockets, error_sockets = select.select(
                all_connections, [], [])

            for sock in read_sockets:
                # new connection recieved through self.server_socket
                if sock == self.server_socket:
                    sockfd, addr = self.server_socket.accept()

                    try:
                        data = sockfd.recv(self.RECV_BUFFER).decode()
                    except:
                        continue

                    # register user
                    if ("REGISTER"+self.separator) in data:
                        username = data.split(self.separator)[1]
                        password = data.split(self.separator)[2]
                        if username in self.users:
                            sockfd.send(("SERVER_FAIL" + self.separator +
                                         "Username already exsts.").encode())
                        else:
                            self.users[username] = password
                            sockfd.send(("SUCCESS").encode())

                    # login user
                    elif("LOGIN"+self.separator) in data:
                        username = data.split(self.separator)[1]
                        password = data.split(self.separator)[2]
                        if username not in self.users:
                            sockfd.send(("SERVER_FAIL" + self.separator +
                                         "Invalid username or password.").encode())
                        elif self.users[username] != password:
                            sockfd.send(("SERVER_FAIL" + self.separator +
                                         "Invalid username or password.").encode())
                        else:
                            sockfd.send(("SUCCESS").encode())

                    # send list of available chat rooms to client
                    roomsList = ""
                    for group in self.network.keys():
                        numberOfMembersInRoom = len(
                            self.network[group]["connections"])
                        if group == "default":
                            numberOfMembersInRoom -= 1  # exclude server
                        roomsList += group + \
                            " <{}>::".format(numberOfMembersInRoom)
                    roomsList = roomsList[:-2]
                    try:
                        sockfd.recv(32).decode()
                    except:
                        continue

                    sockfd.send(roomsList.encode())
                    # client sends the chat room name he wants to join
                    group = sockfd.recv(32).decode()

                    if group == "":
                        continue
                    elif group not in self.network.keys():
                        # create a new group
                        self.network[group] = {
                            "connections": [],
                            "names": []
                        }

                    # client sends his name (identifier in group)
                    clientName = sockfd.recv(100).decode()
                    if clientName == "":
                        continue
                    elif clientName not in self.network[group]["names"]:
                        self.network[group]["names"].append(clientName)
                        self.network[group]["connections"].append(sockfd)
                        msg_notification = "{} joined room [{}] from <{}:{}>".format(
                            clientName, group, addr[0], addr[1])
                        print(msg_notification)
                        self.broadcast(
                            group, sockfd, msg_notification, isInfo=True)
                        sockfd.send(
                            ("SERVER_INFO" + self.separator + "Welcome.").encode())
                    else:
                        sockfd.send(("SERVER_FAIL" + self.separator +
                                    "Cannot have same name.").encode())

                # process incoming message from a client
                else:

                    try:
                        data = sock.recv(self.RECV_BUFFER).decode()
                        group = data.split(self.separator, 1)[0]
                        if "LIST" in data:
                            # if client asks for list of people online
                            self.sendList(group, sock)
                        elif "LEAVE" in data:
                            i = None
                            group = ""
                            for key in self.network:
                                if sock in self.network[key]["connections"]:
                                    i = self.network[key]["connections"].index(
                                        sock)
                                    group = key
                                    break
                            if (i != None and group != ""):
                                msg_notification = "{} left room [{}].".format(
                                    self.network[group]["names"][i], group)
                                self.broadcast(
                                    group, sock, msg_notification, isInfo=True)
                                print(msg_notification)
                                del self.network[group]["connections"][i]
                                del self.network[group]["names"][i]

                            # sock.send("hi".encode())
                            sock.send("hi".encode())
                            # send list of available chat rooms to client
                            roomsList = ""
                            for group in self.network.keys():
                                numberOfMembersInRoom = len(
                                    self.network[group]["connections"])
                                if group == "default":
                                    numberOfMembersInRoom -= 1  # exclude server
                                roomsList += group + \
                                    " <{}>::".format(numberOfMembersInRoom)
                            roomsList = roomsList[:-2]
                            try:
                                sock.recv(32).decode()
                            except:
                                continue

                            sock.send(roomsList.encode())
                            # client sends the chat room name he wants to join
                            group = sock.recv(32).decode()
                            if(group == "LOGOUT"):
                                i = None
                                group = ""
                                for key in self.network:
                                    if sock in self.network[key]["connections"]:
                                        i = self.network[key]["connections"].index(
                                            sock)
                                        group = key
                                        break
                                if (i != None and group != ""):
                                    msg_notification = "{} from [{}] <{}:{}> went offline.".format(
                                        self.network[group]["names"][i], group, addr[0], addr[1])
                                    self.broadcast(
                                        group, sock, msg_notification, isInfo=True)
                                    print(msg_notification)
                                    del self.network[group]["connections"][i]
                                    del self.network[group]["names"][i]
                                continue

                            if group == "":
                                continue
                            elif group not in self.network.keys():
                                # create a new group
                                self.network[group] = {
                                    "connections": [],
                                    "names": []
                                }

                            # client sends his name (identifier in group)
                            clientName = sock.recv(100).decode()
                            if clientName == "":
                                continue
                            elif clientName not in self.network[group]["names"]:
                                self.network[group]["names"].append(clientName)
                                self.network[group]["connections"].append(
                                    sock)
                                msg_notification = "{} joined room [{}] from <{}:{}>".format(
                                    clientName, group, addr[0], addr[1])
                                print(msg_notification)
                                self.broadcast(
                                    group, sock, msg_notification, isInfo=True)
                                sock.send(
                                    ("SERVER_INFO" + self.separator + "Welcome.").encode())
                            else:
                                sock.send(("SERVER_FAIL" + self.separator +
                                           "Cannot have same name.").encode())
                        elif len(data) > 0:
                            self.broadcast(group, sock, data.split(
                                self.separator, 1)[1])
                        else:
                            raise
                    except:
                        # like client pressed ctrl+c
                        # remove client from his group
                        i = None
                        group = ""
                        for key in self.network:
                            if sock in self.network[key]["connections"]:
                                i = self.network[key]["connections"].index(
                                    sock)
                                group = key
                                break
                        if (i != None and group != ""):
                            msg_notification = "{} from [{}] <{}:{}> went offline.".format(
                                self.network[group]["names"][i], group, addr[0], addr[1])
                            self.broadcast(
                                group, sock, msg_notification, isInfo=True)
                            print(msg_notification)
                            del self.network[group]["connections"][i]
                            del self.network[group]["names"][i]
                        continue

        self.server_socket.close()

    # broadcast chat messages to all connected clients, except sender
    def broadcast(self, group, sender, message, isInfo=False):
        i = self.network[group]["connections"].index(sender)
        sender_name = self.network[group]["names"][i]
        if isInfo:
            sender_name = "SERVER_INFO"

        recievers = self.network[group]["connections"]

        message = sender_name + self.separator + message
        for socket in recievers:
            if socket != self.server_socket and socket != sender:
                try:
                    socket.send(message.encode())
                except:
                    # might be broken socket connection
                    i = self.network[group]["connections"].index(socket)
                    print("removing", self.network[group]["names"][i])
                    del self.network[group]["connections"][i]
                    del self.network[group]["names"][i]
                    socket.close()

    # send list of online people to requestor
    def sendList(self, group, requestor):
        if group == "default":
            names = self.network[group]["names"][1:]
            connections = self.network[group]["connections"][1:]
        else:
            names = self.network[group]["names"]
            connections = self.network[group]["connections"]

        list_str = ""
        for name, addr in zip(names, connections):
            host, port = addr.getpeername()
            list_str += name + " <{}:{}>::".format(host, port)
        requestor.send(list_str[:-2].encode())  # remove end ::


def main():
    server = ChatServer()
    server.init()


if __name__ == '__main__':
    main()
