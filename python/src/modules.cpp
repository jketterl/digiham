#include "modules.hpp"
#include "dcblock.hpp"
#include "dstardecoder.hpp"
#include "fskdemodulator.hpp"
#include "digitalvoicefilter.hpp"
#include "narrowrrcfilter.hpp"
#include "widerrcfilter.hpp"

static PyModuleDef pycsdrmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "digiham.modules",
    .m_doc = "Python bindings for the digiham library",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_modules(void) {
    PyObject* DcBlockType = PyType_FromSpec(&DcBlockSpec);
    if (DcBlockType == NULL) return NULL;

    PyObject* DstarDecoderType = PyType_FromSpec(&DstarDecoderSpec);
    if (DstarDecoderType == NULL) return NULL;

    PyObject* FskDemodulatorType = PyType_FromSpec(&FskDemodulatorSpec);
    if (FskDemodulatorType == NULL) return NULL;

    PyObject* DigitalVoiceFilterType = PyType_FromSpec(&DigitalVoiceFilterSpec);
    if (DigitalVoiceFilterType == NULL) return NULL;

    PyObject* NarrowRrcFilterType = PyType_FromSpec(&NarrowRrcFilterSpec);
    if (NarrowRrcFilterType == NULL) return NULL;

    PyObject* WideRrcFilterType = PyType_FromSpec(&WideRrcFilterSpec);
    if (WideRrcFilterType == NULL) return NULL;

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

    return m;
}