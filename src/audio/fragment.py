from typing import Optional
import warnings
import audio.op as audioop


class AudioFragment:
    def __init__(
        self,
        audio: bytes,
        sampling_rate: Optional[int] = None,
        channels: Optional[int] = None,
        width: Optional[int] = None,
    ):
        if sampling_rate is not None:
            self.sampling_rate = sampling_rate
        if channels is not None:
            self.channels = channels
        if width is not None:
            assert len(audio) % width == 0, "Number of audio frames should be int."
            self.width = width

        if self.audio.find("data") != -1:
            if sampling_rate is not None:
                warnings.warn("Audio has header attached. Resampling to provided audio header")
                resample = True
            else:
                resample = False

            self.load_audio_with_header(audio)

            if resample:
                self.audio_array = self.resample(self.audio_array, orig_sr=self.sampling_rate, target_sr=sampling_rate)

    @classmethod
    def load(cls, audio: bytes, format: str):
        return cls(audio)._load_from_format(format)

    def _load_from_format(self, format: str):
        if format == "mulaw":
            self.sampling_rate = 8000
            self.channels = 1
            self.width = 2
            self.audio = audioop.ulaw2lin(self.audio, self.width)

        return self

    def _create_wav_header(self): ...

    def load_format(self, format: str):
        if format == "mulaw":
            return audioop.lin2ulaw(self.audio, self.width)
        if format == "raw":
            return self.audio
        if format == "array":
            return self.get_array()
        if format == "wav":
            return b"".join([self._create_wav_header(), self.audio])
