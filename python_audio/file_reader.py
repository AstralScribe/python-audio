from typing import Union
import wave
import subprocess


class FileReader(object):

    @classmethod
    def read_file(cls, file: str, format: str) -> bytes:
        if format == "wav":
            with open(file, "rb") as f:
                return f.read()
        else:
            commands = []

