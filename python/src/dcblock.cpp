#include "dcblock.hpp"
#include "types.hpp"

#include <digiham/dc_block.hpp>

static int DcBlock_init(DcBlock* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_FLOAT;
    self->outputFormat = FORMAT_FLOAT;
    self->setModule(new Digiham::DcBlock::DcBlock());

    return 0;
}

static PyType_Slot DcBlockSlots[] = {
        {Py_tp_init, (void*) DcBlock_init},
        {0, 0}
};

PyType_Spec DcBlockSpec = {
        "digiham.modules.DcBlock",
        sizeof(DcBlock),
        0,
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
        DcBlockSlots
};
