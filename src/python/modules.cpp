#include "modules.hpp"
#include "dstardecoder.hpp"

static PyModuleDef pycsdrmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "digiham.modules",
    .m_doc = "Python bindings for the digiham library",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_modules(void) {
    PyObject* DstarDecoderType = PyType_FromSpec(&DstarDecoderSpec);
    if (DstarDecoderType == NULL) return NULL;

    PyObject *m = PyModule_Create(&pycsdrmodule);
    if (m == NULL) {
        return NULL;
    }

    PyModule_AddObject(m, "DstarDecoder", DstarDecoderType);

    return m;
}