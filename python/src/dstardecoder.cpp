#include "dstardecoder.hpp"
#include "types.hpp"

#include <digiham/dstar_decoder.hpp>
#include <digiham/meta.hpp>

static int DstarDecoder_init(DstarDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::DStar::Decoder());

    return 0;
}

static PyType_Slot DstarDecoderSlots[] = {
    {Py_tp_init, (void*) DstarDecoder_init},
    {0, 0}
};

PyType_Spec DstarDecoderSpec = {
    "digiham.modules.DstarDecoder",
    sizeof(DstarDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    DstarDecoderSlots
};
