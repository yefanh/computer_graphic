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
#include "../RaytraceMesh.h"
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
#include <cmath>
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
        // For each mesh being used by the scene graph, 
        // create a corresponding RaytraceMesh object
        for (auto& pair : meshes) {
            raytraceMeshes[pair.first] = new RaytraceMesh(pair.second);
        }
    }

    /**
     * @brief Destructor - clean up RaytraceMesh objects
     */
    ~RaycastRenderer() {
        for (auto& pair : raytraceMeshes) {
            delete pair.second;
        }
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
                    color = shade(hitRecord, scenegraph);
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
     * 
     * Identifies the RaytraceMesh object associated with the leaf
     * and processes the ray using the corresponding raytraceable object.
     */
    void visitLeafNode(LeafNode* leafNode) override {
        string meshName = leafNode->getInstanceOf();
        
        // Find the corresponding RaytraceMesh object
        if (raytraceMeshes.find(meshName) == raytraceMeshes.end()) {
            return;  // RaytraceMesh not found
        }

        RaytraceMesh* raytraceMesh = raytraceMeshes[meshName];
        util::Material material = leafNode->getMaterial();
        string textureName = leafNode->getTextureName();

        // Get the current modelview matrix
        glm::mat4 modelviewMatrix = modelview.top();
        glm::mat4 inverseModelview = glm::inverse(modelviewMatrix);
        glm::mat4 normalMatrix = glm::transpose(inverseModelview);

        // Transform ray from view space to object space
        glm::vec4 objRayStart = inverseModelview * currentRay.getStart();
        glm::vec4 objRayDir = inverseModelview * currentRay.getDirection();
        objRayDir.w = 0.0f;  // Ensure it's a direction vector
        
        Ray objectRay(objRayStart, objRayDir);

        // Use RaytraceMesh to find intersection
        // Pass both object space ray and view space ray (for convenience)
        HitRecord hit = raytraceMesh->intersect(objectRay, currentRay, 
                                                 modelviewMatrix, normalMatrix, 
                                                 material, textureName);

        // Keep the closest hit using view-space distance when available
        if (hit.hasHit()) {
            float candidateDist = hit.getViewT();
            if (!std::isfinite(candidateDist)) {
                candidateDist = hit.getT();
            }
            float currentDist = closestHit.getViewT();
            if (!std::isfinite(currentDist)) {
                currentDist = closestHit.getT();
            }
            if (candidateDist < currentDist) {
                closestHit = hit;
            }
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
    map<string, RaytraceMesh*> raytraceMeshes;  // RaytraceMesh for each mesh
    vector<util::Light>& lights;
    
    Ray currentRay;        // Current ray being cast (in view space)
    HitRecord closestHit;  // Closest intersection found so far

    /**
     * @brief Compute shading at a hit point using Phong lighting model
     * 
     * Section 3: Replicates the lighting shader from previous assignment.
     * Supports point/directional lights, spot lights, and ambient/diffuse/specular.
     * Does NOT include texture mapping as per assignment requirements.
     * 
     * Section 3.1: Uses interpolated normals from barycentric coordinates.
     */
    glm::vec3 shade(const HitRecord& hit, IScenegraph* scenegraph) {
        return shadeRecursive(hit, scenegraph, 0);
    }

    glm::vec3 shadeRecursive(const HitRecord& hit, IScenegraph* scenegraph, int bounce) {
        const int MAX_BOUNCES = 5;
        if (bounce > MAX_BOUNCES) {
            return glm::vec3(0.0f);
        }

        glm::vec3 color(0.0f);
        
        util::Material material = hit.getMaterial();
        glm::vec3 viewPos = glm::vec3(hit.getIntersectionPoint());
        glm::vec3 normal = glm::normalize(glm::vec3(hit.getNormal()));
        
        // View direction (from intersection point to camera, which is at origin in view space)
        // Same as shader: viewVec = -fPosition.xyz
        glm::vec3 viewDir = glm::normalize(-viewPos);

        // Ensure normal faces the camera
        if (glm::dot(normal, viewDir) < 0) {
            normal = -normal;
        }

        // Accumulate lighting from all lights (same as shader loop)
        for (const util::Light& light : lights) {
            // Light direction calculation (same as shader)
            glm::vec3 lightPos = glm::vec3(light.getPosition());
            glm::vec3 lightDir;
            float distToLight = std::numeric_limits<float>::infinity();
            
            if (light.getPosition().w != 0.0f) {
                // Point light: lightVec = normalize(light.position.xyz - fPosition.xyz)
                lightDir = glm::normalize(lightPos - viewPos);
                // measure from the offset shadow origin to the light, in the same space as shadow ray
                distToLight = glm::length(lightPos - viewPos);
            } else {
                // Directional light: lightVec = normalize(-light.position.xyz)
                lightDir = glm::normalize(-lightPos);
            }

            // Shadow check
            bool inShadow = false;
            // Fudge factor to avoid self-intersection using surface normal
            const float SHADOW_EPSILON = 0.001f;
            glm::vec3 shadowRayStart = viewPos + SHADOW_EPSILON * normal; 
            Ray shadowRay(glm::vec4(shadowRayStart, 1.0f), glm::vec4(lightDir, 0.0f));
            
            // Cast shadow ray
            HitRecord shadowHit = raycast(shadowRay, scenegraph);
            
            if (shadowHit.hasHit()) {
                // For point lights, check if obstacle is closer than light along this ray
                // For directional lights, any hit means shadow
                if (light.getPosition().w != 0.0f) {
                    float maxT = distToLight - SHADOW_EPSILON;
                    if (shadowHit.getT() > 0.0f && shadowHit.getT() < maxT) {
                        inShadow = true;
                    }
                } else {
                    // Directional light - any hit is shadow
                    inShadow = true;
                }
            }

            // nDotL for diffuse
            float nDotL = glm::dot(normal, lightDir);

            // Ambient: material.ambient * light.ambient
            glm::vec3 ambient = glm::vec3(material.getAmbient()) * glm::vec3(light.getAmbient());
            
            glm::vec3 diffuse(0.0f);
            glm::vec3 specular(0.0f);

            if (!inShadow) {
                // Diffuse: material.diffuse * light.diffuse * max(nDotL, 0)
                diffuse = glm::vec3(material.getDiffuse()) * glm::vec3(light.getDiffuse()) * glm::max(nDotL, 0.0f);

                // Specular using Phong reflection model (same as shader: reflect(-lightVec, normal))
                if (nDotL > 0.0f) {
                    // reflectVec = reflect(-lightVec, normalView)
                    glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
                    reflectDir = glm::normalize(reflectDir);
                    
                    // rDotV = max(dot(reflectVec, viewVec), 0.0)
                    float rDotV = glm::max(glm::dot(reflectDir, viewDir), 0.0f);
                    
                    // specular = material.specular * light.specular * pow(rDotV, shininess)
                    specular = glm::vec3(material.getSpecular()) * glm::vec3(light.getSpecular()) 
                            * glm::pow(rDotV, material.getShininess());
                }
            }

            // Spotlight effect (if spotlight is enabled)
            float spotEffect = 1.0f;
            if (light.getSpotCutoff() < 180.0f) {
                // Spotlight direction
                glm::vec3 spotDir = glm::normalize(glm::vec3(light.getSpotDirection()));
                // cosAngle between light direction and spotlight direction
                float cosAngle = glm::dot(-lightDir, spotDir);
                // cosSpotCutoff from Light class
                float cosCutoff = cos(glm::radians(light.getSpotCutoff()));
                
                if (cosAngle < cosCutoff) {
                    // Outside spotlight cone
                    spotEffect = 0.0f;
                }
                // Note: Could add spotlight exponent for soft edges
            }

            // Accumulate: ambient + diffuse + specular (with spotlight attenuation)
            color += ambient + spotEffect * (diffuse + specular);
        }

        // Reflection
        if (material.getReflection() > 0.0f) {
            glm::vec3 reflectDir = glm::reflect(-viewDir, normal);
            reflectDir = glm::normalize(reflectDir);
            
            const float REFLECT_EPSILON = 0.001f;
            glm::vec3 reflectRayStart = viewPos + REFLECT_EPSILON * normal;
            Ray reflectRay(glm::vec4(reflectRayStart, 1.0f), glm::vec4(reflectDir, 0.0f));
            
            HitRecord reflectHit = raycast(reflectRay, scenegraph);
            glm::vec3 reflectColor(0.0f);
            
            if (reflectHit.hasHit()) {
                reflectColor = shadeRecursive(reflectHit, scenegraph, bounce + 1);
            }
            
            color = material.getAbsorption() * color + material.getReflection() * reflectColor;
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
