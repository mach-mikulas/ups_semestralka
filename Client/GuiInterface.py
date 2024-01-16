
class GuiInterface:
    def switch_to_scene(self, scene_name):
        """Changes gui scenes"""
        raise NotImplementedError

    def alert_popup(self, alert_msg):
        """Creates gui pop up with set msg"""
        raise NotImplementedError

    def update_client_lables(self, index, name, hp, connection):
        """updates clients labels inside of a lobby"""
        raise NotImplementedError

    def reset_client_lablet(self):
        """Resets clients label to the default state"""
        raise NotImplementedError

    def set_start_button(self, show):
        """Shows or hides start button in the lobby"""
        raise NotImplementedError

    def update_client_game_lables(self, index, name, hp, connection, on_turn):
        """updates clients labels inside of a game"""
        raise NotImplementedError

    def update_word_lable(self, updated_word):
        """updates quessed word lable inside of the game"""
        raise NotImplementedError

    def update_client_on_turn(self, old_name_index, old_hp, new_name_index):
        """Changes who is on trun in gui"""
        raise NotImplementedError

    def set_quess_entry(self, show):
        """When client is on the turn shows quess entry box and button"""
        raise NotImplementedError

    def reset_client_game_lablet(self):
        """resets labels in game"""
        raise NotImplementedError

