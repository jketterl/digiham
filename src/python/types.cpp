#include "types.hpp"

static PyObject* getPyCsdrModule() {
    PyObject* module = PyImport_ImportModule("pycsdr.types");
    if (module == NULL) {
        PyErr_Print();
        exit(1);
    }
    return module;
}

PyTypeObject* getFormatType() {
    PyObject* module = getPyCsdrModule();

    PyObject* FormatType = PyObject_GetAttrString(module, "Format");
    if (FormatType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) FormatType;
}

PyObject* getFormat(const char* name) {
    PyObject* format = PyObject_GetAttrString((PyObject*) FORMAT_TYPE, name);
    if (format == NULL) {
        PyErr_Print();
        exit(1);
    }

    return format;
}

PyTypeObject* getAgcProfileType() {
    PyObject* module = getPyCsdrModule();

    PyObject* AgcProfileType = PyObject_GetAttrString(module, "AgcProfile");
    if (AgcProfileType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) AgcProfileType;
}