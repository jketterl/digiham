#include "digitalvoicefilter.hpp"
#include "types.hpp"

#include <digiham/digitalvoice_filter.hpp>

static int DigitalVoiceFilter_init(DigitalVoiceFilter* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_SHORT;
    self->outputFormat = FORMAT_SHORT;
    self->setModule(new Digiham::DigitalVoice::DigitalVoiceFilter());

    return 0;
}

static PyType_Slot DigitalVoiceFilterSlots[] = {
    {Py_tp_init, (void*) DigitalVoiceFilter_init},
    {0, 0}
};

PyType_Spec DigitalVoiceFilterSpec = {
    "digiham.modules.DigitalVoiceFilter",
    sizeof(DigitalVoiceFilter),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    DigitalVoiceFilterSlots
};
