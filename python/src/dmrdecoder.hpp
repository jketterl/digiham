#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "decoder.hpp"

struct DmrDecoder: Decoder {};

extern PyType_Spec DmrDecoderSpec;