from typing import Optional, Union

# import warnings
from audio._core.fragment import AudioFragment as _AudioFragment
import audio.op as audioop


class AudioFragment(_AudioFragment):
    def __init__(
        self,
        audio: Union[str, bytes] = None,
        sampling_rate: Optional[int] = None,
        channels: Optional[int] = None,
        width: Optional[int] = None,
    ):
        if isinstance(audio, str):
            super().__init__(audio)
        elif isinstance(audio, bytes) and not any((sampling_rate, channels, width)):
            super().__init__(audio_buffer=audio)
        else:
            super().__init__(audio_buffer=audio, sampling_rate=sampling_rate, width=width, channels=channels)

    @classmethod
    def load_format(cls, audio: Union[str, bytes], format: str):
        if format == "mulaw":
            audio_bytes = audioop.ulaw2lin(audio, 2)
        return cls(audio=audio_bytes, sampling_rate=8000, channels=1, width=2)
