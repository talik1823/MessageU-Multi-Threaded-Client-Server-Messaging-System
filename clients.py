from datetime import datetime


class Clients:
                        #               # year # month # day # hour # minute #
                        #    dt = datetime(2026,    3,    5,    14,    30)

    def __init__(self, id, user_name, last_seen ):

        if len (id) <= 16:
            self._id = id
        else:
            raise ValueError("ID is too long")

        if len(user_name) <= 255:
            self._user_name = user_name
        else:
            raise ValueError("User Name is too long")

        if isinstance(last_seen, datetime):
            self._last_seen = last_seen
        else:
            raise TypeError("Last_Seen must be DateHeader")


    def client_id(self):
        return self._id

    def user_name(self):
        return self._user_name

    def last_seen(self):
        return  self._last_seen

    def update_last_seen(self, time):
        if isinstance(time,datetime):
            self._last_seen = time
        else:
            raise TypeError("Last_Seen must be DateHeader")

