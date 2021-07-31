#include "mbesynthesizer.hpp"
#include "types.hpp"

#include <digiham/mbe_synthesizer.hpp>

static int MbeSynthesizer_init(MbeSynthesizer* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_SHORT;

    static char* kwlist[] = {(char*) "mode", (char*) "server", NULL};

    PyTypeObject* ModeType = getAmbeModeType();
    char* server = (char*) "";
    PyObject* mode;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|s", kwlist, ModeType, &mode, &server)) {
        Py_DECREF(ModeType);
        return -1;
    }
    Py_DECREF(ModeType);

    Digiham::Mbe::Mode* ambeMode = nullptr;

    PyTypeObject* TableModeType = getAmbeTableModeType();
    int rc = PyObject_IsInstance(mode, (PyObject*) TableModeType);
    Py_DECREF(TableModeType);

    if (rc == -1) return -1;
    if (rc) {
        PyObject* indexObj = PyObject_CallMethod(mode, "getIndex", NULL);
        if (!PyLong_Check(indexObj)) {
            Py_DECREF(indexObj);
            return -1;
        }
        unsigned int index = PyLong_AsUnsignedLong(indexObj);
        if (PyErr_Occurred()) {
            Py_DECREF(indexObj);
            return -1;
        }
        Py_DECREF(indexObj);

        ambeMode = new Digiham::Mbe::TableMode(index);
    }

    PyTypeObject* ControlWordModeType = getAmbeControlWordModeType();
    rc = PyObject_IsInstance(mode, (PyObject*) ControlWordModeType);
    Py_DECREF(TableModeType);

    if (rc == -1) return -1;
    if (rc) {
        PyObject* controlWordBytes = PyObject_CallMethod(mode, "getBytes", NULL);
        if (!PyBytes_Check(controlWordBytes)) {
            Py_DECREF(controlWordBytes);
            return -1;
        }
        if (PyBytes_Size(controlWordBytes) != 12) {
            Py_DECREF(controlWordBytes);
            PyErr_SetString(PyExc_ValueError, "control word size mismatch, should be 12");
            return -1;
        }

        short* controlWords = (short*) malloc(sizeof(short) * 6);
        std::memcpy(controlWords, PyBytes_AsString(controlWordBytes), sizeof(short) * 6);

        ambeMode = new Digiham::Mbe::ControlWordMode(controlWords);
        free(controlWords);
    }

    if (ambeMode == nullptr) {
        PyErr_SetString(PyExc_ValueError, "unsupported ambe mode");
        return -1;
    }

    std::string serverString(server);

    try {
        if (!serverString.length()) {
            // no arguments given, use default behavior
            self->setModule(new Digiham::Mbe::MbeSynthesizer(ambeMode));
        } else if (serverString.at(0) == '/') {
            // is a unix domain socket path
            self->setModule(new Digiham::Mbe::MbeSynthesizer(serverString, ambeMode));
        } else {
            // is an IPv4 / IPv6 address or hostname as string

            // default port
            unsigned short port = 1073;

            // split by port number, if given
            size_t pos = serverString.find(":");
            if (pos != std::string::npos) {
                port = std::stoul(serverString.substr(pos + 1));
                serverString = serverString.substr(0, pos);
            }

            self->setModule(new Digiham::Mbe::MbeSynthesizer(serverString, port, ambeMode));
        }
    } catch (const Digiham::Mbe::ConnectionError& e) {
        PyErr_SetString(PyExc_ConnectionError, e.what());
        return -1;
    }

    return 0;
}

static PyType_Slot MbeSynthesizerSlots[] = {
    {Py_tp_init, (void*) MbeSynthesizer_init},
    {0, 0}
};

PyType_Spec MbeSynthesizerSpec = {
    "digiham.modules.MbeSynthesizer",
    sizeof(MbeSynthesizer),
    0,
    Py_TPFLAGS_DEFAULT,
    MbeSynthesizerSlots
};
