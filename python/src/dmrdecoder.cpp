#include "dmrdecoder.hpp"
#include "types.hpp"

#include <digiham/dmr_decoder.hpp>
#include <digiham/meta.hpp>

static int DmrDecoder_init(DmrDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Dmr::Decoder());

    return 0;
}

static PyObject* DmrDecoder_setSlotFilter(DmrDecoder* self, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = {(char*) "filter", NULL};

    unsigned char filter;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "b", kwlist, &filter)) {
        return NULL;
    }

    // unsigned char cannot be < 0 without overflowing, so no need to check that
    if (filter > 3) {
        PyErr_SetString(PyExc_ValueError, "filter must be 0 <= filter <= 3");
        return NULL;
    }

    dynamic_cast<Digiham::Dmr::Decoder*>(self->module)->setSlotFilter(filter);

    Py_RETURN_NONE;
}

static PyMethodDef DmrDecoder_methods[] = {
    {"setSlotFilter", (PyCFunction) DmrDecoder_setSlotFilter, METH_VARARGS | METH_KEYWORDS,
     "set TDMA timeslot filter"
    },
    {NULL}  /* Sentinel */
};


static PyType_Slot DmrDecoderSlots[] = {
    {Py_tp_init, (void*) DmrDecoder_init},
    {Py_tp_methods, DmrDecoder_methods},
    {0, 0}
};

PyType_Spec DmrDecoderSpec = {
    "digiham.modules.DmrDecoder",
    sizeof(DmrDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    DmrDecoderSlots
};
