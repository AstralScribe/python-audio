import audioop
from typing import List, Optional, Union
import wave
import numpy as np
import io

class Convert:
    # TODO: Write wav to ulaw
    # TODO: Write wav to mp3
    # TODO: Write mp3 to wav
    # TODO: Write ulaw to mp3
    # TODO: Write mp3 to ulaw
    
    @classmethod
    def ulaw_to_wav(cls, data: bytes, width: int = 2) -> bytes:
        return audioop.ulaw2lin(data, width)

    @classmethod
    def wav_to_ulaw(cls, data: Union[bytes, np.ndarray, List], sample_rate: Optional[int] = None, width: int = 2) -> bytes:
        header = False
        if isinstance(data, (np.ndarray, List)) and sample_rate is None:
            raise ValueError("Array or list requires sample rate to be specified")
        
        if isinstance(data, bytes):
            if header:
                pass
            else:
                return audioop.lin2ulaw(data, width)

    @classmethod
    def pcm_to_wav(cls, data: Union[bytes, np.ndarray, List], sample_rate: int, channels: int, nframes: int, width: int = 2) -> bytes:
        
        assert isinstance(data, (bytes, np.ndarray, List)), f"Wrong type for data. Expected bytes, array or list, but found {type(data)}"

        output = io.BytesIO()
        with wave.open(output, "wb") as wav_data:
            wav_data.setframerate(sample_rate)
            wav_data.setnchannels(channels)
            wav_data.setnframes(nframes)
            wav_data.setsampwidth(width)
            if isinstance(data, bytes):
                wav_data.writeframesraw(data)
            else:
                data = np.array(data)


        return output
