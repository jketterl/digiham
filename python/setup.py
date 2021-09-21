#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
from setuptools import setup, Extension
from distutils.sysconfig import get_python_inc
from sys import version_info
from os import path


# some distributions seem to have messed with the install locations of header files, so we need to improvise here...
additional_includes = [
    x for x in [
        get_python_inc(plat_specific=True),
        get_python_inc(prefix="/usr/local"),
        "/usr/local/include/python{major}.{minor}".format(major=version_info.major, minor=version_info.minor),
    ] if path.isdir(x)
]

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
                "src/pickleserializer.cpp",
            ],
            language="c++",
            include_dirs=["src"] + additional_includes,
            libraries=["csdr++", "digiham"],
        )
    ],

    install_requires=["pycsdr"],
)
