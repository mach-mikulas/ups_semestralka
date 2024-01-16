class Game:
    def __init__(self, name):
        self.clients = []
        self.number_of_clients = 0
        self.clien_name = name
        self.clients.append(name)
        self.word_len = 0
        self.starting_hp = None
        self.word = []


    def update_clients(self, clients):
        self.clients = clients
        self.number_of_clients = len(clients)
