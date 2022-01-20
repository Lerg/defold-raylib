#ifndef game_h
#define game_h

#include "../raylib/raylib.h"
#include "../raylib/raymath.h"

#include "../raylib/rlights.h"
#include <dmsdk/dlib/log.h>

#include "game_object.h"

namespace game {
	enum Input {
		INPUT_LEFT,
		INPUT_RIGHT
	};
	static const int MAX_GAME_OBJECT_COUNT = 150;
	static const int MAX_LIGHT_COUNT = 6;
	class Game {
		private:
			Model road_model, target_model;
			void assignShaderToModel(Model model);
		public:
			int game_object_count = 0;
			int light_count = 0;
			Shader shader;
			Camera camera = {};
			Light lights[MAX_LIGHT_COUNT] = {0};
			GameObject *game_objects[MAX_GAME_OBJECT_COUNT] = {nullptr};
			Game(const char *shaders_path);
			int createRoad();
			int createTarget();
			int addGameObject(GameObject *game_object);
			int addLight(Light light);
			void update(float dt);
			void setGameObjectPosition(int game_object_id, Vector3 position);
			void setGameObjectScale(int game_object_id, Vector3 scale);
			void setGameObjectRotation(int game_object_id, Quaternion rotation);
			void setGameObjectModel(int game_object_id, int model_id);
			void setCameraPosition(Vector3 position);
			void setCameraTarget(Vector3 target);
			void setCameraUp(Vector3 up);
			int createLight(LightType light_type);
			void setLightPosition(int light_id, Vector3 position);
			void setLightTarget(int light_id, Vector3 target);
			void setLightColor(int light_id, Vector3 color);
			void setLightEnabled(int light_id, bool enabled);
			void clearObjects();
			~Game();
	};
}

#endif
