from typing import Optional
import audio._core.audioop as _audioop


def add(fragment1: bytes, fragment2: bytes, width: int) -> bytes:
    """
    Return a fragment which is the addition of the two samples passed as parameters.
    width is the sample width in bytes, either 1, 2, 3 or 4.
    Both fragments should have the same length. Samples are truncated in case of overflow.
    """
    return _audioop.add(fragment1, fragment2, width)


def adpcm2lin(fragment: bytes, width: int, state: Optional[tuple] = None) -> tuple:
    """
    Decode an Intel/DVI ADPCM coded fragment to a linear fragment.
    See the description of lin2adpcm() for details on ADPCM coding.
    Return a tuple (sample, newstate) where the sample has the width specified in width.
    """
    raise NotImplementedError("Still in development.")


def alaw2lin(fragment: bytes, width: int) -> bytes:
    """
    Convert sound fragments in a-LAW encoding to linearly encoded sound fragments.
    a-LAW encoding always uses 8 bits samples, so width refers only to the sample width of the output fragment here.
    """
    return _audioop.alaw2lin(fragment, width)


def avg(fragment: bytes, width: int) -> int:
    """
    Return the average over all samples in the fragment.
    """
    return _audioop.avg(fragment, width)


def avgpp(fragment: bytes, width: int) -> int:
    """
    Return the average peak-peak value over all samples in the fragment.
    No filtering is done, so the usefulness of this routine is questionable.
    """
    return _audioop.avgpp(fragment, width)


def bias(fragment: bytes, width: int, bias: int) -> bytes:
    """
    Return a fragment that is the original fragment with a bias added to each sample.
    Samples wrap around in case of overflow.
    """
    return _audioop.bias(fragment, width, bias)


def byteswap(fragment: bytes, width: int) -> bytes:
    """
    “Byteswap” all samples in a fragment and returns the modified fragment.
    Converts big-endian samples to little-endian and vice versa.
    """
    return _audioop.byteswap(fragment, width)


def cross(fragment: bytes, width: int) -> int:
    """Return the number of zero crossings in the fragment passed as an argument."""
    return _audioop.cross(fragment, width)


def findfactor(fragment: bytes, reference: bytes) -> float:
    """
    Return a factor F such that rms(add(fragment, mul(reference, -F))) is minimal, i.e.,
    return the factor with which you should multiply reference to make it match as well as
    possible to fragment. The fragments should both contain 2-byte samples.

    The time taken by this routine is proportional to len(fragment).
    """
    return _audioop.findfactor(fragment, reference)


def findfit(fragment: bytes, reference: bytes) -> tuple[int, float]:
    """
    Try to match reference as well as possible to a portion of fragment
    (which should be the longer fragment). This is (conceptually) done by
    taking slices out of fragment, using findfactor() to compute the best match,
    and minimizing the result. The fragments should both contain 2-byte samples.

    Return a tuple (offset, factor) where offset is the (integer) offset
    into fragment where the optimal match started and factor is the (floating-point)
    factor as per findfactor().
    """
    return _audioop.findfit(fragment, reference)


def findmax(fragment: bytes, length: int) -> int:
    """
    Search fragment for a slice of length length samples (not bytes!)
    with maximum energy, i.e., return i for which rms(fragment[i*2:(i+length)*2]) is maximal.
    The fragments should both contain 2-byte samples.

    The routine takes time proportional to len(fragment).

    """
    return _audioop.findmax(fragment, length)


def getsample(fragment: bytes, width: int, index: int) -> int:
    """
    Return the value of sample index from the fragment.

    """
    return _audioop.getsample(fragment, width, index)


def lin2adpcm(fragment: bytes, width: int, state: Optional[tuple] = None) -> tuple:
    """
    Convert samples to 4 bit Intel/DVI ADPCM encoding.
    ADPCM coding is an adaptive coding scheme, whereby each 4 bit
    number is the difference between one sample and the next, divided by a (varying) step.
    The Intel/DVI ADPCM algorithm has been selected for use by the IMA,
    so it may well become a standard.

    state is a tuple containing the state of the coder.
    The coder returns a tuple (adpcmfrag, newstate), and the newstate should be passed to
    the next call of lin2adpcm(). In the initial call, None can be passed as the state.
    adpcmfrag is the ADPCM coded fragment packed 2 4-bit values per byte.
    """
    raise NotImplementedError("Still in development.")
    # return lin2adpcm(fragment, width, state)


def lin2alaw(fragment: bytes, width: int) -> bytes:
    """
    Convert samples in the audio fragment to a-LAW encoding and return this as a bytes object.
    a-LAW is an audio encoding format whereby you get a dynamic range of about 13 bits using
    only 8 bit samples. It is used by the Sun audio hardware, among others.
    """
    return _audioop.lin2alaw(fragment, width)


def lin2lin(fragment: bytes, width: int, newwidth: int) -> bytes:
    """
    Convert samples between 1-, 2-, 3- and 4-byte formats.
    """
    return _audioop.lin2lin(fragment, width, newwidth)


def lin2ulaw(fragment: bytes, width: int) -> bytes:
    """
    Convert samples in the audio fragment to u-LAW encoding and return this as a bytes object.
    u-LAW is an audio encoding format whereby you get a dynamic range of about 14 bits using
    only 8 bit samples. It is used by the Sun audio hardware, among others.
    """
    return _audioop.lin2ulaw(fragment, width)


def max(fragment: bytes, width: int) -> int:
    """
    Return the maximum of the absolute value of all samples in a fragment.
    """
    return _audioop.max(fragment, width)


def maxpp(fragment: bytes, width: int) -> int:
    """
    Return the maximum peak-peak value in the sound fragment.
    """
    return _audioop.maxpp(fragment, width)


def minmax(fragment: bytes, width: int) -> tuple[int, int]:
    """
    Return a tuple consisting of the minimum and maximum values
    of all samples in the sound fragment.
    """
    return _audioop.minmax(fragment, width)


def mul(fragment: bytes, width: int, factor: float) -> bytes:
    """
    Return a fragment that has all samples in the original fragment
    multiplied by the floating-point value factor.
    Samples are truncated in case of overflow.
    """
    return _audioop.mul(fragment, width, factor)


def ratecv(
    fragment: bytes,
    width: int,
    nchannels: int,
    inrate: int,
    outrate: int,
    state: Optional[tuple] = None,
    weightA: int = 1,
    weightB: int = 0,
) -> tuple:
    raise NotImplementedError("Still in development.")


def reverse(fragment: bytes, width: int) -> bytes:
    """
    Reverse the samples in a fragment and returns the modified fragment.
    """
    return _audioop.reverse(fragment, width)


def rms(fragment: bytes, width: int) -> int:
    """
    Return the root-mean-square of the fragment, i.e. sqrt(sum(S_i^2)/n).
    This is a measure of the power in an audio signal.
    """
    return _audioop.rms(fragment, width)


def tomono(fragment: bytes, width: int, lfactor: float, rfactor: float) -> bytes:
    """
    Convert a stereo fragment to a mono fragment. The left channel is
    multiplied by lfactor and the right channel by rfactor before adding the
    two channels to give a mono signal.
    """
    return _audioop.tomono(fragment, width, lfactor, rfactor)


def tostereo(fragment: bytes, width: int, lfactor: float, rfactor: float) -> bytes:
    """
    Generate a stereo fragment from a mono fragment. Each pair of samples in the
    stereo fragment are computed from the mono sample, whereby left channel samples
    are multiplied by lfactor and right channel samples by rfactor.
    """
    return _audioop.tostereo(fragment, width, lfactor, rfactor)


def ulaw2lin(fragment: bytes, width: int) -> bytes:
    """
    Convert sound fragments in u-LAW encoding to linearly encoded sound fragments.
    u-LAW encoding always uses 8 bits samples, so width refers only to the sample width
    of the output fragment here.
    """
    return _audioop.ulaw2lin(fragment, width)
