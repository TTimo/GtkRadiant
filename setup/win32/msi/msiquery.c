/*
Copyright (C) 2001-2006 William Joseph.
For a list of contributors, see the accompanying CONTRIBUTORS file.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Python.h"
#include "structmember.h"

#include <windows.h>
#include <msi.h>
#include <msiquery.h>

typedef struct {
  PyObject_HEAD
  MSIHANDLE handle;
} MsiRecord;

static void
MsiRecord_dealloc(MsiRecord* self)
{
  if(self->handle != 0)
  {
    unsigned int result = MsiCloseHandle(self->handle);
  }
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
MsiRecord_new(PyTypeObject *type, PyObject *args)
{
  MsiRecord *self;

  self = (MsiRecord *)type->tp_alloc(type, 0);
  if (self != NULL)
  {
    self->handle = 0;
  }

  return (PyObject *)self;
}

static int
MsiRecord_init(MsiRecord *self, PyObject *args)
{
  return 0;
}

static PyMemberDef MsiRecord_members[] = {
    {NULL}  /* Sentinel */
};

static PyObject *
MsiRecord_getstring(MsiRecord* self, PyObject *args)
{
  char string[16384];
  DWORD size = 16384;
  int index;
  unsigned int result = 1;
  if (!PyArg_ParseTuple(args, "i", &index))
  {
    return NULL;
  }
  if(self->handle != 0)
  {
    if(MsiRecordIsNull(self->handle, index))
    {
      puts("null record index");
    }
    result = MsiRecordGetString(self->handle, index, string, &size);
    printf("string: %s, index: %i\n", string, index);
    if(result != 0)
    {
      strcpy(string, "bleh");
    }
  }
  return PyString_FromString(string);
}

static PyObject *
MsiRecord_setstring(MsiRecord* self, PyObject *args)
{
  char* string;
  int index;
  unsigned int result = 1;
  if (!PyArg_ParseTuple(args, "is", &index, &string))
  {
    return NULL;
  }
  if(self->handle != 0)
  {
    result = MsiRecordSetString(self->handle, index, string);
  }
  return Py_BuildValue("i", result);
}

static PyMethodDef MsiRecord_methods[] = {
    {"getstring", (PyCFunction)MsiRecord_getstring, METH_VARARGS,
     "MsiRecordGetString"
    },
    {"setstring", (PyCFunction)MsiRecord_setstring, METH_VARARGS,
     "MsiRecordSetString"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject MsiRecordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "msiquery.MsiRecord",             /*tp_name*/
    sizeof(MsiRecord),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)MsiRecord_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "MsiRecord objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    MsiRecord_methods,             /* tp_methods */
    MsiRecord_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)MsiRecord_init,      /* tp_init */
    0,                         /* tp_alloc */
    (newfunc)MsiRecord_new,                 /* tp_new */
};

PyObject*
MsiRecord_FromHandle(MSIHANDLE handle)
{
  MsiRecord *record = (MsiRecord *)MsiRecordType.tp_new(&MsiRecordType, NULL, NULL);
  record->handle = handle;
  return (PyObject*)record;
}


typedef struct {
  PyObject_HEAD
  MSIHANDLE handle;
} MsiView;

static void
MsiView_dealloc(MsiView* self)
{
  if(self->handle != 0)
  {
    unsigned int result;
    result = MsiViewClose(self->handle);
    result = MsiCloseHandle(self->handle);
  }
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
MsiView_new(PyTypeObject *type, PyObject *args)
{
  MsiView *self;

  self = (MsiView *)type->tp_alloc(type, 0);
  if (self != NULL)
  {
    self->handle = 0;
  }

  return (PyObject *)self;
}

static int
MsiView_init(MsiView *self)
{
  return 0;
}

static PyMemberDef MsiView_members[] = {
    {NULL}  /* Sentinel */
};

static PyObject *
MsiView_fetch(MsiView* self)
{
  unsigned int result = 1;
  MSIHANDLE record;
  result = MsiViewFetch(self->handle, &record);
  if(result == 0)
  {
    return MsiRecord_FromHandle(record);
  }
  else
  {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

static PyObject *
MsiView_update(MsiView* self, PyObject *args)
{
  unsigned int result = 1;
  MsiRecord *record;
  if(!PyArg_ParseTuple(args, "O!", &MsiRecordType, &record))
  {
    return NULL;
  }
  result = MsiViewModify(self->handle, MSIMODIFY_UPDATE, record->handle);
  return Py_BuildValue("i", result);
}

static PyMethodDef MsiView_methods[] = {
    {"fetch", (PyCFunction)MsiView_fetch, METH_NOARGS,
     "MsiViewFetch"
    },
    {"update", (PyCFunction)MsiView_update, METH_VARARGS,
     "MsiViewModify(UPDATE)"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject MsiViewType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "msiquery.MsiView",             /*tp_name*/
    sizeof(MsiView),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)MsiView_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "MsiView objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    MsiView_methods,             /* tp_methods */
    MsiView_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)MsiView_init,      /* tp_init */
    0,                         /* tp_alloc */
    (newfunc)MsiView_new,                 /* tp_new */
};

PyObject*
MsiView_FromHandle(MSIHANDLE handle)
{
  MsiView *view = (MsiView *)MsiViewType.tp_new(&MsiViewType, NULL, NULL);
  view->handle = handle;
  MsiViewExecute(handle, 0);
  return (PyObject*)view;
}


typedef struct {
  PyObject_HEAD
  MSIHANDLE handle;
} MsiDB;

static void
MsiDB_dealloc(MsiDB* self)
{
  if(self->handle != 0)
  {
    unsigned int result = MsiCloseHandle(self->handle);
  }
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
MsiDB_new(PyTypeObject *type, PyObject *args)
{
  MsiDB *self;

  self = (MsiDB *)type->tp_alloc(type, 0);
  if (self != NULL)
  {
    self->handle = 0;
  }

  return (PyObject *)self;
}

static int
MsiDB_init(MsiDB *self, PyObject *args)
{
  char* filename;
  if(!PyArg_ParseTuple(args, "s", &filename))
  {
    return -1;
  }

  MsiOpenDatabase(filename, MSIDBOPEN_TRANSACT, &self->handle);

  return 0;
}

static PyMemberDef MsiDB_members[] = {
    {NULL}  /* Sentinel */
};

static PyObject *
MsiDB_openview(MsiDB* self, PyObject *args)
{
  char* query;
  unsigned int result = 1;
  if (!PyArg_ParseTuple(args, "s", &query))
  {
    return NULL;
  }
  if(self->handle != 0)
  {
    MSIHANDLE view;
    result = MsiDatabaseOpenView(self->handle, query, &view);
    if(result == ERROR_SUCCESS)
    {
      return MsiView_FromHandle(view);
    }
  }
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
MsiDB_commit(MsiDB* self)
{
  unsigned int result = 1;
  if(self->handle != 0)
  {
    result = MsiDatabaseCommit(self->handle);
    if(result == ERROR_INVALID_HANDLE)
    {
      printf("invalid??\n");
    }
  }
  return Py_BuildValue("i", result);
}

static PyMethodDef MsiDB_methods[] = {
    {"openview", (PyCFunction)MsiDB_openview, METH_VARARGS,
     "MsiViewExecute SQL query"
    },
    {"commit", (PyCFunction)MsiDB_commit, METH_NOARGS,
     "MsiDatabaseCommit"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject MsiDBType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "msiquery.MsiDB",             /*tp_name*/
    sizeof(MsiDB),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)MsiDB_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "MsiDB objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    MsiDB_methods,             /* tp_methods */
    MsiDB_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)MsiDB_init,      /* tp_init */
    0,                         /* tp_alloc */
    (newfunc)MsiDB_new,                 /* tp_new */
};

static PyMethodDef msiquery_methods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC
initmsiquery(void) 
{
    PyObject* m;

    if (PyType_Ready(&MsiRecordType) < 0)
        return;

    if (PyType_Ready(&MsiViewType) < 0)
        return;

    if (PyType_Ready(&MsiDBType) < 0)
        return;

    m = Py_InitModule3("msiquery", msiquery_methods, "Msi API module.");

    if (m == NULL)
      return;

    Py_INCREF(&MsiRecordType);
    PyModule_AddObject(m, "MsiRecord", (PyObject *)&MsiRecordType);

    Py_INCREF(&MsiViewType);
    PyModule_AddObject(m, "MsiView", (PyObject *)&MsiViewType);

    Py_INCREF(&MsiDBType);
    PyModule_AddObject(m, "MsiDB", (PyObject *)&MsiDBType);
}

