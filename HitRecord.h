#ifndef __HITRECORD_H__
#define __HITRECORD_H__

#include <glm/glm.hpp>
#include "Material.h"
#include <string>
#include <limits>

/**
 * @brief Stores all information about ray-object intersection
 * 
 * When a ray intersects with objects in the scene, we need to record information to:
 * 1. Determine which intersection is closest (by comparing t values)
 * 2. Calculate lighting/shading at that point (requires intersection point, normal, material, etc.)
 * 
 * HitRecord is the data structure used to store this information.
 */
class HitRecord {
public:
    /**
     * @brief Default constructor
     * 
     * Initializes to "no hit" state:
     * - t = infinity (indicates no intersection)
     * - other values set to defaults
     */
    HitRecord() {
        t = std::numeric_limits<float>::infinity();  // infinity means no hit
        intersectionPoint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        normal = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        textureCoordinates = glm::vec2(0.0f, 0.0f);
        textureName = "";
    }

    /**
     * @brief Check if there is a valid intersection
     * @return true if there is an intersection (t is finite and positive)
     * 
     * t > 0 is important:
     * - t = 0 means intersection at ray origin (usually camera, we don't want to "see" the camera)
     * - t < 0 means intersection behind the ray origin (behind camera)
     * - t > 0 means intersection in forward direction (in front of camera, what we want to see)
     */
    bool hasHit() const {
        return t > 0.0f && t < std::numeric_limits<float>::infinity();
    }

    // ==================== Getters and Setters ====================

    /**
     * @brief Get the t value of intersection
     * 
     * t is the parameter in ray equation P(t) = start + t * direction.
     * Smaller t means closer intersection; we always choose the closest intersection to render.
     */
    float getT() const {
        return t;
    }

    void setT(float t) {
        this->t = t;
    }

    /**
     * @brief Get intersection point in view coordinates
     * 
     * Why view coordinates?
     * - Lighting calculations are typically done in view coordinates
     * - Light positions are also in view coordinates
     * - Easy to compute view direction (from origin to intersection point)
     */
    glm::vec4 getIntersectionPoint() const {
        return intersectionPoint;
    }

    void setIntersectionPoint(const glm::vec4& point) {
        this->intersectionPoint = point;
    }

    /**
     * @brief Get normal vector at intersection point (in view coordinates)
     * 
     * Normal is used for lighting calculation:
     * - Diffuse: N dot L (dot product of normal and light direction)
     * - Specular: need normal to compute reflection direction
     * 
     * Note: normal should be normalized (length = 1)
     */
    glm::vec4 getNormal() const {
        return normal;
    }

    void setNormal(const glm::vec4& normal) {
        this->normal = normal;
    }

    /**
     * @brief Get material properties at intersection point
     * 
     * Material determines object appearance:
     * - ambient: ambient light reflection coefficient
     * - diffuse: diffuse reflection coefficient (determines base color)
     * - specular: specular reflection coefficient (highlights)
     * - shininess: glossiness (sharpness of highlights)
     */
    util::Material getMaterial() const {
        return material;
    }

    void setMaterial(const util::Material& material) {
        this->material = material;
    }

    /**
     * @brief Get texture coordinates
     * 
     * Used for texture mapping (not required for this assignment, but interface reserved)
     * (u, v) coordinates, typically in [0, 1] range
     */
    glm::vec2 getTextureCoordinates() const {
        return textureCoordinates;
    }

    void setTextureCoordinates(const glm::vec2& texCoords) {
        this->textureCoordinates = texCoords;
    }

    /**
     * @brief Get texture name
     * 
     * Used to look up the corresponding texture object
     */
    std::string getTextureName() const {
        return textureName;
    }

    void setTextureName(const std::string& name) {
        this->textureName = name;
    }

private:
    /**
     * t value: parameter in ray equation
     * P(t) = start + t * direction
     * 
     * Initialized to infinity to indicate "no hit"
     */
    float t;

    /**
     * Intersection point (in view coordinates)
     * The 3D point where ray intersects with object surface
     */
    glm::vec4 intersectionPoint;

    /**
     * Normal vector (in view coordinates)
     * Surface normal at the intersection point
     * w = 0 indicates this is a vector, not a point
     */
    glm::vec4 normal;

    /**
     * Material properties
     * Stores ambient, diffuse, specular, shininess, etc.
     */
    util::Material material;

    /**
     * Texture coordinates (u, v)
     * Used for texture mapping
     */
    glm::vec2 textureCoordinates;

    /**
     * Texture name
     * Used to look up actual texture object from texture map
     */
    std::string textureName;
};

#endif
