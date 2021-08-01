#include "dstardecoder.hpp"
#include "types.hpp"

#include <digiham/dstar_decoder.hpp>
#include <digiham/meta.hpp>

static int DstarDecoder_init(DstarDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::DStar::Decoder());

    return 0;
}

static PyObject* DstarDecoder_setMetaWriter(DstarDecoder* self, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = {(char*) "writer", NULL};

    PyTypeObject* WriterType = getWriterType();
    Writer* writer;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist, WriterType, &writer)) {
        Py_DECREF(WriterType);
        return NULL;
    }
    Py_DECREF(WriterType);

    if ((PyObject*) writer != Py_None && writer->writerFormat != FORMAT_CHAR) {
        PyErr_SetString(PyExc_ValueError, "invalid writer format");
        return NULL;
    }

    if (self->metaWriter != nullptr) {
        Py_DECREF(self->metaWriter);
        self->metaWriter = nullptr;
    }

    if ((PyObject*) writer != Py_None) {
        self->metaWriter = writer;
        Py_INCREF(self->metaWriter);

        auto metaWriter = new Digiham::PipelineMetaWriter();
        metaWriter->setWriter(dynamic_cast<Csdr::Writer<unsigned char>*>(writer->writer));
        dynamic_cast<Digiham::Decoder*>(self->module)->setMetaWriter(metaWriter);
    }

    Py_RETURN_NONE;
}

static PyMethodDef DstarDecoder_methods[] = {
    {"setMetaWriter", (PyCFunction) DstarDecoder_setMetaWriter, METH_VARARGS | METH_KEYWORDS,
     "set writer for metadata"
    },
    {NULL}  /* Sentinel */
};

static PyType_Slot DstarDecoderSlots[] = {
    {Py_tp_init, (void*) DstarDecoder_init},
    {Py_tp_methods, DstarDecoder_methods},
    {0, 0}
};

PyType_Spec DstarDecoderSpec = {
    "digiham.modules.DstarDecoder",
    sizeof(DstarDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    DstarDecoderSlots
};
