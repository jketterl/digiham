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

static PyType_Slot DmrDecoderSlots[] = {
    {Py_tp_init, (void*) DmrDecoder_init},
    {0, 0}
};

PyType_Spec DmrDecoderSpec = {
    "digiham.modules.DmrDecoder",
    sizeof(DmrDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    DmrDecoderSlots
};
