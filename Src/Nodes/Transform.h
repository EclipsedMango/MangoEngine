
#ifndef MANGORENDERING_TRANSFORM_H
#define MANGORENDERING_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
    glm::vec3 Position = {0, 0, 0};
    glm::quat Rotation = glm::quat(1, 0, 0, 0); // identity
    glm::vec3 Scale    = {1, 1, 1};

    [[nodiscard]] glm::mat4 GetModelMatrix() const {
        glm::mat4 mat = glm::translate(glm::mat4(1.0f), Position);
        mat *= glm::mat4_cast(Rotation);
        mat = glm::scale(mat, Scale);
        return mat;
    }

    // set rotation from Euler angles in degrees (XYZ order)
    void SetEuler(const glm::vec3 degrees) {
        Rotation = glm::quat(glm::radians(degrees));
    }

    // get rotation as Euler angles in degrees (XYZ order)
    [[nodiscard]] glm::vec3 GetEuler() const {
        return glm::degrees(glm::eulerAngles(Rotation));
    }
};


#endif //MANGORENDERING_TRANSFORM_H