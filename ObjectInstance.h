// Reconstructed minimal ObjectInstance implementation compatible with View.cpp usage
#ifndef _OBJECTINSTANCE_H_
#define _OBJECTINSTANCE_H_

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ShaderLocationsVault.h>
#include <PolygonMesh.h>

namespace util {

// Forward decl
template <typename V> class PolygonMesh;

class ObjectInstance {
public:
    explicit ObjectInstance(const std::string &name)
        : name(name), vao(0), vbo(0), ebo(0), primitive(GL_TRIANGLES), vertexCount(0), indexCount(0) {}

    ~ObjectInstance() { cleanup(); }

    void cleanup() {
        if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
        if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
        if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    }

    template <class V>
    void initPolygonMesh(const util::ShaderLocationsVault &shaderLocations,
                         const std::map<std::string, std::string> &shaderVarsToAttributeNames,
                         const util::PolygonMesh<V> &mesh) {
        // Gather mesh data
        const std::vector<V> &verts = mesh.getVertexAttributes();
        const std::vector<unsigned int> &indices = mesh.getPrimitives();

        if (verts.empty()) {
            std::cerr << "Warning: Mesh '" << name << "' has no vertex data. Skipping GPU upload." << std::endl;
            primitive = GL_TRIANGLES; vertexCount = 0; indexCount = 0; return;
        }

        // Build interleaved buffer according to shaderVarsToAttributeNames order
        // Determine per-vertex stride by querying first vertex
        int stride = 0;
        std::vector<std::string> attribOrder;
        attribOrder.reserve(shaderVarsToAttributeNames.size());
        for (auto it = shaderVarsToAttributeNames.begin(); it != shaderVarsToAttributeNames.end(); ++it) {
            const std::string &attrib = it->second; // e.g., "position"
            // Request data from first vertex to compute size (in floats)
            std::vector<float> data = verts[0].getData(attrib);
            stride += static_cast<int>(data.size());
            attribOrder.push_back(attrib);
        }
        stride *= sizeof(float);

        // Create a tightly packed interleaved array
        std::vector<float> interleaved;
        interleaved.reserve(verts.size() * (stride / sizeof(float)));
        for (const auto &v : verts) {
            for (const auto &attrib : attribOrder) {
                std::vector<float> data = v.getData(attrib);
                interleaved.insert(interleaved.end(), data.begin(), data.end());
            }
        }

        // Create VAO/VBO/EBO
        cleanup();
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

        // Set vertex attrib pointers according to shaderLocations
        GLsizei floatStride = static_cast<GLsizei>(stride);
        GLsizei offsetBytes = 0;
        for (auto it = shaderVarsToAttributeNames.begin(); it != shaderVarsToAttributeNames.end(); ++it) {
            const std::string &shaderVar = it->first;  // e.g., vPosition
            const std::string &attrib    = it->second; // e.g., position
            GLint loc = shaderLocations.getLocation(shaderVar);
            if (loc < 0) { continue; }
            // Determine size by checking first vertex
            std::vector<float> data = verts[0].getData(attrib);
            GLint comps = static_cast<GLint>(data.size());
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, comps, GL_FLOAT, GL_FALSE, floatStride, reinterpret_cast<void*>(static_cast<uintptr_t>(offsetBytes)));
            offsetBytes += comps * sizeof(float);
        }

        // Upload EBO if indices present
        indexCount = static_cast<GLsizei>(indices.size());
        if (!indices.empty()) {
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }

        glBindVertexArray(0);

        vertexCount = static_cast<GLsizei>(verts.size());
        primitive = GL_TRIANGLES;
    }

    void draw() const {
        if (!vao) return;
        glBindVertexArray(vao);
        if (indexCount > 0) {
            glDrawElements(primitive, indexCount, GL_UNSIGNED_INT, 0);
        } else if (vertexCount > 0) {
            glDrawArrays(primitive, 0, vertexCount);
        }
        glBindVertexArray(0);
    }

private:
    std::string name;
    GLuint vao, vbo, ebo;
    GLenum primitive;
    GLsizei vertexCount;
    GLsizei indexCount;
};

} // namespace util

#endif // _OBJECTINSTANCE_H_