#include "widerrcfilter.hpp"
#include "types.hpp"

#include <digiham/rrc_filter.hpp>

static int WideRrcFilter_init(WideRrcFilter* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_FLOAT;
    self->outputFormat = FORMAT_FLOAT;
    self->setModule(new Digiham::RrcFilter::WideRrcFilter());

    return 0;
}

static PyType_Slot WideRrcFilterSlots[] = {
    {Py_tp_init, (void*) WideRrcFilter_init},
    {0, 0}
};

PyType_Spec WideRrcFilterSpec = {
    "digiham.modules.WideRrcFilter",
    sizeof(WideRrcFilter),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    WideRrcFilterSlots
};
