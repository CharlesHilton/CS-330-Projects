#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace Hilton {
    struct HMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
        GLfloat verts[];
    };


    template <size_t N>
    void UCreateMesh(HMesh& mesh, GLfloat(&verts)[N])
    {
        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;

        mesh.nVertices = N / (floatsPerVertex + floatsPerNormal + floatsPerUV);

        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }

}

    // Objects to render
    // -----------------
