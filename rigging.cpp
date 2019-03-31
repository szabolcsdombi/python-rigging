#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct Transform {
    glm::vec3 position;
    glm::mat3 rotation;
};

struct BoneVertex {
    union {
        int bone;
        struct {
            unsigned short bone_index;
            unsigned short bone_continue;
        };
    };
    glm::vec3 vert;
    glm::vec3 norm;
    float coeff;
};

struct SkinVertex {
    glm::vec3 vert;
    glm::vec3 norm;
};

void calculate_skin(int num_vertices, BoneVertex * input, Transform * bones, SkinVertex * output) {
    while (num_vertices--) {
        const BoneVertex & u = *input++;
        const Transform & b = bones[u.bone_index];
        SkinVertex & skin = *output++;
        skin.vert = b.position + b.rotation * u.vert;
        skin.norm = b.rotation * u.norm;
        if (u.bone_continue) {
            const BoneVertex & v = *input++;
            const Transform & b = bones[v.bone_index];
            skin.vert = skin.vert * u.coeff + (b.position + b.rotation * v.vert) * v.coeff;
            skin.norm = skin.norm * u.coeff + (b.rotation * v.norm) * v.coeff;
            while (input[-1].bone_continue) {
                const BoneVertex & w = *input++;
                const Transform & b = bones[w.bone_index];
                skin.vert += (b.position + b.rotation * w.vert) * w.coeff;
                skin.norm += (b.rotation * w.norm) * w.coeff;
            }
        }
        skin.norm = glm::normalize(skin.norm);
    }
}

PyObject * meth_skin(PyObject * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"source", "bones", NULL};

    Py_buffer source;
    PyObject * bones;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*O|O", keywords, &source, &bones)) {
        return 0;
    }

    int num_bones = 0;
    int num_vertices = 0;
    BoneVertex * bone_vertices = (BoneVertex *)source.buf;
    int num_bone_vertices = (int)(source.len / sizeof(BoneVertex));
    for (int i = 0; i < num_bone_vertices; ++i) {
        if (num_bones < bone_vertices[i].bone_index + 1) {
            num_bones = bone_vertices[i].bone_index + 1;
        }
        if (!bone_vertices[i].bone_continue) {
            num_vertices += 1;
        }
    }

    bones = PySequence_Fast(bones, "not iterable");
    if (!bones) {
        return 0;
    }

    if (PySequence_Fast_GET_SIZE(bones) != num_bones) {
        PyErr_Format(PyExc_Exception, "number of bones");
        return 0;
    }

    Transform * transforms = (Transform *)alloca(sizeof(Transform) * num_bones);
    for (int i = 0; i < num_bones; ++i) {
        PyObject * bone = PySequence_Fast(PySequence_Fast_GET_ITEM(bones, i), "not iterable");
        if (!bone) {
            return 0;
        }
        PyObject * position = PySequence_Fast_GET_ITEM(bone, 0);
        PyObject * rotation = PySequence_Fast_GET_ITEM(bone, 1);
        Py_DECREF(bone);

        if (position == Py_None) {
            transforms[i].position = glm::vec3(0.0f, 0.0f, 0.0f);
        } else {
            if (!PyArg_ParseTuple(position, "fff", &transforms[i].position.x, &transforms[i].position.y, &transforms[i].position.z)) {
                return 0;
            }
        }

        if (rotation == Py_None) {
            transforms[i].rotation = glm::mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        } else if (PyObject_Size(rotation) == 9) {
            float * ptr = (float *)&transforms[i].rotation;
            if (!PyArg_ParseTuple(rotation, "fffffffff", &ptr[0], &ptr[1], &ptr[2], &ptr[3], &ptr[4], &ptr[5], &ptr[6], &ptr[7], &ptr[8])) {
                return 0;
            }
        } else {
            glm::quat quat;
            if (!PyArg_ParseTuple(rotation, "ffff", &quat.x, &quat.y, &quat.z, &quat.w)) {
                return 0;
            }
            transforms[i].rotation = glm::mat3(glm::inverse(quat));
        }
    }
    Py_DECREF(bones);

    PyObject * res = PyBytes_FromStringAndSize(NULL, num_vertices * sizeof(SkinVertex));
    calculate_skin(num_vertices, (BoneVertex *)source.buf, transforms, (SkinVertex *)PyBytes_AS_STRING(res));
    PyBuffer_Release(&source);
    return res;
}

PyMethodDef module_methods[] = {
    {"skin", (PyCFunction)meth_skin, METH_VARARGS | METH_KEYWORDS, 0},
    {0},
};

PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "rigging", 0, -1, module_methods, 0, 0, 0, 0};

extern "C" PyObject * PyInit_rigging() {
    PyObject * module = PyModule_Create(&module_def);
    return module;
}
