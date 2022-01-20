#include "game_object.h"
#include "../raylib/raymath.h"

namespace game {
	GameObject::GameObject() {
		position.x = 0;
		position.y = 0;
		position.z = 0;
		rotation = QuaternionIdentity();
		scale.x = 1;
		scale.y = 1;
		scale.z = 1;
	}

	void GameObject::loadModel(const char *path) {
		model = LoadModel(path);
	}

	void GameObject::loadModelFromMesh(Mesh mesh) {
		model = LoadModelFromMesh(mesh);
	}

	GameObject::~GameObject() {
	}
}