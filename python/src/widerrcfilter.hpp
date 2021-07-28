#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pycsdr/module.hpp>

struct WideRrcFilter: Module {};

extern PyType_Spec WideRrcFilterSpec;