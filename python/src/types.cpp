#include "types.hpp"

static PyObject* getPyCsdrTypeModule() {
    PyObject* module = PyImport_ImportModule("pycsdr.types");
    if (module == NULL) {
        PyErr_Print();
        exit(1);
    }
    return module;
}

static PyObject* getPyCsdrModulesModule() {
    PyObject* module = PyImport_ImportModule("pycsdr.modules");
    if (module == NULL) {
        PyErr_Print();
        exit(1);
    }
    return module;
}

static PyObject* getDigihamAmbeModule() {
    PyObject* module = PyImport_ImportModule("digiham.ambe");
    if (module == NULL) {
        PyErr_Print();
        exit(1);
    }
    return module;
}

PyTypeObject* getFormatType() {
    PyObject* module = getPyCsdrTypeModule();

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
    PyObject* module = getPyCsdrTypeModule();

    PyObject* AgcProfileType = PyObject_GetAttrString(module, "AgcProfile");
    if (AgcProfileType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) AgcProfileType;
}

PyTypeObject* getModuleType() {
    PyObject* module = getPyCsdrModulesModule();

    PyObject* ModuleType = PyObject_GetAttrString(module, "Module");
    if (ModuleType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) ModuleType;
}

PyTypeObject* getWriterType() {
    PyObject* module = getPyCsdrModulesModule();

    PyObject* WriterType = PyObject_GetAttrString(module, "Writer");
    if (WriterType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) WriterType;
}

PyTypeObject* getAmbeModeType() {
    PyObject* module = getDigihamAmbeModule();

    PyObject* ModeType = PyObject_GetAttrString(module, "Mode");
    if (ModeType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) ModeType;
}

PyTypeObject* getAmbeTableModeType() {
    PyObject* module = getDigihamAmbeModule();

    PyObject* ModeType = PyObject_GetAttrString(module, "TableMode");
    if (ModeType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) ModeType;
}

PyTypeObject* getAmbeControlWordModeType() {
    PyObject* module = getDigihamAmbeModule();

    PyObject* ModeType = PyObject_GetAttrString(module, "ControlWordMode");
    if (ModeType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) ModeType;
}

PyTypeObject* getAmbeDynamicModeType() {
    PyObject* module = getDigihamAmbeModule();

    PyObject* ModeType = PyObject_GetAttrString(module, "DynamicMode");
    if (ModeType == NULL) {
        PyErr_Print();
        exit(1);
    }

    Py_DECREF(module);

    return (PyTypeObject*) ModeType;
}