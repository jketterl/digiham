# Digital Ham Radio decoding tools

This is a simple set of command-line tools that is intended to be used to decode digital modulations used by ham radio
operators. The main focus is on digital voice modes.

Right now this project enables you to decode DMR and YSF, future plans include NXDN and D-Star.

The main use of this project is to run in the backend of [OpenWebRX](https://github.com/jketterl/openwebrx), where it
decodes the available information, which is then displayed on the receiver's website.

## Requirements

Please make sure you install the following dependencies before compiling digiham:

- [csdr](https://github.com/jketterl/csdr) (version 0.18 or later)
- [codecserver](https://github.com/jketterl/codecserver)
- [ICU](https://icu.unicode.org/) (for Debian, install `libicu-dev`)


## About the AMBE codec

Most digital voice modes in the ham radio universe right now use some version of the AMBE digital voice codec. In order
to decode them, you will need to setup the correspoding decoding infrastructure.

This project comes with mbe_synthesizer that can send the received audio data to a
[codecserver](https://github.com/jketterl/codecserver) instance for decoding.

## Installation

This project comes with a cmake build. It is recommended to build in a separate directory.

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Examples

You can find shell scripts that show the basic usage of the components in the `examples` folder.