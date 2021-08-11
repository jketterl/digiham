#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
from setuptools import setup, Extension


setup(
    name="digiham",
    version="0.6.0-dev",
    packages=["digiham"],

    package_data={
        "digiham": ["**.pyi", "**.py"],
    },

    headers=[
    ],

    ext_modules=[
        Extension(
            name="digiham.modules",
            sources=[
                "src/modules.cpp",
                "src/types.cpp",
                "src/decoder.cpp",
                "src/dcblock.cpp",
                "src/dstardecoder.cpp",
                "src/fskdemodulator.cpp",
                "src/digitalvoicefilter.cpp",
                "src/narrowrrcfilter.cpp",
                "src/widerrcfilter.cpp",
                "src/mbesynthesizer.cpp",
                "src/nxdndecoder.cpp",
                "src/gfskdemodulator.cpp",
                "src/dmrdecoder.cpp",
                "src/ysfdecoder.cpp",
                "src/pocsagdecoder.cpp",
            ],
            language="c++",
            include_dirs=[
                "src",
            ],
            libraries=["csdr++", "digiham"],
        )
    ],

    install_requires=["pycsdr"],
)
