#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "decoder.hpp"

struct YsfDecoder: Decoder {};

extern PyType_Spec YsfDecoderSpec;