#include "ysfdecoder.hpp"
#include "types.hpp"

#include <digiham/ysf_decoder.hpp>
#include <digiham/meta.hpp>

static int YsfDecoder_init(YsfDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Ysf::Decoder());

    return 0;
}

static PyType_Slot YsfDecoderSlots[] = {
    {Py_tp_init, (void*) YsfDecoder_init},
    {0, 0}
};

PyType_Spec YsfDecoderSpec = {
    "digiham.modules.YsfDecoder",
    sizeof(YsfDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    YsfDecoderSlots
};
