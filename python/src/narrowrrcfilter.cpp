#include "narrowrrcfilter.hpp"
#include "types.hpp"

#include <digiham/rrc_filter.hpp>

static int NarrowRrcFilter_init(NarrowRrcFilter* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_FLOAT;
    self->outputFormat = FORMAT_FLOAT;
    self->setModule(new Digiham::RrcFilter::NarrowRrcFilter());

    return 0;
}

static PyType_Slot NarrowRrcFilterSlots[] = {
    {Py_tp_init, (void*) NarrowRrcFilter_init},
    {0, 0}
};

PyType_Spec NarrowRrcFilterSpec = {
    "digiham.modules.NarrowRrcFilter",
    sizeof(NarrowRrcFilter),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    NarrowRrcFilterSlots
};
