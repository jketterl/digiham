from pycsdr.modules import Module, Writer
from digiham.ambe import Mode

version: str = ...


class DcBlock(Module):
    def __init__(self):
        ...


class FskDemodulator(Module):
    def __init__(self, samplesPerSymbol: int = 40, invert: bool = False):
        ...


class DigitalVoiceFilter(Module):
    def __init__(self):
        ...


class WideRrcFilter(Module):
    def __init__(self):
        ...


class NarrowRrcFilter(Module):
    def __init__(self):
        ...


class MbeSynthesizer(Module):
    def __init__(self, mode: Mode, server: str = ""):
        ...


class GfskDemodulator(Module):
    def __init__(self, samplesPerSymbol: int = 10):
        ...


class Decoder(Module):
    def setMetaWriter(self, metaWriter: Writer) -> None:
        ...


class DstarDecoder(Decoder):
    def __init__(self):
        ...


class NxdnDecoder(Decoder):
    def __init__(self):
        ...


class DmrDecoder(Decoder):
    def __init__(self):
        ...

    def setSlotFilter(self, filter: int) -> None:
        ...

class YsfDecoder(Decoder):
    def __init__(self):
        ...


class PocsagDecoder(Decoder):
    def __init__(self):
        ...
