from typing import Optional, Union

from audio._core.fragment import AudioFragment as _AudioFragment
import audio.op as audioop


class AudioFragment(_AudioFragment):
    def __init__(
        self,
        audio: Union[str, bytes] = None,
        sampling_rate: Optional[int] = None,
        channels: int = 1,
        width: int = 2,
    ):
        if isinstance(audio, str):
            super().__init__(audio)
        elif isinstance(audio, bytes) and not sampling_rate:
            super().__init__(audio_buffer=audio)
        else:
            super().__init__(
                audio_buffer=audio,
                sampling_rate=sampling_rate,
                width=width,
                channels=channels,
            )

        self.sampling_rate = self.get_sampling_rate()
        self.channels = self.get_channels()
        self.width = self.get_width()

    @classmethod
    def load_format(cls, audio: Union[str, bytes], format: str):
        if format == "mulaw":
            audio_bytes = audioop.ulaw2lin(audio, 2)
        return cls(audio=audio_bytes, sampling_rate=8000, channels=1, width=2)

    def to(self, format: str):
        if format == "raw":
            return self._get_raw_bytes()
        if format == "riff":
            return b"".join([self._create_riff_header(), self._get_raw_bytes()])
        if format == "mulaw":
            audio_raw = self._get_raw_bytes()
            audio_downsample = audioop.ratecv(
                audio_raw,
                width=2,
                nchannels=1,
                inrate=self.sampling_rate,
                outrate=8000,
                state=None,
            )
            return audioop.lin2ulaw(audio_downsample, 2)
