
class Message:


    def __init__(self, message_id, to_client, from_client, message_type, content):

        if len(to_client) > 16 or len(from_client) > 16:
            raise ValueError("The destination or sander ID is too long")

        if not isinstance(message_id, int) or message_id > 0xFFFFFFFF:
            raise ValueError("The ID index is too long")

        if message_type > 0xFF:
            raise ValueError("The type message is too long")

        self._id = message_id
        self._to_client = to_client
        self._from_client = from_client
        self._type = message_type
        self._content = content


    def get_message_id(self):
        return self._id

    def get_to_client(self):
        return self._to_client

    def get_from_client(self):
        return self._from_client

    def get_message_type(self):
        return self._type

    def get_content(self):
        return self._content
