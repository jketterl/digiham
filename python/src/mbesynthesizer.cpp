#include "mbesynthesizer.hpp"
#include "types.hpp"

#include <digiham/mbe_synthesizer.hpp>

static int MbeSynthesizer_init(MbeSynthesizer* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_SHORT;

    static char* kwlist[] = {(char*) "server", NULL};

    char* server = (char*) "";
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &server)) {
        return -1;
    }

    std::string serverString(server);

    try {
        if (!serverString.length()) {
            // no arguments given, use default behavior
            self->setModule(new Digiham::Mbe::MbeSynthesizer());
        } else if (serverString.at(0) == '/') {
            // is a unix domain socket path
            self->setModule(new Digiham::Mbe::MbeSynthesizer(serverString));
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

            self->setModule(new Digiham::Mbe::MbeSynthesizer(serverString, port));
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
