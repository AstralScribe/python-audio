import audio.op as aup
import audioop


test_value = b"\x03\xf7V}G\x04\xdd\x04!\x92S\xc0\xc2\xb4\x04\xcd\xa88f&\xcb\xfcf\xe3\xaf\x05(\xbfij;\xf6\x96D`^Z\xedk\xce\xb0>\x8d\xdb\x1d['\xbc\x17\x0f\xc3Z1\xe6\xb81\xb4 \xef\x82\xf3\x1d\xa9\xe8\xb5\xe3\xac!\xdar\xad]\xf9\x0fg\xe0\xff\x06J\x8e\xa7 \x98\x101)\xcd~\xfbV\xc0h\xc1\xbb\x13\xe5\xe0\xebS\x0b"
test_value_ref = b'4D44\x83D8\x19\xc7\xf4\x8d+1\xbe+\xde\xcf6;\xd2\xdc5\xd9\xf0\xc6\xff!_R\xb7\xa7\xca\xc6\xbe\x0f\x90\xd3\x8b;\xaf\xf6\xbd\x1a\x04\xfb\xa8\xe5\xdff\xa9Q\xab\x12b\x9d\xd7\x96\xc7\x1c\xdfw2\x06-\xe0\xcb\xd6Y\xe7\xd6\x8b\xfe\xc4\xf0\xe7oDG\x81j\x10\x04\r\xe2\xef\xa1s\x1e\x06\xbf\x83\xc8\xa3\xb6\xc7d\x01\xa4O\x16'
test_value_ref_small = b'\xd2\xcdZuz\xf2"i\xb8\x9c'
test_width = 2


def test_add():
    output = aup.add(test_value, test_value_ref, test_width)
    assert output == audioop.add(test_value, test_value_ref, test_width)

def test_alaw2lin():
    output = aup.alaw2lin(test_value, test_width)
    assert output == audioop.alaw2lin(test_value, test_width)

def test_avg():
    output = aup.avg(test_value, test_width)
    assert output == audioop.avg(test_value, test_width)

def test_avgpp():
    output = aup.avgpp(test_value, test_width)
    assert output == audioop.avgpp(test_value, test_width)

def test_bias():
    output = aup.bias(test_value, test_width, 3)
    assert output == audioop.bias(test_value, test_width, 3)

def test_byteswap():
    output = aup.byteswap(test_value, test_width)
    assert output == audioop.byteswap(test_value, test_width)

def test_cross():
    output = aup.cross(test_value, test_width)
    assert output == audioop.cross(test_value, test_width)

def test_findfactor():
    output = aup.findfactor(test_value, test_value_ref)
    assert output == audioop.findfactor(test_value, test_value_ref)

def test_findfit():
    output = aup.findfit(test_value, test_value_ref_small)
    assert output == audioop.findfit(test_value, test_value_ref_small)

def test_findmax():
    output = aup.findmax(test_value, 45)
    assert output == audioop.findmax(test_value, 45)

def test_getsample():
    output = aup.getsample(test_value, test_width, 45)
    assert output == audioop.getsample(test_value, test_width, 45)

def test_lin2alaw():
    output = aup.lin2alaw(test_value, test_width)
    assert output == audioop.lin2alaw(test_value, test_width)

def test_lin2lin():
    output = aup.lin2lin(test_value, 2, 3)
    assert output == audioop.lin2lin(test_value, 2, 3)

def test_lin2ulaw():
    output = aup.lin2ulaw(test_value, test_width)
    assert output == b'<\x80\xcc\xca\x04\x0f\r\x16\x93\x9cR"\xc7\x0f\x85;\x8e\x88,\x16\x90\x1d\x89\x0e\xb0\x89%\x96\x9f\x00\xa1(#\x9e\x83\x88\xaf\x1f\xc1\x03\x9f\xae\x9b\x80\x8a\x85\x0e$+\xb8'

def test_max():
    output = aup.max(test_value, test_width)
    assert output == audioop.max(test_value, test_width)

def test_maxpp():
    output = aup.maxpp(test_value, test_width)
    assert output == audioop.maxpp(test_value, test_width)

def test_minmax():
    output = aup.minmax(test_value, test_width)
    assert output == (-32017, 32461)

def test_mul():
    output = aup.mul(test_value, test_width, 4)
    assert output == audioop.mul(test_value, test_width, 4)

def test_reverse():
    output = aup.reverse(test_value, test_width)
    assert output == audioop.reverse(test_value, test_width)

def test_rms():
    output = aup.rms(test_value, test_width)
    assert output == audioop.rms(test_value, test_width)

def test_tomono():
    output = aup.tomono(test_value, test_width, 1.2, 1.2)
    assert output == audioop.tomono(test_value, test_width, 1.2, 1.2)

def test_tostereo():
    output = aup.tostereo(test_value, test_width, 1.2, 1.2)
    assert output == audioop.tostereo(test_value, test_width, 1.2, 1.2)

def test_ulaw2lin():
    output = aup.ulaw2lin(test_value, test_width)
    assert output == b'\x84\x8e@\x00T\xfd\xf0\xffd\xfa\x84\x92\xcc\x01\x84\x92\x04\xe2|:\xf4\xfc\\\x07\xdc\x06<\r\x84\x92\x1c\x04\xfc\x16\xc4\xf4\xec\xfe\x04\xe7\x9c\x04\x18\x00\xec\xfeD\x01\xfc\x0f\x84\x96\x04\xe9\xbc\x07\x1c\xff,\xffD\xf6H\x00|2\xa4\xf9\x8c\xfeT\xfe\xd4\xfd\xa4\x00<\xff\xdc\x03<\x0f\xc4\xf7|I\x0c\x02\x84\xdb\xf4\xfd\x04\xe8<\t\x84\xcf\x84\xbe\x9c\x06\xd4\xfdD\xf1\x14\x01<\x0bD\xf1<\r\x04\xe1\x84\x00|u`\x00\x84\xdb\xfc\x15\xf4\x00\xbc\x0cD\x01\xfc\x12\x04\xe2,\x02\x98\xff\xfc\x114\xfe0\x00\x84\xbe\xfc\xfet\x01\x00\x00\x84\x9a$\xfb|E\xfc\x17\x04\xe1|.\x84\xc1D\xf1\x04\xea\x1c\x04\xf8\xff \x00T\xfd\\\x07\x0c\xff\x1c\x07\xbc\t\x84\xc7$\x01t\x01\xc4\x00\xf4\xfc\x84\xae'
