#ifndef _RAYCASTRENDERER_H_
#define _RAYCASTRENDERER_H_

#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include "IScenegraph.h"
#include "../Ray.h"
#include "../HitRecord.h"
#include "PolygonMesh.h"
#include "../VertexAttrib.h"
#include "Light.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stack>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <limits>
using namespace std;

namespace sgraph {

/**
 * This visitor implements ray casting for the scene graph.
 * It follows the same design pattern as GLScenegraphRenderer.
 * 
 * For each ray, it traverses the scene graph and finds the closest
 * intersection point with any object.
 */
class RaycastRenderer : public SGNodeVisitor {
public:
    /**
     * @brief Construct a new RaycastRenderer object
     * 
     * @param mv Reference to modelview stack (same as OpenGL renderer)
     * @param meshes Map of mesh names to PolygonMesh objects
     */
    RaycastRenderer(stack<glm::mat4>& mv, 
                    map<string, util::PolygonMesh<VertexAttrib>>& meshes,
                    vector<util::Light>& lights)
        : modelview(mv)
        , meshes(meshes)
        , lights(lights) {
    }

    /**
     * @brief Raytrace the entire scene and save to PPM file
     * 
     * @param scenegraph The scene graph to render
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param filename Output PPM filename
     */
    void raytrace(IScenegraph* scenegraph, int width, int height, const string& filename) {
        cout << "Starting ray tracing..." << endl;
        cout << "Image size: " << width << " x " << height << endl;
        cout << "This may take a while without KD-tree acceleration..." << endl;
        cout.flush();

        // Create image buffer (RGB for each pixel)
        vector<unsigned char> image(width * height * 3);

        // Camera setup (same as OpenGL)
        float fovy = glm::radians(60.0f);
        float aspect = (float)width / height;
        float halfHeight = tan(fovy / 2.0f);
        float halfWidth = halfHeight * aspect;

        // For each pixel
        for (int j = 0; j < height; j++) {
            // Progress indicator - print every 10 rows
            if (j % 10 == 0) {
                cout << "Progress: " << (j * 100 / height) << "% (row " << j << "/" << height << ")" << endl;
                cout.flush();
            }
            for (int i = 0; i < width; i++) {
                // Step 1: Compute the ray starting from the eye and passing through this pixel
                // Convert pixel coordinates to normalized device coordinates [-1, 1]
                float ndcX = (2.0f * (i + 0.5f) / width) - 1.0f;
                float ndcY = 1.0f - (2.0f * (j + 0.5f) / height);

                // Convert to view space coordinates on image plane (at z = -1)
                float viewX = ndcX * halfWidth;
                float viewY = ndcY * halfHeight;

                // Ray in view space: starts at origin, points towards pixel
                glm::vec4 rayStart(0.0f, 0.0f, 0.0f, 1.0f);
                glm::vec4 rayDir(viewX, viewY, -1.0f, 0.0f);
                rayDir = glm::normalize(rayDir);

                Ray viewRay(rayStart, rayDir);

                // Step 2: Cast the ray into the scene graph
                HitRecord hitRecord = raycast(viewRay, scenegraph);

                // Step 3: Compute the color of the pixel based on ray casting results
                glm::vec3 color;
                if (hitRecord.hasHit()) {
                    // Use Phong shading to compute color
                    color = shade(hitRecord);
                } else {
                    // Background color (black)
                    color = glm::vec3(0.0f, 0.0f, 0.0f);
                }

                // Step 4: Write the color to the appropriate place in the array
                // Clamp color values to [0, 1] and convert to [0, 255]
                color = glm::clamp(color, 0.0f, 1.0f);
                int index = (j * width + i) * 3;
                image[index] = static_cast<unsigned char>(color.r * 255);
                image[index + 1] = static_cast<unsigned char>(color.g * 255);
                image[index + 2] = static_cast<unsigned char>(color.b * 255);
            }
        }
        
        cout << "Progress: 100%" << endl;

        // Write image to PPM file
        writePPM(image, width, height, filename);
        cout << "Ray tracing complete! Output saved to: " << filename << endl;
    }

    /**
     * @brief Cast a ray into the scene graph and find the closest intersection
     * 
     * @param viewRay Ray in view coordinates
     * @param scenegraph The scene graph
     * @return HitRecord with intersection information
     */
    HitRecord raycast(const Ray& viewRay, IScenegraph* scenegraph) {
        // Store the ray for use during traversal
        this->currentRay = viewRay;
        this->closestHit = HitRecord();  // Reset to no hit

        // Push identity matrix and traverse the scene graph
        // The modelview stack already has the view transform
        scenegraph->getRoot()->accept(this);

        return closestHit;
    }

    // ===================== Visitor Methods =====================

    /**
     * @brief Visit group node - recurse to all children
     */
    void visitGroupNode(GroupNode* groupNode) override {
        for (int i = 0; i < groupNode->getChildren().size(); i++) {
            groupNode->getChildren()[i]->accept(this);
        }
    }

    /**
     * @brief Visit leaf node - test ray intersection with the mesh
     */
    void visitLeafNode(LeafNode* leafNode) override {
        string meshName = leafNode->getInstanceOf();
        
        if (meshes.find(meshName) == meshes.end()) {
            return;  // Mesh not found
        }

        util::PolygonMesh<VertexAttrib>& mesh = meshes[meshName];
        util::Material material = leafNode->getMaterial();
        string textureName = leafNode->getTextureName();

        // Get the current modelview matrix
        glm::mat4 modelviewMatrix = modelview.top();
        glm::mat4 inverseModelview = glm::inverse(modelviewMatrix);
        glm::mat4 normalMatrix = glm::transpose(inverseModelview);

        // Transform ray from view space to object space
        glm::vec4 objRayStart = inverseModelview * currentRay.getStart();
        glm::vec4 objRayDir = inverseModelview * currentRay.getDirection();
        // Note: direction doesn't need normalization for intersection test
        // but we normalize for consistency
        objRayDir.w = 0.0f;  // Ensure it's a direction vector
        
        Ray objectRay(objRayStart, objRayDir);

        // Test intersection with all triangles in the mesh
        HitRecord hit = intersectMesh(objectRay, mesh, modelviewMatrix, normalMatrix, material, textureName);

        // Keep the closest hit
        if (hit.hasHit() && hit.getT() < closestHit.getT()) {
            closestHit = hit;
        }
    }

    /**
     * @brief Visit transform node - multiply transform and recurse
     */
    void visitTransformNode(TransformNode* transformNode) override {
        modelview.push(modelview.top());
        modelview.top() = modelview.top() * transformNode->getTransform();
        
        if (transformNode->getChildren().size() > 0) {
            transformNode->getChildren()[0]->accept(this);
        }
        
        modelview.pop();
    }

    void visitScaleTransform(ScaleTransform* scaleNode) override {
        visitTransformNode(scaleNode);
    }

    void visitTranslateTransform(TranslateTransform* translateNode) override {
        visitTransformNode(translateNode);
    }

    void visitRotateTransform(RotateTransform* rotateNode) override {
        visitTransformNode(rotateNode);
    }

private:
    stack<glm::mat4>& modelview;
    map<string, util::PolygonMesh<VertexAttrib>>& meshes;
    vector<util::Light>& lights;
    
    Ray currentRay;        // Current ray being cast (in view space)
    HitRecord closestHit;  // Closest intersection found so far

    /**
     * @brief Test ray intersection with all triangles in a mesh
     */
    HitRecord intersectMesh(const Ray& objectRay, 
                            util::PolygonMesh<VertexAttrib>& mesh,
                            const glm::mat4& modelviewMatrix,
                            const glm::mat4& normalMatrix,
                            const util::Material& material,
                            const string& textureName) {
        HitRecord closestHit;
        
        vector<VertexAttrib> vertices = mesh.getVertexAttributes();
        vector<unsigned int> indices = mesh.getPrimitives();
        int primitiveSize = mesh.getPrimitiveSize();

        // Only handle triangles (primitiveSize == 3)
        if (primitiveSize != 3) {
            return closestHit;
        }

        // Pre-extract all vertex data to avoid repeated getData() calls
        vector<glm::vec3> positions(vertices.size());
        vector<glm::vec3> normals(vertices.size());
        vector<glm::vec2> texcoords(vertices.size());
        
        for (size_t v = 0; v < vertices.size(); v++) {
            vector<float> pos = vertices[v].getData("position");
            vector<float> norm = vertices[v].getData("normal");
            vector<float> tex = vertices[v].getData("texcoord");
            positions[v] = glm::vec3(pos[0], pos[1], pos[2]);
            normals[v] = glm::vec3(norm[0], norm[1], norm[2]);
            texcoords[v] = glm::vec2(tex[0], tex[1]);
        }

        // Iterate through all triangles
        for (size_t i = 0; i < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i+1];
            unsigned int i2 = indices[i+2];
            
            // Get vertex positions
            glm::vec4 v0 = glm::vec4(positions[i0], 1.0f);
            glm::vec4 v1 = glm::vec4(positions[i1], 1.0f);
            glm::vec4 v2 = glm::vec4(positions[i2], 1.0f);

            // Test ray-triangle intersection
            float t;
            glm::vec3 baryCoords;
            if (rayTriangleIntersect(objectRay, v0, v1, v2, t, baryCoords)) {
                if (t > 0.0001f && t < closestHit.getT()) {
                    // Compute intersection point in object space
                    glm::vec4 objIntersection = objectRay.getPointAt(t);
                    
                    // Transform intersection point to view space
                    glm::vec4 viewIntersection = modelviewMatrix * objIntersection;

                    // Get vertex normals
                    glm::vec4 n0 = glm::vec4(normals[i0], 0.0f);
                    glm::vec4 n1 = glm::vec4(normals[i1], 0.0f);
                    glm::vec4 n2 = glm::vec4(normals[i2], 0.0f);

                    // Interpolate normal using barycentric coordinates
                    glm::vec4 objNormal = baryCoords.x * n0 + baryCoords.y * n1 + baryCoords.z * n2;
                    objNormal.w = 0.0f;
                    objNormal = glm::normalize(objNormal);

                    // Transform normal to view space
                    glm::vec4 viewNormal = normalMatrix * objNormal;
                    viewNormal.w = 0.0f;
                    viewNormal = glm::normalize(viewNormal);

                    // Fill hit record
                    closestHit.setT(t);
                    closestHit.setIntersectionPoint(viewIntersection);
                    closestHit.setNormal(viewNormal);
                    closestHit.setMaterial(material);
                    closestHit.setTextureName(textureName);

                    // Interpolate texture coordinates
                    glm::vec2 tc = baryCoords.x * texcoords[i0] + 
                                   baryCoords.y * texcoords[i1] + 
                                   baryCoords.z * texcoords[i2];
                    closestHit.setTextureCoordinates(tc);
                }
            }
        }

        return closestHit;
    }

    /**
     * @brief Moller-Trumbore ray-triangle intersection algorithm
     * 
     * @param ray The ray to test
     * @param v0, v1, v2 Triangle vertices
     * @param t Output: parameter t at intersection
     * @param baryCoords Output: barycentric coordinates (u, v, w) where w = 1 - u - v
     * @return true if intersection exists
     */
    bool rayTriangleIntersect(const Ray& ray, 
                              const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2,
                              float& t, glm::vec3& baryCoords) {
        const float EPSILON = 0.0000001f;
        
        glm::vec3 rayOrigin = glm::vec3(ray.getStart());
        glm::vec3 rayDir = glm::vec3(ray.getDirection());
        glm::vec3 vertex0 = glm::vec3(v0);
        glm::vec3 vertex1 = glm::vec3(v1);
        glm::vec3 vertex2 = glm::vec3(v2);

        glm::vec3 edge1 = vertex1 - vertex0;
        glm::vec3 edge2 = vertex2 - vertex0;
        glm::vec3 h = glm::cross(rayDir, edge2);
        float a = glm::dot(edge1, h);

        // Ray is parallel to triangle
        if (a > -EPSILON && a < EPSILON) {
            return false;
        }

        float f = 1.0f / a;
        glm::vec3 s = rayOrigin - vertex0;
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f) {
            return false;
        }

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(rayDir, q);

        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }

        // Compute t to find intersection point
        t = f * glm::dot(edge2, q);

        if (t > EPSILON) {
            // Barycentric coordinates: (1-u-v, u, v) for vertices (v0, v1, v2)
            baryCoords = glm::vec3(1.0f - u - v, u, v);
            return true;
        }

        return false;
    }

    /**
     * @brief Compute shading at a hit point using Phong lighting model
     */
    glm::vec3 shade(const HitRecord& hit) {
        glm::vec3 color(0.0f);
        
        util::Material material = hit.getMaterial();
        glm::vec3 viewPos = glm::vec3(hit.getIntersectionPoint());
        glm::vec3 normal = glm::normalize(glm::vec3(hit.getNormal()));
        
        // View direction (from intersection point to camera, which is at origin in view space)
        glm::vec3 viewDir = glm::normalize(-viewPos);

        // Ensure normal faces the camera
        if (glm::dot(normal, viewDir) < 0) {
            normal = -normal;
        }

        // Accumulate lighting from all lights
        for (const util::Light& light : lights) {
            glm::vec3 ambient = glm::vec3(light.getAmbient()) * glm::vec3(material.getAmbient());
            
            // Light direction
            glm::vec3 lightPos = glm::vec3(light.getPosition());
            glm::vec3 lightDir;
            
            if (light.getPosition().w == 0.0f) {
                // Directional light
                lightDir = glm::normalize(-lightPos);
            } else {
                // Point light
                lightDir = glm::normalize(lightPos - viewPos);
            }

            // Diffuse
            float diff = max(glm::dot(normal, lightDir), 0.0f);
            glm::vec3 diffuse = diff * glm::vec3(light.getDiffuse()) * glm::vec3(material.getDiffuse());

            // Specular (Blinn-Phong)
            glm::vec3 halfDir = glm::normalize(lightDir + viewDir);
            float spec = pow(max(glm::dot(normal, halfDir), 0.0f), material.getShininess());
            glm::vec3 specular = spec * glm::vec3(light.getSpecular()) * glm::vec3(material.getSpecular());

            // Spotlight effect
            float spotEffect = 1.0f;
            if (light.getSpotCutoff() < 180.0f) {
                glm::vec3 spotDir = glm::normalize(glm::vec3(light.getSpotDirection()));
                float cosAngle = glm::dot(-lightDir, spotDir);
                float cosCutoff = cos(glm::radians(light.getSpotCutoff()));
                
                if (cosAngle < cosCutoff) {
                    spotEffect = 0.0f;
                } else {
                    spotEffect = pow(cosAngle, 1.0f);  // Can add spotlight exponent
                }
            }

            color += ambient + spotEffect * (diffuse + specular);
        }

        return color;
    }

    /**
     * @brief Write image data to PPM file
     */
    void writePPM(const vector<unsigned char>& image, int width, int height, const string& filename) {
        ofstream outFile(filename, ios::binary);
        if (outFile.is_open()) {
            outFile << "P6\n" << width << " " << height << "\n255\n";
            outFile.write(reinterpret_cast<const char*>(image.data()), image.size());
            outFile.close();
        } else {
            cerr << "Error: Could not open file for writing: " << filename << endl;
        }
    }
};

}  // namespace sgraph

#endif
