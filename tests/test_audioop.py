import audioop
import random
import audio.op as aup


test_value = random.randbytes(1000)
test_width = 2


def test_minmax():
    out_orig = audioop.minmax(test_value, test_width)
    out_new = aup.minmax(test_value, test_width)

    assert out_orig == out_new
