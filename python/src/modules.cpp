#include "types.hpp"

#include "modules.hpp"
#include "dcblock.hpp"
#include "dstardecoder.hpp"
#include "fskdemodulator.hpp"
#include "digitalvoicefilter.hpp"
#include "narrowrrcfilter.hpp"
#include "widerrcfilter.hpp"
#include "mbesynthesizer.hpp"
#include "nxdndecoder.hpp"

static PyModuleDef pycsdrmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "digiham.modules",
    .m_doc = "Python bindings for the digiham library",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_modules(void) {
    PyObject* bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* DcBlockType = PyType_FromSpecWithBases(&DcBlockSpec, bases);
    if (DcBlockType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* DstarDecoderType = PyType_FromSpecWithBases(&DstarDecoderSpec, bases);
    if (DstarDecoderType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* FskDemodulatorType = PyType_FromSpecWithBases(&FskDemodulatorSpec, bases);
    if (FskDemodulatorType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* DigitalVoiceFilterType = PyType_FromSpecWithBases(&DigitalVoiceFilterSpec, bases);
    if (DigitalVoiceFilterType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* NarrowRrcFilterType = PyType_FromSpecWithBases(&NarrowRrcFilterSpec, bases);
    if (NarrowRrcFilterType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* WideRrcFilterType = PyType_FromSpecWithBases(&WideRrcFilterSpec, bases);
    if (WideRrcFilterType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* MbeSynthesizerType = PyType_FromSpecWithBases(&MbeSynthesizerSpec, bases);
    if (MbeSynthesizerType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* NxdnDecoderType = PyType_FromSpecWithBases(&NxdnDecoderSpec, bases);
    if (NxdnDecoderType == NULL) return NULL;

    PyObject *m = PyModule_Create(&pycsdrmodule);
    if (m == NULL) {
        return NULL;
    }

    PyModule_AddObject(m, "DcBlock", DcBlockType);

    PyModule_AddObject(m, "DstarDecoder", DstarDecoderType);

    PyModule_AddObject(m, "FskDemodulator", FskDemodulatorType);

    PyModule_AddObject(m, "DigitalVoiceFilter", DigitalVoiceFilterType);

    PyModule_AddObject(m, "NarrowRrcFilter", NarrowRrcFilterType);

    PyModule_AddObject(m, "WideRrcFilter", WideRrcFilterType);

    PyModule_AddObject(m, "MbeSynthesizer", MbeSynthesizerType);

    PyModule_AddObject(m, "NxdnDecoder", NxdnDecoderType);

    return m;
}