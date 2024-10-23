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

        self.bits_per_sample = 16

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

    def _create_wav_header(self):
        audio_duration = len(self.audio)
        riff = b"RIFF"  # 32 bytes
        chunk = (audio_duration + 36).to_bytes(4, "little")
        wavfmt = b"WAVEfmt "
        bits16 = (1).to_bytes(4, "little")  # b"\x10\x00\x00\x00"
        audio_format = (1).to_bytes(2, "little")  # b"\x01\x00"
        channel_bytes = self.channels.to_bytes(2, "little")
        sample_rate_bytes = self.sampling_rate.to_bytes(4, "little")
        byte_rate = (self.sampling_rate * self.channels * self.bits_per_sample / 8).to_bytes(4, "little")
        bytes_in_frame = (self.channels * self.bits_per_sample / 8).to_bytes(2, "little")
        bits_per_sample_bytes = self.bits_per_sample.to_bytes(2, "little")
        data_bytes = b"data"
        file_size = audio_duration.to_bytes(4, "little")

        header = (
            riff
            + chunk
            + wavfmt
            + bits16
            + audio_format
            + channel_bytes
            + sample_rate_bytes
            + byte_rate
            + bytes_in_frame
            + bits_per_sample_bytes
            + data_bytes
            + file_size
        )

        return header

    def load_format(self, format: str):
        if format == "mulaw":
            return audioop.lin2ulaw(self.audio, self.width)
        if format == "raw":
            return self.audio
        if format == "array":
            return self.get_array()
        if format == "wav":
            return b"".join([self._create_wav_header(), self.audio])
