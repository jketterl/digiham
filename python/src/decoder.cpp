#include "decoder.hpp"
#include "types.hpp"

#include <digiham/decoder.hpp>
#include "pickleserializer.hpp"

static PyObject* Decoder_setMetaWriter(Decoder* self, PyObject* args, PyObject* kwds) {
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

        auto metaWriter = new Digiham::PipelineMetaWriter(new Digiham::PickleSerializer());
        metaWriter->setWriter(dynamic_cast<Csdr::Writer<unsigned char>*>(writer->writer));
        dynamic_cast<Digiham::Decoder*>(self->module)->setMetaWriter(metaWriter);
    }

    Py_RETURN_NONE;
}

static PyMethodDef Decoder_methods[] = {
    {"setMetaWriter", (PyCFunction) Decoder_setMetaWriter, METH_VARARGS | METH_KEYWORDS,
     "set writer for metadata"
    },
    {NULL}  /* Sentinel */
};

static PyType_Slot DecoderSlots[] = {
    {Py_tp_methods, Decoder_methods},
    {0, 0}
};

PyType_Spec DecoderSpec = {
    "digiham.modules.Decoder",
    sizeof(Decoder),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_FINALIZE,
    DecoderSlots
};
