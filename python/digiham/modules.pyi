from pycsdr.modules import Module


class DcBlock(Module):
    def __init__(self):
        ...


class DstarDecoder(Module):
    def __init__(self):
        ...


class FskDemodulator(Module):
    def __init__(self, samplesPerSymbol: int = 40, invert: bool = False):
        ...