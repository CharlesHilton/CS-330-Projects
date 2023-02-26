#include "HiltonMesher.hpp"

int Hilton::parseOBJFile(std::string fileName, VertexData& vertexData) {

    std::unordered_map<std::string, int> lineTypes = {
        { "v", 0 },
        { "vt", 1 },
        { "vn", 2 },
        { "f", 3 },
        { "o", 4}
    };

    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << fileName << std::endl;
        return false;
    }
    else
    {
        cout << "Opened " << fileName << std::endl;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream lineStream(line);
        std::string type;
        lineStream >> type;

        switch (lineTypes[type]) {
        case 0: {
            GLfloat x, y, z;
            lineStream >> x >> y >> z;
            vertexData.vertices.push_back({ x, y, z });
            break;
        }
        case 1: {
            GLfloat u, v;
            lineStream >> u >> v;
            vertexData.textureCoordinates.push_back({ u, v });
            break;
        }
        case 2: {
            GLfloat x, y, z;
            lineStream >> x >> y >> z;
            vertexData.normals.push_back({ x, y, z });
            break;
        }
        case 3: {
            GLuint v1, v2, v3, t1 = 0, t2 = 0, t3 = 0, n1 = 0, n2 = 0, n3 = 0;
            char discard;
            lineStream >> v1 >> discard >> t1 >> discard >> n1
                >> v2 >> discard >> t2 >> discard >> n2
                >> v3 >> discard >> t3 >> discard >> n3;
            vertexData.faces.push_back({ v1 - 1, v2 - 1, v3 - 1, t1 - 1, t2 - 1, t3 - 1, n1 - 1, n2 - 1, n3 - 1 });
            break;
        }
        case 4:
            break;
        default:
            //cout << "Line not read: " << line << endl;
            break;
        }
    }

    std::cout << "Number of faces: " << vertexData.faces.size() << std::endl;
    return 0;
}

void Hilton::bindVertexData(HiltonMesh& mesh, VertexData& vertexData) {

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create a VBO for the vertex data
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.vertices.size() * sizeof(Vertex), vertexData.vertices.data(), GL_STATIC_DRAW);

    // Create a VBO for the texture coordinate data
    glGenBuffers(1, &mesh.tbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.tbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.textureCoordinates.size() * sizeof(TextureCoordinate), vertexData.textureCoordinates.data(), GL_STATIC_DRAW);

    // Create a VBO for the normal data
    glGenBuffers(1, &mesh.nbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.nbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.normals.size() * sizeof(Normal), vertexData.normals.data(), GL_STATIC_DRAW);

    // Create an IBO for the face data
    //GLuint indexBuffer;
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    //std::vector<unsigned int> indices;
    for (const auto& face : vertexData.faces) {
        mesh.indices.push_back(face.v1);
        mesh.indices.push_back(face.v2);
        mesh.indices.push_back(face.v3);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
}

void Hilton::bindVertexData(HiltonMesh& mesh)
{
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create a VBO for the vertex data
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertexData.vertices.size() * sizeof(Vertex), mesh.vertexData.vertices.data(), GL_STATIC_DRAW);

    // Create a VBO for the texture coordinate data
    glGenBuffers(1, &mesh.tbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.tbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertexData.textureCoordinates.size() * sizeof(TextureCoordinate), mesh.vertexData.textureCoordinates.data(), GL_STATIC_DRAW);

    // Create a VBO for the normal data
    glGenBuffers(1, &mesh.nbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.nbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertexData.normals.size() * sizeof(Normal), mesh.vertexData.normals.data(), GL_STATIC_DRAW);

    // Create an IBO for the face data
    //GLuint indexBuffer;
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    //std::vector<unsigned int> indices;
    for (const auto& face : mesh.vertexData.faces) {
        mesh.indices.push_back(face.v1);
        mesh.indices.push_back(face.v2);
        mesh.indices.push_back(face.v3);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
}

void Hilton::HCreateMesh(std::string const fileName, HiltonMesh& mesh) {
    VertexData vertexData;

    parseOBJFile(fileName, vertexData);

    mesh.vertexData = vertexData;

    bindVertexData(mesh);
}

void Hilton::HDrawMesh(HiltonMesh& mesh) {
    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.nbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Normal), 0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.tbo);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TextureCoordinate), 0);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
}
