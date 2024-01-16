import threading
import time
import queue
import socket
import select
import re
from GuiInterface import GuiInterface
from Game import Game


class CommunicationHandlerer:
    def __init__(self, app: GuiInterface):
        self.input_queue = queue.Queue()
        self.output_queue = queue.Queue()
        self._thread = None
        self.app = app
        self.client_socket = None
        self.connected = False
        self.client_state = "UNLOGGED"
        self._stop_event = threading.Event()
        self.number_of_wrong_msg = 0
        self.number_of_clients = 0

        self.client_on_turn = None

        self.name = None
        self.game = None

        self.ping_interval = 100
        self.max_missed_pings = 100

        self.got_server_ping = True
        self.missed_pings = 0

        self.got_starting_time = False
        self.start_time = None

        self.temp_disconnect = False
        self.ip = None
        self.port = None

    def start(self):
        """Start the CommunicationHandlerer thread."""
        print("starting thread: CommunicationHandlerer")
        self._thread = threading.Thread(target=self._run)
        self._thread.start()

    def stop(self):
        """Stops the CommunicationHandlerer thread."""
        self._stop_event.set()
        if self._thread:
            self._thread.join()

    def send_instruction(self, instruction):
        """Send an instruction to the CommunicationHandlerer thread."""
        self.input_queue.put(instruction)

    def get_instruction(self):
        """Retrieve a instruction from the CommunicationHandlerer thread."""
        try:
            return self.output_queue.get_nowait()
        except queue.Empty:
            return None

    def send_msg_to_server(self, msg):
        """Sends msg to the server"""
        message_bytes = msg.encode('utf-8')
        try:
            self.client_socket.send(message_bytes)
        except (socket.error, ConnectionError, ConnectionAbortedError, Exception) as e:
            print(f"Error sending message: {e}")
            self.temp_disconnect_from_server()


    def wrong_server_msg(self):
        """Creates error pop up and checks if number of wrong msgs from server is higher than set threshold, if yes
         than it disconnects client from the server"""
        #self.app.alert_popup("WRONG MSG FROM SERVER!")
        popup_thread = threading.Thread(target=self.app.alert_popup, args=("WRONG MSG FROM SERVER!",))
        popup_thread.start()
        self.number_of_wrong_msg += 1

        if self.number_of_wrong_msg == 3:
            self.disconnect_from_server()

    def change_client_on_turn(self, old_client_on_turn_name, old_hp, client_on_turn_name):
        """Updates client on turn in gui"""
        index_new = self.game.clients.index(client_on_turn_name)
        index_old = self.game.clients.index(old_client_on_turn_name)

        if self.name == client_on_turn_name:
            self.app.set_quess_entry(True)
        else:
            self.app.set_quess_entry(False)

        self.app.update_client_on_turn(index_old, old_hp, index_new)

    def establish_connection(self, ip_address, port_number, name):
        """Connects to the server"""
        try:

            # Create a socket object
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            # Attempt to connect to the server
            self.client_socket.connect((ip_address, int(port_number)))

            print(f"Successfully connected to server at {ip_address}:{port_number}")
            self.connected = True
            message_to_send = "LOGIN|" + name
            message_bytes = message_to_send.encode('utf-8')
            self.client_socket.send(message_bytes)

            self.name = name

            return 1

        except Exception as e:
            print(f"Error connecting to server: {e}")
            return -1
            # Handle error (e.g., display an error message)

    def temp_disconnect_from_server(self):
        """Temporarily disconnects from the server and tries to reconnect"""
        if self.client_socket:
            try:
                self.client_socket.shutdown(socket.SHUT_RDWR)
                self.client_socket.close()
                print("Disconnected from the server.")
            except Exception as e:
                print(f"Error while disconnecting: {e}")
            finally:
                self.connected = False
                self.client_socket = None
                self.got_starting_time = False
                self.client_state = "UNLOGGED"
                self.app.switch_to_scene("temp_dis_frame")

                for i in range(0, self.max_missed_pings):
                    result = self.establish_connection(self.ip, self.port, self.name)
                    if result == 1:
                        return
                    else:
                        time.sleep(self.ping_interval)

                self.app.switch_to_scene("login_frame")

    def disconnect_from_server(self):
        """Disconnects from the server."""
        if self.client_socket:
            try:
                self.client_socket.shutdown(socket.SHUT_RDWR)
                self.client_socket.close()
                print("Disconnected from the server.")
            except Exception as e:
                print(f"Error while disconnecting: {e}")
            finally:
                self.connected = False
                self.client_state = "UNLOGGED"
                self.got_starting_time = False
                self.client_socket = None
                self.number_of_wrong_msg = 0
                self.app.switch_to_scene("login_frame")

    def handle_gui_msg(self, gui_msg):
        """Handles msg from gui thread"""

        msg_tokens = gui_msg.split('|')

        if msg_tokens[0] == "CONNECT":
            self.app.switch_to_scene("login_frame")
            self.ip = msg_tokens[1]
            self.port = msg_tokens[2]
            self.establish_connection(msg_tokens[1], msg_tokens[2], msg_tokens[3])
            return
        elif msg_tokens[0] == "JOIN":
            self.send_msg_to_server("JOIN")
        elif msg_tokens[0] == "START":
            self.send_msg_to_server("START")
        elif msg_tokens[0] == "QUESS":
            self.send_msg_to_server(gui_msg)


    def _run(self):
        """Method that gets executed in the thread."""

        #TODO

        try:
            while not self._stop_event.is_set():

                # ---------------------
                # REACT TO MSG FROM SERVER
                # ---------------------
                if self.connected:

                    if not self.got_starting_time:
                        self.got_starting_time = True
                        self.start_time = time.time()

                    # Create lists of sockets to monitor for read and write events
                    read_sockets = [self.client_socket]
                    write_sockets = []
                    error_sockets = [self.client_socket]

                    readable, writable, exceptional = select.select(read_sockets, write_sockets, error_sockets, 0)

                    for sock in readable:
                        if sock == self.client_socket:
                            # Receive data from the server

                            try:
                                data = self.client_socket.recv(2048)
                            except (socket.error, ConnectionAbortedError) as e:
                                print(f"Error receiving messages: {e}")
                                popup_thread = threading.Thread(target=self.app.alert_popup,
                                                                args=("Server is OFFLINE",))
                                popup_thread.start()
                                self.disconnect_from_server()

                            if len(data) > 2048:
                                self.wrong_server_msg()
                                self.disconnect_from_server()

                            #self.handleServer_Msg(data.decode())

                            try:
                                decoded_msg = data.decode('utf-8')
                            except UnicodeDecodeError:
                                print("Received message is not UTF-8")
                                self.disconnect_from_server()
                                break

                            self.handleServer_Msg(decoded_msg)

                            if not data:
                                break

                            #TODO

                            # Process the received data (e.g., display it in the GUI)
                            print(f"Received message: {data.decode()}")

                    time_now = time.time()

                    if time_now - self.start_time >= self.ping_interval + 1:

                        print("Check server connection")

                        if self.got_server_ping:
                            self.missed_pings = 0
                            self.got_server_ping = False
                            self.start_time = time.time()
                        else:
                            self.got_server_ping = False
                            self.missed_pings += 1
                            self.start_time = time.time()

                            if self.missed_pings == 1:

                                if self.client_state == "GAME" or self.client_state == "LOBBY":
                                    self.temp_disconnect_from_server()
                                else:
                                    self.disconnect_from_server()

                # ---------------------
                # REACT TO MSG FROM GUI
                # ---------------------
                try:

                    instruction = self.input_queue.get(timeout=0.2)
                    # Process the instruction here

                    self.handle_gui_msg(instruction)
                    print(f"Worker received instruction: {instruction}")

                except queue.Empty:
                    pass  # Continue processing or wait for the next instruction
                #print("Worker is running...")
                #time.sleep(1)

        except Exception as e:
            print(f"Error receiving messages: {e}")
            # Handle error (e.g., display an error message)

    def handleServer_Msg(self, msg):
        """Handles msg from the server."""

        msgs = msg.split("\n")

       # print(str(len(msgs)) + "xd")

        for index in range(0, len(msgs)-1):

            message = msgs[index]
            message = message.strip()
            msg_tokens = message.split("|")

            print(repr(message))

            if message.startswith("ACCEPT|1|"):

                if self.client_state != "UNLOGGED":
                    self.wrong_server_msg()
                    return

                if len(msg_tokens) != 4:
                    self.wrong_server_msg()
                    self.disconnect_from_server()
                    return

                if not msg_tokens[2].isdigit() or int(msg_tokens[2]) <= 0:
                    self.wrong_server_msg()
                    print("Wrong ping format")
                    self.disconnect_from_server()
                    return

                if not msg_tokens[3].isdigit() or int(msg_tokens[3]) < 0:
                    self.wrong_server_msg()
                    print("Wrong max missed pings format")
                    self.disconnect_from_server()
                    return

                self.ping_interval = int(msg_tokens[2])
                self.max_missed_pings = int(msg_tokens[3])

                print("LOGGED")
                self.client_state = "LOGGED"
                self.app.switch_to_scene("join_frame")

            elif message == "ERROR|1|1":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|1|1, wrong client state for msg LOGIN",))
                popup_thread.start()
                self.disconnect_from_server()
            elif message == "ERROR|1|2":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|1|2, wrong LOGIN msg format",))
                popup_thread.start()
                self.disconnect_from_server()
            elif message == "ERROR|1|3":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|1|3, invalid login name format",))
                popup_thread.start()
                self.disconnect_from_server()
            elif message == "ERROR|1|4":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|1|4, username is already taken",))
                popup_thread.start()
                self.disconnect_from_server()
            elif message == "ERROR|2|1":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|2|1 wrongclient state for msg JOIN",))
                popup_thread.start()
            elif message == "ERROR|2|2":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|2|2 wrong JOIN msg format",))
                popup_thread.start()
            elif message == "ERROR|3|1":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|3|1 wrongclient state for msg START",))
                popup_thread.start()
            elif message == "ERROR|3|2":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|3|2 wrong START msg format",))
                popup_thread.start()
            elif message == "ERROR|3|3":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|3|3 low number of clients in lobby for starting",))
                popup_thread.start()
            elif message == "ERROR|4|1":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|4|1 wrongclient state for msg QUESS",))
                popup_thread.start()
            elif message == "ERROR|4|2":
                popup_thread = threading.Thread(target=self.app.alert_popup, args=("ERROR|4|2 wrong QUESS msg format",))
                popup_thread.start()
            elif message == "PING":

                self.got_server_ping = True
                self.send_msg_to_server("PONG")

            elif message == "ACCEPT|2":

                if self.client_state != "LOGGED":
                    self.wrong_server_msg()
                    return

                self.client_state = "LOBBY"
                self.game = Game(self.name)
                self.app.switch_to_scene("lobby_frame")

            elif msg_tokens[0] == "LOBBYUPDATE":

                # WRONG SERVER MSG
                if len(msg_tokens) < 2 or len(msg_tokens) > 5 or self.client_state != "LOBBY":
                    self.wrong_server_msg()
                    return

                self.number_of_clients = 0
                updated_clients = []

                for i in range(1, len(msg_tokens)):

                    self.number_of_clients += 1

                    client_tokens = msg_tokens[i].split(";")

                    if len(client_tokens) != 2:
                        self.wrong_server_msg()
                        return
                    # CHECK NAME FORMAT
                    pattern = r"^[a-zA-Z]{1,15}$"
                    if not re.match(pattern, client_tokens[0]):
                        print(client_tokens[0])
                        self.wrong_server_msg()
                        return
                    # CHECK CONNECTION FORMAT
                    if not client_tokens[1].isdigit() or 0 > int(client_tokens[1]) > 1:
                        print(client_tokens[1])
                        self.wrong_server_msg()
                        return

                    if self.game.number_of_clients == len(msg_tokens)-1:
                        if client_tokens[0] not in self.game.clients:
                            print("[CommunicationHandlerer] Client" + client_tokens[0] + "from msg should not be in game")
                            self.wrong_server_msg()
                            return

                    updated_clients.append(client_tokens[0])

                if self.game.clien_name not in self.game.clients:
                    print("[CommunicationHandlerer] clients name is not in update msg")
                    self.wrong_server_msg()

                self.game.update_clients(updated_clients)
                self.app.reset_client_lablet()

                for i in range(1, len(msg_tokens)):

                    client_tokens = msg_tokens[i].split(";")

                    conn_state = "CONNECTED"
                    if client_tokens[1] == "1":
                        conn_state = "UNREACHABLE"

                    self.app.update_client_lables(i-1, client_tokens[0], "HP: -", conn_state)

                if self.number_of_clients >= 2:

                    self.app.set_start_button(True)
                else:
                    self.app.set_start_button(False)

            elif msg_tokens[0] == "STARTING":
                # CHECK CLIENT STATE
                if self.client_state != "LOBBY":
                    self.wrong_server_msg()
                    return
                # CHECK LENGHT OF MSG
                if len(msg_tokens) != 4:
                    self.wrong_server_msg()
                # CHECKING HP FORMAT
                if not msg_tokens[1].isdigit() or 1 > int(msg_tokens[1]) > 5:
                    print(msg_tokens[1])
                    self.wrong_server_msg()
                    return

                # CHECKING LENGHT OF WORD
                if not msg_tokens[2].isdigit() or 2 > int(msg_tokens[2]) > 15:
                    print(msg_tokens[2])
                    self.wrong_server_msg()
                    return

                pattern = r"^[a-zA-Z]{1,15}$"
                if not re.match(pattern, msg_tokens[3]):
                    print(msg_tokens[3])
                    self.wrong_server_msg()
                    return

                if msg_tokens[3] not in self.game.clients:
                    print("Client with name " + msg_tokens[3] + "is not in game")
                    self.wrong_server_msg()
                    return

                self.game.word_len = int(msg_tokens[2])
                self.game.word = ['_' for _ in range(self.game.word_len)]
                self.game.starting_hp = int(msg_tokens[1])
                self.app.switch_to_scene("game_frame")
                self.client_state = "GAME"

                for client_in_game in self.game.clients:

                    on_turn = False
                    if client_in_game == msg_tokens[3]:
                        on_turn = True

                    self.app.update_client_game_lables(self.game.clients.index(client_in_game), client_in_game, self.game.starting_hp, "CONNECTED", on_turn)
                    self.app.update_word_lable(self.game.word)

                if self.name == msg_tokens[3]:
                    self.app.set_quess_entry(True)
                else:
                    self.app.set_quess_entry(False)

            elif msg_tokens[0] == "CORRECTQUESS":
                # CHECK CLIENT STATE
                if self.client_state != "GAME":
                    print("Wrong client state")
                    self.wrong_server_msg()
                    return
                if len(msg_tokens) != 2:
                    print("Wrong msg format")
                    self.wrong_server_msg()
                    return

                quess_tokens = msg_tokens[1].split(";")

                if len(quess_tokens) < 2:
                    print("Wrong msg format. msg is to short")
                    self.wrong_server_msg()
                    return

                if not re.match(r'^[A-Z]$', quess_tokens[0]):
                    print("Wrong msg format. not a char")
                    self.wrong_server_msg()
                    return

                for quess_index in range (1,len(quess_tokens)):
                    if not quess_tokens[quess_index].isdigit():
                        print("Wrong msg format. Index is not a digit")
                        self.wrong_server_msg()
                        return
                    if int(quess_tokens[quess_index]) > len(self.game.word)-1:
                        print("Wrong msg format. Index is out of bounds: " + str(quess_index))
                        self.wrong_server_msg()
                        return

                for quess_index in range(1, len(quess_tokens)):
                    self.game.word[int(quess_tokens[quess_index])] = quess_tokens[0]

                self.app.update_word_lable(self.game.word)

            elif msg_tokens[0] == "BADQUESS":
                if self.client_state != "GAME":
                    print("Wrong client state")
                    self.wrong_server_msg()
                    return
                if len(msg_tokens) != 3:
                    print("Wrong msg format")
                    self.wrong_server_msg()
                    return

                bad_quess_tokens = msg_tokens[1].split(";")

                if len(bad_quess_tokens) != 2:
                    print("Wrong msg format")
                    self.wrong_server_msg()
                    return

                pattern = r"^[a-zA-Z]{1,15}$"
                if not re.match(pattern, bad_quess_tokens[0]):
                    print("wrong name format" + bad_quess_tokens[0])
                    self.wrong_server_msg()
                    return
                    # CHECKING HP FORMAT
                if not bad_quess_tokens[1].isdigit() or 1 > int(bad_quess_tokens[1]) > 5:
                    print("wrong hp format:" + bad_quess_tokens[1])
                    self.wrong_server_msg()
                    return

                if not re.match(pattern, msg_tokens[2]):
                    print("wrong name format" + msg_tokens[2])
                    self.wrong_server_msg()
                    return

                if bad_quess_tokens[0] not in self.game.clients:
                    print("client not in game" + bad_quess_tokens[0])
                    self.wrong_server_msg()
                    return
                if msg_tokens[2] not in self.game.clients:
                    print("client not in game" + msg_tokens[2])
                    self.wrong_server_msg()
                    return

                self.app.update_client_on_turn(self.game.clients.index(bad_quess_tokens[0]), bad_quess_tokens[1], self.game.clients.index(msg_tokens[2]))

                show = False

                if msg_tokens[2] == self.name:
                    show = True

                self.app.set_quess_entry(show)

            elif msg_tokens[0] == "WINNER":
                if self.client_state != "GAME":
                    print("Wrong client state")
                    self.wrong_server_msg()
                    return
                if len(msg_tokens) != 3:
                    print("Wrong msg format")
                    self.wrong_server_msg()
                    return
                pattern = r"^[a-zA-Z]{1,15}$"
                if not re.match(pattern, msg_tokens[1]):
                    print("wrong name format" + msg_tokens[1])
                    self.wrong_server_msg()
                    return
                if not re.match(pattern, msg_tokens[2]):
                    print("wrong word format" + msg_tokens[2])
                    self.wrong_server_msg()
                    return

                message = "WINNER: " + msg_tokens[1] + ", QUESSED WORD: " + msg_tokens[2]
                self.client_state = "LOBBY"

                popup_thread = threading.Thread(target=self.app.alert_popup, args=(message,))
                popup_thread.start()

                self.app.switch_to_scene("lobby_frame")

            elif msg_tokens[0] == "GAMEUPDATE":
                # WRONG SERVER MSG
                if len(msg_tokens) < 2 or len(msg_tokens) > 5:
                    self.wrong_server_msg()
                    return
                if self.client_state != "GAME":
                    print("Wrong client state")
                    self.wrong_server_msg()
                    return

                clients = msg_tokens[len(msg_tokens)-1].split("!")
                client_on_turn = clients[1]
                msg_tokens[len(msg_tokens) - 1] = clients[0]

                self.number_of_clients = 0
                updated_clients = []

                for i in range (1, len(msg_tokens)):

                    self.number_of_clients += 1
                    client_tokens = msg_tokens[i].split(";")

                    if len(client_tokens) != 3:
                        self.wrong_server_msg()
                        return
                    # CHECK NAME FORMAT
                    pattern = r"^[a-zA-Z]{1,15}$"
                    if not re.match(pattern, client_tokens[0]):
                        print(client_tokens[0])
                        self.wrong_server_msg()
                        return
                    # CHECK HP FORMAT
                    if not client_tokens[1].isdigit() or 0 > int(client_tokens[1]) > 5:
                        print(client_tokens[1])
                        self.wrong_server_msg()
                        return
                    # CHECK CONNECTION FORMAT
                    if not client_tokens[2].isdigit() or 0 > int(client_tokens[2]) > 1:
                        print(client_tokens[2])
                        self.wrong_server_msg()
                        return

                    if self.game.number_of_clients == len(msg_tokens)-1:
                        if client_tokens[0] not in self.game.clients:
                            print("[CommunicationHandlerer] Client" + client_tokens[0] + "from msg should not be in game")
                            self.wrong_server_msg()
                            return

                    updated_clients.append(client_tokens[0])

                if self.game.clien_name not in updated_clients:
                    print("[CommunicationHandlerer] clients name is not in update msg")
                    self.wrong_server_msg()

                self.game.update_clients(updated_clients)
                self.app.reset_client_game_lablet()

                for i in range(1, len(msg_tokens)):

                    client_tokens = msg_tokens[i].split(";")

                    conn_state = "CONNECTED"
                    if client_tokens[2] == "1":
                        conn_state = "UNREACHABLE"

                    on_turn = False
                    if client_on_turn == client_tokens[0]:
                        on_turn = True

                    self.app.update_client_game_lables(i-1, client_tokens[0], client_tokens[1], conn_state, on_turn)

                show = False

                if client_on_turn == self.name:
                    show = True

                self.app.set_quess_entry(show)
            elif msg_tokens[0] == "RECONNECT":

                if len(msg_tokens) != 4:
                    self.wrong_server_msg()
                    return
                if msg_tokens[1] != "GAME" and msg_tokens[1] != "LOBBY":
                    self.wrong_server_msg()
                    print("Wrong ping format")
                    return
                if not msg_tokens[2].isdigit() or int(msg_tokens[2]) <= 0:
                    self.wrong_server_msg()
                    print("Wrong ping format")
                    return

                if not msg_tokens[3].isdigit() or int(msg_tokens[3]) < 0:
                    self.wrong_server_msg()
                    print("Wrong max missed pings format")
                    return

                self.ping_interval = int(msg_tokens[2])
                self.max_missed_pings = int(msg_tokens[3])

                if msg_tokens[1] == "GAME":
                    self.client_state = "GAME"
                    self.game = Game(self.name)
                    self.app.switch_to_scene("game_frame")
                else:
                    self.client_state = "LOBBY"
                    self.game = Game(self.name)
                    self.app.switch_to_scene("lobby_frame")

            elif msg_tokens[0] == "QUESSED":

                if len(msg_tokens) < 2:
                    self.wrong_server_msg()
                if not msg_tokens[1].isdigit() or 2 > int(msg_tokens[1]) > 15:
                    print(msg_tokens[1])
                    self.wrong_server_msg()
                    return
                self.game.word_len = int(msg_tokens[1])
                self.game.word = ['_' for _ in range(self.game.word_len)]

                for y in range(2, len(msg_tokens)):
                    char_tokens = msg_tokens[y].split(";")
                    if len(char_tokens) < 2:
                        self.wrong_server_msg()

                    if not re.match(r'^[A-Z]$', char_tokens[0]):
                        print("Wrong msg format. not a char")
                        self.wrong_server_msg()
                        return

                    for x in range(1, len(char_tokens)):
                        if not char_tokens[x].isdigit() or 2 > int(char_tokens[x]) > 15:
                            self.wrong_server_msg()
                            return
                        self.game.word[int(char_tokens[x])] = char_tokens[0]

                self.app.update_word_lable(self.game.word)

            else:
                self.wrong_server_msg()

                if self.client_state == "UNLOGGED":
                    self.disconnect_from_server()
                    return

                if self.number_of_wrong_msg >= 3:
                    self.disconnect_from_server()
        return
