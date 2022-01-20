#ifndef game_object_h
#define game_object_h

#include <stdint.h>
#include <stddef.h>
#include "../raylib/raylib.h"

namespace game {
	class GameObject {
		private:
			/* data */
		public:
			Vector3 position;
			Quaternion rotation;
			Vector3 scale;
			Model model;
			void loadModel(const char *path);
			void loadModelFromMesh(Mesh mesh);
			GameObject();
			~GameObject();
	};
}

#endif