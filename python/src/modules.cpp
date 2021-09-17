#include "types.hpp"

#include "modules.hpp"
#include "decoder.hpp"
#include "dcblock.hpp"
#include "dstardecoder.hpp"
#include "fskdemodulator.hpp"
#include "gfskdemodulator.hpp"
#include "digitalvoicefilter.hpp"
#include "narrowrrcfilter.hpp"
#include "widerrcfilter.hpp"
#include "mbesynthesizer.hpp"
#include "nxdndecoder.hpp"
#include "dmrdecoder.hpp"
#include "ysfdecoder.hpp"
#include "pocsagdecoder.hpp"

#include <digiham/version.hpp>

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
    PyObject* DecoderType = PyType_FromSpecWithBases(&DecoderSpec, bases);
    if (DecoderType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* DcBlockType = PyType_FromSpecWithBases(&DcBlockSpec, bases);
    if (DcBlockType == NULL) return NULL;

    Py_INCREF(DecoderType);
    bases = PyTuple_Pack(1, DecoderType);
    if (bases == NULL) return NULL;
    PyObject* DstarDecoderType = PyType_FromSpecWithBases(&DstarDecoderSpec, bases);
    if (DstarDecoderType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* FskDemodulatorType = PyType_FromSpecWithBases(&FskDemodulatorSpec, bases);
    if (FskDemodulatorType == NULL) return NULL;

    bases = PyTuple_Pack(1, getModuleType());
    if (bases == NULL) return NULL;
    PyObject* GfskDemodulatorType = PyType_FromSpecWithBases(&GfskDemodulatorSpec, bases);
    if (GfskDemodulatorType == NULL) return NULL;

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

    Py_INCREF(DecoderType);
    bases = PyTuple_Pack(1, DecoderType);
    if (bases == NULL) return NULL;
    PyObject* NxdnDecoderType = PyType_FromSpecWithBases(&NxdnDecoderSpec, bases);
    if (NxdnDecoderType == NULL) return NULL;

    Py_INCREF(DecoderType);
    bases = PyTuple_Pack(1, DecoderType);
    if (bases == NULL) return NULL;
    PyObject* DmrDecoderType = PyType_FromSpecWithBases(&DmrDecoderSpec, bases);
    if (DmrDecoderType == NULL) return NULL;

    Py_INCREF(DecoderType);
    bases = PyTuple_Pack(1, DecoderType);
    if (bases == NULL) return NULL;
    PyObject* YsfDecoderType = PyType_FromSpecWithBases(&YsfDecoderSpec, bases);
    if (YsfDecoderType == NULL) return NULL;

    Py_INCREF(DecoderType);
    bases = PyTuple_Pack(1, DecoderType);
    if (bases == NULL) return NULL;
    PyObject* PocsagDecoderType = PyType_FromSpecWithBases(&PocsagDecoderSpec, bases);
    if (PocsagDecoderType == NULL) return NULL;

    PyObject *m = PyModule_Create(&pycsdrmodule);
    if (m == NULL) {
        return NULL;
    }

    PyModule_AddObject(m, "DcBlock", DcBlockType);

    PyModule_AddObject(m, "DstarDecoder", DstarDecoderType);

    PyModule_AddObject(m, "FskDemodulator", FskDemodulatorType);

    PyModule_AddObject(m, "GfskDemodulator", GfskDemodulatorType);

    PyModule_AddObject(m, "DigitalVoiceFilter", DigitalVoiceFilterType);

    PyModule_AddObject(m, "NarrowRrcFilter", NarrowRrcFilterType);

    PyModule_AddObject(m, "WideRrcFilter", WideRrcFilterType);

    PyModule_AddObject(m, "MbeSynthesizer", MbeSynthesizerType);

    PyModule_AddObject(m, "NxdnDecoder", NxdnDecoderType);

    PyModule_AddObject(m, "DmrDecoder", DmrDecoderType);

    PyModule_AddObject(m, "YsfDecoder", YsfDecoderType);

    PyModule_AddObject(m, "PocsagDecoder", PocsagDecoderType);

    PyObject* version = PyUnicode_FromStringAndSize(Digiham::version.c_str(), Digiham::version.length());
    if (version == NULL) return NULL;
    PyModule_AddObject(m, "version", version);

    return m;
}