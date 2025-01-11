#pragma once

#pragma warning(push, 0)
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include "Hazel/Core/UUID.h"
#pragma warning(pop)

enum class API { OPENGL = 0, VULKAN = 1, UNDEFINED = 2, };

enum class ShaderDataType : uint8_t {
	NONE = 0,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	MAT3,
	MAT4,
	INT,
	INT2,
	INT3,
	INT4,
	BOOL
};

#define PI 3.141592653589793f

#define SMALL_NUMBER		(float)9.99999993922529e-9

//struct Transform {
//	glm::vec3 position = glm::vec3(0);
//	glm::vec3 rotation = glm::vec3(0);
//	glm::vec3 scale = glm::vec3(1);
//	glm::mat4 to_mat4() {
//		glm::mat4 m = glm::translate(glm::mat4(1), position);
//		m *= glm::mat4_cast(glm::quat(rotation));
//		m = glm::scale(m, scale);
//		return m;
//	};
//};


namespace Hazel {
	class Asset {
	public:
		UUID id;

		bool isEmbedded = false;
		std::string path;
	};
}

namespace Hazel {
	using MouseCode = uint16_t;
	namespace Mouse {
		enum : MouseCode {
			// From glfw3.h
			Button0    = 0,
			Button1    = 1,
			Button2    = 2,
			Button3    = 3,
			Button4    = 4,
			Button5    = 5,
			Button6    = 6,
			Button7    = 7,

			ButtonLast = Button7,
			ButtonLeft = Button0,
			ButtonRight = Button1,
			ButtonMiddle = Button2
		};
	}
}