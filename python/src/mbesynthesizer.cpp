#include "mbesynthesizer.hpp"
#include "types.hpp"

#include <cstring>
#include <digiham/mbe_synthesizer.hpp>

static Digiham::Mbe::Mode* convertToAmbeMode(PyObject* mode) {
    PyTypeObject* TableModeType = getAmbeTableModeType();
    int rc = PyObject_IsInstance(mode, (PyObject*) TableModeType);
    Py_DECREF(TableModeType);

    if (rc == -1) return nullptr;
    if (rc) {
        PyObject* indexObj = PyObject_CallMethod(mode, "getIndex", NULL);
        if (indexObj == NULL) {
            return nullptr;
        }
        if (!PyLong_Check(indexObj)) {
            Py_DECREF(indexObj);
            return nullptr;
        }
        unsigned int index = PyLong_AsUnsignedLong(indexObj);
        if (PyErr_Occurred()) {
            Py_DECREF(indexObj);
            return nullptr;
        }
        Py_DECREF(indexObj);

        return new Digiham::Mbe::TableMode(index);
    }

    PyTypeObject* ControlWordModeType = getAmbeControlWordModeType();
    rc = PyObject_IsInstance(mode, (PyObject*) ControlWordModeType);
    Py_DECREF(ControlWordModeType);

    if (rc == -1) return nullptr;
    if (rc) {
        PyObject* controlWordBytes = PyObject_CallMethod(mode, "getBytes", NULL);
        if (controlWordBytes == NULL) {
            return nullptr;
        }
        if (!PyBytes_Check(controlWordBytes)) {
            Py_DECREF(controlWordBytes);
            return nullptr;
        }
        if (PyBytes_Size(controlWordBytes) != 12) {
            Py_DECREF(controlWordBytes);
            PyErr_SetString(PyExc_ValueError, "control word size mismatch, should be 12");
            return nullptr;
        }

        short* controlWords = (short*) malloc(sizeof(short) * 6);
        std::memcpy(controlWords, PyBytes_AsString(controlWordBytes), sizeof(short) * 6);

        auto result = new Digiham::Mbe::ControlWordMode(controlWords);
        free(controlWords);
        return result;
    }

    PyTypeObject* DynamicModeType = getAmbeDynamicModeType();
    rc = PyObject_IsInstance(mode, (PyObject*) DynamicModeType);
    Py_DECREF(DynamicModeType);

    if (rc == -1) return nullptr;
    if (rc) {
        Py_INCREF(mode);
        return new Digiham::Mbe::DynamicMode([mode] (unsigned char code) {
            // acquire GIL
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            PyObject* newMode = PyObject_CallMethod(mode, "getModeFor", "b", code);
            Digiham::Mbe::Mode* result = nullptr;
            if (newMode == NULL) {
                std::cerr << "failed to get mode for code " << +code << "\n";
            } else if (newMode == Py_None) {
                std::cerr << "mode for code " << +code << " was None\n";
                Py_DECREF(newMode);
            } else {
                result = convertToAmbeMode(newMode);
                Py_DECREF(newMode);
            }

            /* Release the thread. No Python API allowed beyond this point. */
            PyGILState_Release(gstate);

            return result;
        });
    }

    return nullptr;
}

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

    Digiham::Mbe::Mode* ambeMode = convertToAmbeMode(mode);
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

            // creating an mbesysnthesizer module potentially waits for network traffic, so we allow other threads in the meantime

            Csdr::UntypedModule* module;
            Py_BEGIN_ALLOW_THREADS
            module = new Digiham::Mbe::MbeSynthesizer(serverString, port, ambeMode);
            Py_END_ALLOW_THREADS
            self->setModule(module);
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    MbeSynthesizerSlots
};
