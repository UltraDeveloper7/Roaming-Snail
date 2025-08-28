#include "../precompiled.h"
#include "Drawable.hpp"
#include <tinyxml2.h>
#include <tiny_obj_loader.h>
#include <map>
#include <tuple>
#include <unordered_map>
#include <iostream>
#include <sstream>

using namespace glm;
using namespace std;
using namespace tinyxml2;

Drawable* lightbulb = nullptr;

Drawable::Drawable(string path) {
    if (path.substr(path.size() - 3, 3) == "obj") {
        loadOBJWithTiny(path.c_str(), vertices, uvs, normals, VEC_UINT_DEFAUTL_VALUE);
    }
    else if (path.substr(path.size() - 3, 3) == "vtp") {
        loadVTP(path.c_str(), vertices, uvs, normals, VEC_UINT_DEFAUTL_VALUE);
    }
    else {
        throw runtime_error("File format not supported: " + path);
    }

    createContext();
}

Drawable::Drawable(const vector<vec3>& vertices, const vector<vec2>& uvs,
    const vector<vec3>& normals) : vertices(vertices), uvs(uvs), normals(normals) {
    createContext();
}

Drawable::~Drawable() {
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &uvsVBO);
    glDeleteBuffers(1, &normalsVBO);
    glDeleteBuffers(1, &elementVBO);
    glDeleteBuffers(1, &VAO);
}

void Drawable::bind() {
    glBindVertexArray(VAO);
}

void Drawable::draw(int mode) {
    glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, NULL);
}

void Drawable::createContext() {
    indices = vector<unsigned int>();
    indexVBO(vertices, uvs, normals, indices, indexedVertices, indexedUVS, indexedNormals);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, indexedVertices.size() * sizeof(vec3),
        &indexedVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    if (indexedNormals.size() != 0) {
        glGenBuffers(1, &normalsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
        glBufferData(GL_ARRAY_BUFFER, indexedNormals.size() * sizeof(vec3),
            &indexedNormals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }

    if (indexedUVS.size() != 0) {
        glGenBuffers(1, &uvsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, uvsVBO);
        glBufferData(GL_ARRAY_BUFFER, indexedUVS.size() * sizeof(vec2),
            &indexedUVS[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
    }

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);
}

// Define the function here
void loadOBJWithTiny(const char* path, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices) {
    // Implementation of the function
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);

    if (!warn.empty()) {
        std::cerr << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        throw std::runtime_error("Failed to load/parse .obj file");
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                vertices.push_back(glm::vec3(vx, vy, vz));

                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                    normals.push_back(glm::vec3(nx, ny, nz));
                }

                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    uvs.push_back(glm::vec2(tx, ty));
                }

                indices.push_back(static_cast<unsigned int>(index_offset + v));
            }
            index_offset += fv;
        }
    }
}

void loadVTP(const char* path, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices) {
    indices.clear();
    XMLDocument vtp;
    auto res = vtp.LoadFile(path);
    if (res != XML_SUCCESS) {
        throw runtime_error("Failed to load VTP file");
    }

    XMLElement* root = vtp.FirstChildElement("VTKFile");
    if (!root || !root->Attribute("type", "PolyData")) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* polydata = root->FirstChildElement("PolyData");
    if (!polydata) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* piece = polydata->FirstChildElement("Piece");
    if (!piece) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* enormals = piece->FirstChildElement("PointData");
    if (!enormals) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* points = piece->FirstChildElement("Points");
    if (!points) {
        throw runtime_error("Invalid VTP file format");
    }

    int numPoints, numPolys;
    piece->QueryIntAttribute("NumberOfPoints", &numPoints);
    piece->QueryIntAttribute("NumberOfPolys", &numPolys);

    const char* normalsStr = enormals->FirstChildElement("DataArray")->FirstChild()->Value();
    stringstream sNorm(normalsStr);
    vector<vec3> tempNormals;
    do {
        vec3 normal;
        sNorm >> normal.x >> normal.y >> normal.z;
        tempNormals.push_back(normal);
    } while (sNorm.good());
    tempNormals.pop_back(); // Remove the last corrupted element
    if (tempNormals.size() != numPoints) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* pointData = points->FirstChildElement("DataArray");
    if (!pointData || !pointData->Attribute("format", "ascii")) {
        throw runtime_error("Invalid VTP file format");
    }

    const char* coordsStr = points->FirstChildElement("DataArray")->FirstChild()->Value();
    stringstream sCoord(coordsStr);
    vector<vec3> coordinates;
    do {
        vec3 coord;
        sCoord >> coord.x >> coord.y >> coord.z;
        coordinates.push_back(coord);
    } while (sCoord.good());
    coordinates.pop_back(); // Remove the last corrupted element
    if (coordinates.size() != numPoints) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* polys = piece->FirstChildElement("Polys");
    if (!polys) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* econnectivity = polys->FirstChildElement("DataArray");
    if (!econnectivity || !econnectivity->Attribute("Name", "connectivity") || !econnectivity->Attribute("format", "ascii")) {
        throw runtime_error("Invalid VTP file format");
    }

    XMLElement* eoffsets = polys->LastChildElement("DataArray");
    if (!eoffsets || !eoffsets->Attribute("Name", "offsets") || !eoffsets->Attribute("format", "ascii")) {
        throw runtime_error("Invalid VTP file format");
    }

    const char* offsetsStr = eoffsets->FirstChild()->Value();
    stringstream sOffsets(offsetsStr);
    vector<int> offsets;
    do {
        int offset;
        sOffsets >> offset;
        offsets.push_back(offset);
    } while (sOffsets.good());
    offsets.pop_back(); // Remove the last corrupted element
    if (offsets.size() != numPolys) {
        throw runtime_error("Invalid VTP file format");
    }

    const char* connStr = econnectivity->FirstChild()->Value();
    stringstream sConn(connStr);
    vector<int> connectivity;
    do {
        int conn;
        sConn >> conn;
        connectivity.push_back(conn);
    } while (sConn.good());
    connectivity.pop_back(); // Remove the last corrupted element
    if (connectivity.size() != offsets.back()) {
        throw runtime_error("Invalid VTP file format");
    }

    int startPoly = 0;
    for (int i = 0; i < numPolys; ++i) {
        vector<int> face(connectivity.begin() + startPoly, connectivity.begin() + offsets[i]);
        int i1 = 0, i2 = 1, i3 = 2;
        while (i3 < face.size()) {
            vertices.push_back(coordinates[face[i1]]);
            normals.push_back(tempNormals[face[i1]]);
            indices.push_back(static_cast<unsigned int>(indices.size()));
            vertices.push_back(coordinates[face[i2]]);
            normals.push_back(tempNormals[face[i2]]);
            indices.push_back(static_cast<unsigned int>(indices.size()));
            vertices.push_back(coordinates[face[i3]]);
            normals.push_back(tempNormals[face[i3]]);
            indices.push_back(static_cast<unsigned int>(indices.size()));
            i2++;
            i3++;
        }
        startPoly = offsets[i];
    }
}

void indexVBO(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec2>& uvs, const std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices, std::vector<glm::vec3>& indexedVertices, std::vector<glm::vec2>& indexedUVS, std::vector<glm::vec3>& indexedNormals) {
    struct Vec3Hash {
        std::size_t operator()(const glm::vec3& v) const {
            std::size_t h1 = std::hash<float>()(v.x);
            std::size_t h2 = std::hash<float>()(v.y);
            std::size_t h3 = std::hash<float>()(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    std::unordered_map<glm::vec3, unsigned int, Vec3Hash> vertexToIndexMap;

    for (size_t i = 0; i < vertices.size(); ++i) {
        glm::vec3 vertex = vertices[i];
        glm::vec2 uv = uvs.size() > i ? uvs[i] : glm::vec2(0.0f, 0.0f);
        glm::vec3 normal = normals.size() > i ? normals[i] : glm::vec3(0.0f, 0.0f, 0.0f);

        if (vertexToIndexMap.count(vertex) == 0) {
            indexedVertices.push_back(vertex);
            indexedUVS.push_back(uv);
            indexedNormals.push_back(normal);
            vertexToIndexMap[vertex] = static_cast<unsigned int>(indexedVertices.size() - 1);
        }

        indices.push_back(vertexToIndexMap[vertex]);
    }
}

