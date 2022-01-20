#include "game.h"

namespace game {
	Game::Game(const char *shaders_path) {
		game_object_count = 0;
		light_count = 0;
		shader = LoadShader(TextFormat("%s%s", shaders_path, "base_lighting.vs"), TextFormat("%s%s", shaders_path, "lighting.fs"));

		// Get some required shader loactions
		shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
		// NOTE: "matModel" location name is automatically assigned on shader loading,
		// no need to get the location again if using that uniform name
		//shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

		// Ambient light level (some basic lighting)
		int ambientLoc = GetShaderLocation(shader, "ambient");
		SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

		// Using 4 point lights: gold, red, green and blue
		/*lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, -2 }, Vector3Zero(), ORANGE, shader);
		lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, 2 }, Vector3Zero(), PURPLE, shader);
		lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, 2 }, Vector3Zero(), GREEN, shader);
		lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, -2 }, Vector3Zero(), RED, shader);
		light_count = 4;*/

		camera.position = (Vector3){0.0f, 2.0f, -5.0f};    // Camera position
		camera.target = (Vector3){0.0f, 0.5f, 0.0f};      // Camera looking at point
		camera.up = (Vector3){0.0f, 1.0f, 0.0f};          // Camera up vector (rotation towards target)
		camera.fovy = 60.0f;                              // Camera field-of-view Y
		camera.projection = CAMERA_PERSPECTIVE;

		road_model = LoadModel("/resources/models/road_straight.obj");
		road_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("/resources/textures/road/asphalt_albedo.jpg");
		road_model.materials[0].maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("/resources/textures/road/asphalt_metalness.jpg");
		road_model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("/resources/textures/road/asphalt_normals.jpg");
		assignShaderToModel(road_model);

		target_model = LoadModel("/resources/models/target.obj");
		assignShaderToModel(target_model);
	}

	void Game::assignShaderToModel(Model model) {
		for (int i = 0; i < model.materialCount; ++i) {
			model.materials[i].shader = shader;
		}
	}

	int Game::createRoad() {
		GameObject *road = new GameObject();
		road->model = road_model;
		return addGameObject(road);
	}

	int Game::createTarget() {
		GameObject *target = new GameObject();
		target->model = target_model;
		return addGameObject(target);
	}

	int Game::addGameObject(GameObject *game_object) {
		if (game_object_count == MAX_GAME_OBJECT_COUNT) {
			dmLogError("Max GameObject count reached!!!");
		}
		game_objects[game_object_count] = game_object;
		return game_object_count++;
	}

	int Game::addLight(Light light) {
		lights[light_count] = light;
		return light_count++;
	}

	void Game::update(float dt) {
	}

	void Game::setGameObjectPosition(int game_object_id, Vector3 position) {
		GameObject *game_object = game_objects[game_object_id];
		game_object->position = position;
	}

	void Game::setGameObjectScale(int game_object_id, Vector3 scale) {
		GameObject *game_object = game_objects[game_object_id];
		game_object->scale = scale;
	}

	void Game::setGameObjectRotation(int game_object_id, Quaternion rotation) {
		GameObject *game_object = game_objects[game_object_id];
		game_object->rotation = rotation;
	}

	void Game::setGameObjectModel(int game_object_id, int model_id) {
		GameObject *game_object = game_objects[game_object_id];
		switch (model_id) {
			case 0:
				game_object->model = road_model;
				break;
			case 1:
				game_object->model = target_model;
				break;
		}
	}

	void Game::setCameraPosition(Vector3 position) {
		camera.position = position;
	}

	void Game::setCameraTarget(Vector3 target) {
		camera.target = target;
	}

	void Game::setCameraUp(Vector3 up) {
		camera.up = up;
	}

	int Game::createLight(LightType light_type) {
		Light light = CreateLight(light_type, (Vector3){1, 1, 1}, Vector3Zero(), (Vector3){1, 1, 1}, shader);
		return addLight(light);
	}

	void Game::setLightPosition(int light_id, Vector3 position) {
		lights[light_id].position = position;
	}

	void Game::setLightTarget(int light_id, Vector3 target) {
		lights[light_id].target = target;
	}

	void Game::setLightColor(int light_id, Vector3 color) {
		lights[light_id].color = color;
	}

	void Game::setLightEnabled(int light_id, bool enabled) {
		lights[light_id].enabled = enabled;
	}

	void Game::clearObjects() {
		for (int i = 0; i < game_object_count; ++i) {
			GameObject *game_object = game_objects[i];
			if (game_object != nullptr) {
				delete game_object;
				game_objects[i] = nullptr;
			}
		}
		game_object_count = 0;
		ClearLights();
		light_count = 0;
	}

	Game::~Game() {
		clearObjects();

		UnloadModel(target_model);
		UnloadModel(road_model);
		UnloadShader(shader);
	}
}