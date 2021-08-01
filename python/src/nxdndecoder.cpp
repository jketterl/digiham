#include "nxdndecoder.hpp"
#include "types.hpp"

#include <digiham/nxdn_decoder.hpp>

static int NxdnDecoder_init(NxdnDecoder* self, PyObject* args, PyObject* kwds) {
    self->inputFormat = FORMAT_CHAR;
    self->outputFormat = FORMAT_CHAR;
    self->setModule(new Digiham::Nxdn::Decoder());

    return 0;
}

static PyObject* NxdnDecoder_setMetaWriter(NxdnDecoder* self, PyObject* args, PyObject* kwds) {
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

static PyMethodDef NxdnDecoder_methods[] = {
    {"setMetaWriter", (PyCFunction) NxdnDecoder_setMetaWriter, METH_VARARGS | METH_KEYWORDS,
     "set writer for metadata"
    },
    {NULL}  /* Sentinel */
};

static PyType_Slot NxdnDecoderSlots[] = {
    {Py_tp_init, (void*) NxdnDecoder_init},
    {Py_tp_methods, NxdnDecoder_methods},
    {0, 0}
};

PyType_Spec NxdnDecoderSpec = {
    "digiham.modules.NxdnDecoder",
    sizeof(NxdnDecoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    NxdnDecoderSlots
};
