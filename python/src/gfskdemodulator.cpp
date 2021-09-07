#include "gfskdemodulator.hpp"
#include "types.hpp"

#include <digiham/gfsk_demodulator.hpp>

static int GfskDemodulator_init(GfskDemodulator* self, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = {(char*) "samplesPerSymbol", NULL};

    unsigned int samplesPerSymbol = 10;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I", kwlist, &samplesPerSymbol)) {
        return -1;
    }

    if (samplesPerSymbol == 0) {
        PyErr_SetString(PyExc_ValueError, "samplesPerSymbol must not be zero");
        return -1;
    }

    self->inputFormat = FORMAT_FLOAT;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Fsk::GfskDemodulator(samplesPerSymbol));

    return 0;
}

static PyType_Slot GfskDemodulatorSlots[] = {
        {Py_tp_init, (void*) GfskDemodulator_init},
        {0, 0}
};

PyType_Spec GfskDemodulatorSpec = {
        "digiham.modules.GfskDemodulator",
        sizeof(GfskDemodulator),
        0,
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
        GfskDemodulatorSlots
};
