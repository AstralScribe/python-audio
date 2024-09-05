from typing import Optional
import _audioop

def add(fragment1: bytes, fragment2: bytes, width: int) -> bytes:
    return _audioop._add(fragment1, fragment2, width)

def adpcm2lin(fragment: bytes, width: int, state: Optional[tuple] = None) -> tuple:
    raise NotImplementedError("Still in development.")

def alaw2lin(fragment: bytes, width: int) -> bytes:
    return _audioop._alaw2lin(fragment, width)
    
def avg(fragment: bytes, width: int) -> int:
    return _audioop._avg(fragment, width)

def avgpp(fragment: bytes, width: int) -> int:
    return _audioop._avgpp(fragment, width)

def bias(fragment: bytes, width: int, bias: int) -> bytes:
    return _audioop._bias(fragment, width, bias)

def byteswap(fragment: bytes, width: int) -> bytes:
    return _audioop._byteswap(fragment, width)
    
def cross(fragment: bytes, width: int) -> int:
    return _audioop._cross(fragment, width)

def findfactor(fragment: bytes, reference: bytes) -> float:
    return _audioop._findfactor(fragment, reference)

def findfit(fragment: bytes, reference: bytes) -> tuple[int, float]:
    return _audioop._findfit(fragment, reference)

def findmax(fragment: bytes, length: int) -> int:
    return _audioop._findmax(fragment, length)

def getsample(fragment: bytes, width: int, index: int) -> int:
    return _audioop._getsample(fragment, width, index)

def lin2adpcm(fragment: bytes, width: int, state: Optional[tuple] = None) -> tuple:
    raise NotImplementedError("Still in development._")
    # return lin2adpcm(fragment, width, state)

def lin2alaw(fragment: bytes, width: int) -> bytes:
    return _audioop._lin2alaw(fragment, width)

def lin2lin(fragment: bytes, width: int, newwidth: int) -> bytes:
    return _audioop._lin2lin(fragment, width, newwidth)

def lin2ulaw(fragment: bytes, width: int) -> bytes:
    return _audioop._lin2ulaw(fragment, width)

def max(fragment: bytes, width: int) -> int:
    return _audioop._max(fragment, width)

def maxpp(fragment: bytes, width: int) -> int:
    return _audioop._maxpp(fragment, width)
    
def minmax(fragment: bytes, width: int) -> tuple[int, int]:
    return _audioop._minmax(fragment, width)

def mul(fragment: bytes, width: int, factor: float) -> bytes:
    return _audioop._mul(fragment, width, factor)

def ratecv(fragment: bytes, width: int, nchannels: int, inrate: int, outrate: int, state: Optional[tuple] = None, weightA: int = 1, weightB: int = 0) -> tuple:
    raise NotImplementedError("Still in development._")

def reverse(fragment: bytes, width: int) -> bytes:
    return _audioop._reverse(fragment, width)

def rms(fragment: bytes, width: int) -> int:
    return _audioop._rms(fragment, width)

def tomono(fragment: bytes, width: int, lfactor: float, rfactor: float) -> bytes:
    return _audioop._tomono(fragment, width, lfactor, rfactor)

def tostereo(fragment: bytes, width: int, lfactor: float, rfactor: float) -> bytes:
    return _audioop._tostereo(fragment, width, lfactor, rfactor)

def ulaw2lin(fragment: bytes, width: int) -> bytes:
    return _audioop._ulaw2lin(fragment, width)

