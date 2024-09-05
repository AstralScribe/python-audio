class Audio(object):
    def __init__(self):
        self.raw = None

    def load_file(self, file):
        with open(file, "rb") as audio_file:
            self._load_to_raw(audio_file.read())

    def _load_to_raw(self, data: bytes):






