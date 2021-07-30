from abc import ABC, abstractmethod


class Mode:
    pass


class TableMode(Mode):
    def __init__(self, index: int):
        self.index = index

    def getIndex(self) -> int:
        return self.index


class ControlWordMode(Mode):
    def __init__(self, cwds: bytes):
        self.cwds = cwds

    def getBytes(self):
        return self.cwds


class DynamicMode(ABC):
    @abstractmethod
    def getModeFor(self, code: int):
        pass


class YsfMode(DynamicMode, TableMode):
    def __init__(self):
        super().__init__(34)
        self.modeTable = {
            0: TableMode(33),
            1: TableMode(34),
            2: ControlWordMode(b'\x05\x58\x08\x6b\x10\x30\x00\x00\x00\x00\x01\x90')
        }

    def getModeFor(self, code: int):
        if code in self.modeTable:
            return self.modeTable[code]


class Modes:
    DmrMode = TableMode(33)
    DStarMode = ControlWordMode(b'\x01\x30\x07\x63\x40\x00\x00\x00\x00\x00\x00\x48')
    NxdnMode = TableMode(33)
    YsfMode = YsfMode()
