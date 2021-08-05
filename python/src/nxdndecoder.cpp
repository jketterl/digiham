#include "nxdndecoder.hpp"
#include "types.hpp"

#include <digiham/nxdn_decoder.hpp>

static int NxdnDecoder_init(NxdnDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Nxdn::Decoder());

    return 0;
}

static PyType_Slot NxdnDecoderSlots[] = {
    {Py_tp_init, (void*) NxdnDecoder_init},
    {0, 0}
};

PyType_Spec NxdnDecoderSpec = {
    "digiham.modules.NxdnDecoder",
    sizeof(NxdnDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    NxdnDecoderSlots
};
