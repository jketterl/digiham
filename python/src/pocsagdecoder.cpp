#include "pocsagdecoder.hpp"
#include "types.hpp"
#include "pickleserializer.hpp"

#include <digiham/pocsag_decoder.hpp>

static int PocsagDecoder_init(PocsagDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Pocsag::Decoder(new Digiham::PickleSerializer()));

    return 0;
}

static PyType_Slot PocsagDecoderSlots[] = {
    {Py_tp_init, (void*) PocsagDecoder_init},
    {0, 0}
};

PyType_Spec PocsagDecoderSpec = {
    "digiham.modules.PocsagDecoder",
    sizeof(PocsagDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    PocsagDecoderSlots
};
