#pragma once

#include "gala_model.h"

// libs

#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace gala {
	struct TransformComponent {
		glm::vec3 translation{};  // (position offset)
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		 // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		 // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	class GalaGameObject {
	public:
		using id_t = unsigned int;

		static GalaGameObject createGameObject() {
			static id_t currentId = 0;
			return GalaGameObject{ currentId++ };
		}

		GalaGameObject(const GalaGameObject&) = delete;
		GalaGameObject& operator=(const GalaGameObject&) = delete;
		GalaGameObject(GalaGameObject&&) = default;
		GalaGameObject& operator=(GalaGameObject&&) = default;

		id_t getId() { return id; }

		std::shared_ptr<GalaModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		GalaGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}
