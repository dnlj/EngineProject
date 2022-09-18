#pragma once

// GLM
#include <glm/gtx/compatibility.hpp>

namespace Engine::Gfx {
	class AnimSeq {
		public:
			struct InterpValues {
				glm::vec3 pos = {};
				glm::vec3 scale = {};
				glm::quat rot = {};
			};

		private:
			using TimePoint = float32; // Might be worth using double? Assimp imports as doubles.

			struct VecKey {
				glm::vec3 value;
				TimePoint time;
			};

			struct QuatKey {
				glm::quat value;
				TimePoint time;
			};

			template<class T>
			static auto interpImpl(TimePoint t, const std::vector<T>& cont) noexcept {
				ENGINE_DEBUG_ASSERT(cont.size());

				const auto last = std::lower_bound(cont.begin(), cont.end(), t, [](const T& b, const TimePoint& t) noexcept {
					return b.time < t;
				});

				if (last == cont.end()) { return (last - 1)->value; }
				if (last == cont.begin()) { return last->value; }

				const auto first = last - 1;
				ENGINE_DEBUG_ASSERT(first->time != last->time, "Invalid animation timestamp. No frames should have the same time.");

				const auto p = (t - first->time) / (last->time - first->time);

				if constexpr (std::same_as<T, QuatKey>) {
					return glm::slerp(first->value, last->value, p);
				} else {
					return glm::lerp(first->value, last->value, p);
				}
			}

		public:
			int nodeId = -1;
			std::vector<VecKey> pos;
			std::vector<VecKey> scale;
			std::vector<QuatKey> rot;

			// TODO: should this take a [0,1] instead of tick?
			InterpValues sample(TimePoint t) const {
				ENGINE_DEBUG_ASSERT(pos.size(), "Cannot interp animation: invalid positions.");
				ENGINE_DEBUG_ASSERT(scale.size(), "Cannot interp animation: invalid scales.");
				ENGINE_DEBUG_ASSERT(rot.size(), "Cannot interp animation: invalid rotations.");

				InterpValues res;
				res.pos = interpImpl(t, pos);
				res.scale = interpImpl(t, scale);
				res.rot = interpImpl(t, rot);
				return res;
			}
	};
}
