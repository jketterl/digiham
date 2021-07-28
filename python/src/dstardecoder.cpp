#include "dstardecoder.hpp"
#include "types.hpp"

#include <digiham/dstar_decoder.hpp>

static int DstarDecoder_init(DstarDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->module = new Digiham::DStar::Decoder();
    self->source = dynamic_cast<Csdr::UntypedSource*>(self->module);
    self->sink = dynamic_cast<Csdr::UntypedSink*>(self->module);

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
    Py_TPFLAGS_DEFAULT,
    DstarDecoderSlots
};
