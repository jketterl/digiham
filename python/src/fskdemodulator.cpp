#include "fskdemodulator.hpp"
#include "types.hpp"

#include <digiham/fsk_demodulator.hpp>

static int FskDemodulator_init(FskDemodulator* self, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = {(char*) "samplesPerSymbol", (char*) "invert", NULL};

    unsigned int samplesPerSymbol = 40;
    int invert = false;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Ip", kwlist, &samplesPerSymbol, &invert)) {
        return -1;
    }

    if (samplesPerSymbol == 0) {
        PyErr_SetString(PyExc_ValueError, "samplesPerSymbol must not be zero");
        return -1;
    }

    self->inputFormat = FORMAT_FLOAT;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Fsk::FskDemodulator(samplesPerSymbol, invert));

    return 0;
}

static PyType_Slot FskDemodulatorSlots[] = {
    {Py_tp_init, (void*) FskDemodulator_init},
    {0, 0}
};

PyType_Spec FskDemodulatorSpec = {
    "digiham.modules.FskDemodulator",
    sizeof(FskDemodulator),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    FskDemodulatorSlots
};
