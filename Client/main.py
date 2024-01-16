import tkinter as tk
import ipaddress
from tkinter import ttk
from tkinter import messagebox
from GuiInterface import GuiInterface
from CommunicationHandlerer import CommunicationHandlerer
import socket
import re


class MyApp(tk.Tk, GuiInterface):

    def __init__(self):
        super().__init__()
        self.title("Hangman Client")
        self.geometry("900x600")

        # Create a container frame to hold the scenes
        self.container = ttk.Frame(self)
        self.container.pack(padx=20, pady=100)

        self.container.grid_rowconfigure(0, weight=1)
        self.container.grid_columnconfigure(0, weight=1)
        self.container.grid_rowconfigure(2, weight=1)
        self.container.grid_columnconfigure(2, weight=1)

        self.com_handlerer = CommunicationHandlerer(self)
        self.com_handlerer.start()

        self.protocol("WM_DELETE_WINDOW", self.on_closing)

        # labels
        self.scenes = {}
        self.client_name_labels = {}
        self.client_hp_labels = {}
        self.client_connection_labels = {}

        self.client_name_game_labels = {}
        self.client_hp_game_labels = {}
        self.client_connection_game_labels = {}
        self.client_turn_game_labels = {}
        self.start_button = None
        self.quess_entry = None
        self.quess_button = None
        self.word_label = None

        self.create_scene_lobby()
        self.create_scene_game()
        self.create_scene_join()
        self.create_scene_temp_dis()
        self.create_scene_login()

        self.current_scene = None

        # Initialize with Scene 1
        self.switch_to_scene("login_frame")

    def on_closing(self):
        """Method to handle the closing event."""
        print("Application is closing...")

        # Stop the CommunicationHandlerer thread
        self.com_handlerer.stop()

        # Destroy the Tkinter window
        self.destroy()

    def is_valid_ip(self, ip_str, port):

        try:
            if not port.isdigit():
                return False
            port_num = int(port)

            if not 0 < port_num < 65535:
                return False
            ipaddress.ip_address(ip_str)
            return True
        except ValueError:
            return False

    def connect_to_server(self, name,ip_address, port):

        try:
            ip_address = socket.gethostbyname(ip_address)
        except socket.error as e:
            messagebox.showinfo("Alert", "Wrong ip address or port format!")
            return

        if self.is_valid_ip(ip_address, port):
            pattern = r"^[a-zA-Z]{1,15}$"
            if re.match(pattern, name):
                connect_msg = "CONNECT|" + ip_address + "|" + port + "|" + name
                print(connect_msg)
                self.com_handlerer.send_instruction(connect_msg)
            else:
                messagebox.showinfo("Alert", "Wrong name format, [a-zA-Z]{1,15}$ !")
        else:

            messagebox.showinfo("Alert", "Wrong ip address or port format!")

    def join_game(self):
        join_msg = "JOIN"
        self.com_handlerer.send_instruction(join_msg)

    def start_game(self):
        start_msg = "START"
        self.com_handlerer.send_instruction(start_msg)

    def quess(self):

        quessed_word = self.quess_entry.get().upper()

        if re.match(r'^[A-Z]$', quessed_word):
            quess_msg = "QUESS|" + quessed_word
            self.com_handlerer.send_instruction(quess_msg)
        else:
            self.alert_popup("Wrong quess format - only one char A-Z")

        self.quess_entry.delete(0, "end")

    def alert_popup(self, alert_msg):
        messagebox.showinfo("Alert", alert_msg)

    def set_start_button(self, show):

        if show:
            self.start_button.grid(row=2, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        else:
            self.start_button.grid_forget()

    def set_quess_entry(self, show):
        if show:
            self.quess_entry.grid(row=5, column=2, sticky=(tk.W, tk.E, tk.N, tk.S))
            self.quess_button.grid(row=6, column=2, sticky=(tk.W, tk.E, tk.N, tk.S))
        else:
            self.quess_entry.grid_forget()
            self.quess_button.grid_forget()

    def create_scene_login(self):
        """Creates login_frame scene"""
        # Create a frame to hold the input widgets
        login_frame = ttk.Frame(self.container)

        username_label = ttk.Label(login_frame, text="Username:")
        username_label.pack(padx=10, pady=10)
        username_entry = ttk.Entry(login_frame, width=30)
        username_entry.pack(padx=10, pady=10)

        # IP Address Input
        ip_label = ttk.Label(login_frame, text="IP Address:")
        ip_label.pack(pady=10)
        ip_entry = ttk.Entry(login_frame, width=30)
        ip_entry.pack(pady=10)

        # Port Number Input
        port_label = ttk.Label(login_frame, text="Port Number:")
        port_label.pack(pady=10)
        port_entry = ttk.Entry(login_frame, width=30)
        port_entry.pack(pady=10)

        # Connect Button
        connect_button = tk.Button(login_frame,
                                   background="#03b1fc",
                                   activebackground="#3262a8",
                                   text="Connect",
                                   width=15,
                                   height=2,
                                   command=lambda: self.connect_to_server(username_entry.get(), ip_entry.get(),
                                                                          port_entry.get()))

        #connect_button.grid(row=3, columnspan=2, pady=10)
        connect_button.pack(pady=10)



        self.scenes["login_frame"] = login_frame

    def create_scene_join(self):
        """Creates login_frame scene"""
        # Create a frame to hold the input widgets
        join_frame = ttk.Frame(self.container)

        # Label with blue background
        label_join = ttk.Label(join_frame, text="Join game", foreground="black", font=("Arial", 16))
        label_join.pack(pady=10)

        # Button with "Join" text
        join_button = ttk.Button(join_frame, text="Join", command=self.join_game)
        join_button.pack(pady=10)

        self.scenes["join_frame"] = join_frame

    def create_scene_temp_dis(self):
        """Creates login_frame scene"""
        # Create a frame to hold the input widgets
        temp_dis_frame = ttk.Frame(self.container)

        # Label with blue background
        temp_dis_label_join = ttk.Label(temp_dis_frame, text="Server is not Reachable", foreground="black", font=("Arial", 16))
        temp_dis_label_join.pack(pady=10)
        temp_dis_label_join2 = ttk.Label(temp_dis_frame, text="Trying to reconnect", foreground="black",
                                        font=("Arial", 16))
        temp_dis_label_join2.pack(pady=10)

        self.scenes["temp_dis_frame"] = temp_dis_frame

    def switch_to_scene(self, scene_name):
        """Switches scenes"""
        if self.current_scene:
            self.current_scene.grid_forget()  # Hide the current scene
        self.current_scene = self.scenes[scene_name]
        self.current_scene.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

    def create_scene_lobby(self):
        """Creates login_frame scene"""
        # Create a frame to hold the input widgets
        lobby_frame = ttk.Frame(self.container)
        style = ttk.Style()
        style.configure("ClientFrame.TFrame", background="#325aa8")
        client_frame = ttk.Frame(lobby_frame, style="ClientFrame.TFrame")
        client_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        for i in range(4):
            self.client_name_labels[i] = ttk.Label(client_frame, text="NO_CLIENT", foreground="black", font=("Arial", 16))
            self.client_name_labels[i].grid(row=0, column=i, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)

        for i in range(4):
            self.client_hp_labels[i] = ttk.Label(client_frame, text="HP: -", foreground="black", font=("Arial", 16))
            self.client_hp_labels[i].grid(row=1, column=i, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)

        for i in range(4):
            self.client_connection_labels[i] = ttk.Label(client_frame, text="NOT CONNECTED", foreground="black", font=("Arial", 16))
            self.client_connection_labels[i].grid(row=2, column=i, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)

        # Button with "Join" text
        self.start_button = tk.Button(lobby_frame, background="#03b1fc", activebackground="#3262a8", width=15, height=2,
                                      text="Start", command=self.start_game)

        self.scenes["lobby_frame"] = lobby_frame

    def create_scene_game(self):
        """Creates login_frame scene"""
        # Create a frame to hold the input widgets
        game_frame = ttk.Frame(self.container)

        client_game_frame = ttk.Frame(game_frame)
        client_game_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        for i in range(4):
            self.client_name_game_labels[i] = ttk.Label(client_game_frame, text="NO_CLIENT", foreground="black", font=("Arial", 16))
            if i > 1:
                self.client_name_game_labels[i].grid(row=0, column=i + 1, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)
            else:
                self.client_name_game_labels[i].grid(row=0, column=i, columnspan=1, padx=10, pady=10,
                                                     sticky=tk.W + tk.E)

        for i in range(4):
            self.client_hp_game_labels[i] = ttk.Label(client_game_frame, text="HP: -", foreground="black", font=("Arial", 16))
            if i > 1:
                self.client_hp_game_labels[i].grid(row=1, column=i + 1, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)
            else:
                self.client_hp_game_labels[i].grid(row=1, column=i, columnspan=1, padx=10, pady=10,
                                                   sticky=tk.W + tk.E)

        for i in range(4):
            self.client_connection_game_labels[i] = ttk.Label(client_game_frame, text="NOT CONNECTED", foreground="black", font=("Arial", 16))
            if i > 1:
                self.client_connection_game_labels[i].grid(row=2, column=i+1, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)
            else:
                self.client_connection_game_labels[i].grid(row=2, column=i, columnspan=1, padx=10, pady=10,
                                                           sticky=tk.W + tk.E)

        for i in range(4):

            self.client_turn_game_labels[i] = ttk.Label(client_game_frame, text="WAITING", foreground="black", font=("Arial", 16))
            if i > 1:
                self.client_turn_game_labels[i].grid(row=3, column=i+1, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)
            else:
                self.client_turn_game_labels[i].grid(row=3, column=i, columnspan=1, padx=10, pady=10,
                                                     sticky=tk.W + tk.E)

        self.word_label = ttk.Label(client_game_frame, text="_____", foreground="black", font=("Arial", 30))
        self.word_label.grid(row=4, column=2, columnspan=1, padx=10, pady=10, sticky=tk.W + tk.E)

        self.quess_entry = ttk.Entry(client_game_frame, width=30)
        self.quess_button = tk.Button(client_game_frame, background="#03b1fc", activebackground="#3262a8", width=15, height=2,
                                      text="Quess", command=self.quess)

        self.scenes["game_frame"] = game_frame

    def switch_to_scene(self, scene_name):
        print("Switching scene to: " + scene_name)

        if self.current_scene:
            self.current_scene.pack_forget()  # Hide the current scene
        self.current_scene = self.scenes[scene_name]
        self.current_scene.pack(fill=tk.BOTH, expand=True)

    def reset_client_lablet(self):
        for i in range(0, 4):
            self.client_name_labels[i].config(text="NO_CLIENT")
            self.client_hp_labels[i].config(text="HP: -")
            self.client_connection_labels[i].config(text="NOT CONNECTED")

    def reset_client_game_lablet(self):
        for i in range(0, 4):
            self.client_name_game_labels[i].config(text="NO_CLIENT")
            self.client_hp_game_labels[i].config(text="HP: -")
            self.client_connection_game_labels[i].config(text="NOT CONNECTED")
            self.client_turn_game_labels[i].config(text="WAITING")

    def update_client_lables(self, index, name, hp, connection):

        hp_string = "HP: -"

        print(connection)

        if hp.isdigit():
            hp_string = "HP: " + str(hp)

        self.client_name_labels[index].config(text=name)
        self.client_hp_labels[index].config(text=hp_string)
        self.client_connection_labels[index].config(text=connection)

    def update_client_game_lables(self, index, name, hp, connection, on_turn):

        hp_string = "HP: " + str(hp)

        if on_turn:
            self.client_turn_game_labels[index].config(text="ON TURN")
        else:
            self.client_turn_game_labels[index].config(text="WAITING")

        self.client_name_game_labels[index].config(text=name)
        self.client_hp_game_labels[index].config(text=hp_string)
        self.client_connection_game_labels[index].config(text=connection)

    def update_client_on_turn(self, old_name_index, old_hp, new_name_index):
        self.client_hp_game_labels[old_name_index].config(text=old_hp)
        self.client_turn_game_labels[old_name_index].config(text="WAITING")
        self.client_turn_game_labels[new_name_index].config(text="ON TURN")

    def update_word_lable(self, updated_word):

        new_word = updated_word[0]
        for char in range(1, len(updated_word)):
            new_word = new_word + " " + updated_word[char]

        self.word_label.config(text=new_word)


if __name__ == "__main__":
    app = MyApp()
    app.mainloop()
