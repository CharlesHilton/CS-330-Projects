/* Author: Charles Hilton
            Charles.Hilton@snhu.edu
            Credits to ChatGPT for a bulk of this code.
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <GL/glew.h>        // GLEW library

using namespace std;

namespace Hilton {
    struct Vertex {
        float x, y, z;
    };

    struct TextureCoordinate {
        float u, v;
    };

    struct Normal {
        float x, y, z;
    };

    struct Face {
        GLuint v1, v2, v3;
        GLuint t1, t2, t3;
        GLuint n1, n2, n3;
    };

    struct VertexData {
        std::vector<Vertex> vertices;
        std::vector<TextureCoordinate> textureCoordinates;
        std::vector<Normal> normals;
        std::vector<Face> faces;
    };

    struct HiltonMesh {
        GLuint vao; // Vertex Array Object
        GLuint vbo; // Vertex Buffer Object
        GLuint tbo; // Texture Buffer Object
        GLuint nbo; // Normal Buffer Object
        GLuint ibo; // Indicies (Element) Buffer Object
        VertexData vertexData;
        std::vector<GLuint> indices;
    };

    int parseOBJFile(std::string const fileName, VertexData& vertexData);

    void bindVertexData(HiltonMesh& mesh, VertexData& vertexData);

    void bindVertexData(HiltonMesh& mesh);

    void HCreateMesh(std::string const fileName, HiltonMesh& mesh);

    void HDrawMesh(HiltonMesh& mesh);
}
