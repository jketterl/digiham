# Digital Ham Radio decoding tools

This a simple set of command-line tools that is intended to be used to decode digital modulations used by ham radio
operators. The main focus is on digital voice modes.

Right now this project enables you to decode DMR and YSF, future plans include NXDN and D-Star.

The main use of this project is to run in the backend of [OpenWebRX](https://github.com/jketterl/openwebrx), where it
decodes the available information, which is then displayed on the receiver's website.

## Requirements

Most digital voice modes in the ham radio universe right now use some version of the AMBE digital voice codec. In order
to decode them, you will need to install [mbelib](https://github.com/szechyjs/mbelib).

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