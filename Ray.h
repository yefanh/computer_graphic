#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

/**
 * @brief Represents a 3D ray
 * 
 * A ray is defined by a starting point (start) and a direction (direction).
 * Any point on the ray can be represented by the parametric equation: P(t) = start + t * direction
 * where t >= 0 represents points in the forward direction of the ray.
 * 
 * In ray tracing, rays are used for:
 * 1. Primary rays: from camera through each pixel
 * 2. Shadow rays: from intersection point to light sources
 * 3. Reflection/refraction rays: for advanced effects
 */
class Ray {
public:
    /**
     * @brief Default constructor
     * Creates a ray starting from origin, pointing in -Z direction
     */
    Ray() {
        start = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // point, w=1
        direction = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);  // vector, w=0
    }

    /**
     * @brief Parameterized constructor
     * @param start Ray origin (typically camera position or intersection point)
     * @param direction Ray direction (should be normalized)
     */
    Ray(const glm::vec4& start, const glm::vec4& direction) {
        this->start = start;
        this->direction = direction;
    }

    /**
     * @brief Constructor using 3D vectors
     * @param start Ray origin
     * @param direction Ray direction
     */
    Ray(const glm::vec3& start, const glm::vec3& direction) {
        this->start = glm::vec4(start, 1.0f);      // point: w = 1
        this->direction = glm::vec4(direction, 0.0f);  // vector: w = 0
    }

    /**
     * @brief Get ray origin
     * @return The starting position of the ray
     */
    glm::vec4 getStart() const {
        return start;
    }

    /**
     * @brief Get ray direction
     * @return The direction vector of the ray
     */
    glm::vec4 getDirection() const {
        return direction;
    }

    /**
     * @brief Set ray origin
     * @param start New starting position
     */
    void setStart(const glm::vec4& start) {
        this->start = start;
    }

    /**
     * @brief Set ray direction
     * @param direction New direction vector
     */
    void setDirection(const glm::vec4& direction) {
        this->direction = direction;
    }

    /**
     * @brief Calculate a point on the ray at parameter t
     * 
     * Ray parametric equation: P(t) = start + t * direction
     * 
     * @param t Parameter value
     * @return The 3D point on the ray corresponding to t
     * 
     * Examples:
     *   - t = 0: returns ray origin
     *   - t = 1: returns start + direction
     *   - t > 0: points in forward direction of the ray
     */
    glm::vec4 getPointAt(float t) const {
        return start + t * direction;
    }

private:
    glm::vec4 start;      // Ray origin (homogeneous coordinates, w=1 for point)
    glm::vec4 direction;  // Ray direction (homogeneous coordinates, w=0 for vector)
};

#endif
