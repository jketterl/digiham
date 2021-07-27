#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
from setuptools import setup, Extension


setup(
    name="digiham",
    version="0.5.0-dev",
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
                "src/python/modules.cpp",
                "src/python/types.cpp",
                "src/python/dstardecoder.cpp",
            ],
            language="c++",
            include_dirs=[
                "src/python",
                "src/lib",
                "src/dstar_decoder",
            ],
            libraries=["csdr++", "digiham"],
        )
    ],

    install_requires=["pycsdr"],
)
