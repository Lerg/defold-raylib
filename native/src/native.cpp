#include <stdint.h>
#include <stddef.h>

#include <dmsdk/extension/extension.h>
#include <dmsdk/graphics/graphics.h>
#include <dmsdk/script/script.h>
#include <dmsdk/vectormath/cpp/vectormath_aos.h>
#include <dmsdk/dlib/log.h>
#include <dmsdk/lua/luaconf.h>
#include <dmsdk/lua/lauxlib.h>
#include <dmsdk/lua/lua.h>
#include "extension.h"

#ifndef DM_PLATFORM_HTML5
	//#define DEBUG_OPENGL_CALLS
#endif

#define RLIGHTS_IMPLEMENTATION
#include "game/game.h"

#include "gl_loader.h"

static lua_State *cached_L;
static int screen_width = 2048;
static int screen_height = 2048;
static game::Game *game_instance;
static RenderTexture2D render_target;
static GLint defold_draw_fbo = 0, defold_read_fbo = 0, defold_vao = 0;
static bool is_pause = false;

static char *copy_data(const char *source, size_t length) {
	if (source != NULL) {
		char *destination = (char *)RL_MALLOC(length * sizeof(unsigned char));
		memcpy(destination, source, length);
		return destination;
	}
	return NULL;
}

static char *copy_string(const char *source) {
	if (source != NULL) {
		size_t length = strlen(source) + 1;
		char *destination = (char *)RL_MALLOC(length * sizeof(unsigned char));
		strncpy(destination, source, length);
		destination[length - 1] = 0;
		return destination;
	}
	return NULL;
}

static const char *sys_load_resource(lua_State *L, const char *filename, size_t *out_length) {
	lua_getglobal(L, "sys");
	lua_getfield(L, -1, "load_resource");
	lua_remove(L, -2); //sys
	lua_pushstring(L, filename);
	lua_call(L, 1, 1);
	return lua_tolstring(L, -1, out_length);
}

/*static bool get_render_target_texture_id(void *render_target, int *color_texture_id) {
	dmGraphics::HTexture color_texture = dmGraphics::GetRenderTargetAttachment((dmGraphics::HRenderTarget)render_target, dmGraphics::ATTACHMENT_COLOR);
	if (color_texture == NULL) {
		dmLogError("Could not get the color attachment from render target.");
		return false;
	}

	uint32_t *color_gl_handle = NULL;
	dmGraphics::HandleResult result = dmGraphics::GetTextureHandle(color_texture, (void **)&color_gl_handle);
	if (result != dmGraphics::HANDLE_RESULT_OK) {
		dmLogError("Could not get color texture handle from render target.");
		return false;
	}
	*color_texture_id = *color_gl_handle;
	return true;
}*/

unsigned char *defold_loadFileData(const char *fileName, unsigned int *bytesRead) {
	size_t length = 0;
	const char *content = sys_load_resource(cached_L, fileName, &length);
	*bytesRead = length;
	return (unsigned char *)copy_data(content, length);
}

char *defold_loadFileText(const char *fileName) {
	size_t length = 0;
	const char *content = sys_load_resource(cached_L, fileName, &length);
	return copy_string(content);
}

static void save_defold_state() {
	//glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defold_draw_fbo);
	//glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &defold_read_fbo);
	#if defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING_OES, &defold_vao);
	#else
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &defold_vao);
	#endif
	GLenum error = glGetError(); if (error) {dmLogError("render save state error: %#04X\n", error);}
}

static void restore_defold_state() {
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defold_draw_fbo);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, defold_read_fbo);
	#if defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
		glBindVertexArrayOES(defold_vao);
	#else
		glBindVertexArray(defold_vao);
	#endif
	GLenum error = glGetError(); if (error) {dmLogError("render restore state error: %#04X\n", error);}
}

static int extension_init(lua_State *L) {
	//void *defold_render_target = lua_touserdata(L, 1);
	screen_width = (int)lua_tonumber(L, 1);
	screen_height = (int)lua_tonumber(L, 2);

	SetLoadFileDataCallback(&defold_loadFileData);
	SetLoadFileTextCallback(&defold_loadFileText);
	cached_L = L;

	#if !defined(DM_PLATFORM_HTML5) && !defined(DM_PLATFORM_ANDROID) && !defined(DM_PLATFORM_IOS)
		gladLoadGL((GLADloadfunc)glfwGetProcAddress);
		LoadGLExtensions((void *)&glfwGetProcAddress);
	#endif

	save_defold_state();

	InitWindow(screen_width, screen_height, "title");

	game_instance = new game::Game(SHADERS_PATH);
	game_instance->camera.aspect = (float)screen_width / (float)screen_height;

	/*int color_texture_id = 0;
	bool success = get_render_target_texture_id(defold_render_target, &color_texture_id);
	if (!success) {
		dmLogError("Failed to get render target texture id.");
	}

	render_target = LoadRenderTexture(screen_width, screen_height);
	#if defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
		render_target.id = 16;
	#else
		render_target.id = 1;
	#endif
	render_target.texture.id = color_texture_id;*/

	GLenum error = glGetError(); if (error) {dmLogError("init error: %#04X\n", error);}

	restore_defold_state();
	return 0;
}

//static int fbo_id = 0;
//static int fbo_index = 0;
static int extension_render(lua_State *L) {
	save_defold_state();

	float dt = lua_tonumber(L, 1);
	if (!is_pause) {
		game_instance->update(dt);
	}

	/*if (fbo_index == 45) {
		if (render_target.id > 1) {
			render_target.id -= 1;
			dmLogInfo("Trying target id = %d", render_target.id);
		}
		fbo_index = 0;
	} else {
		++fbo_index;
	}*/

	for (int i = 0; i < game_instance->light_count; ++i) {
		//printf("light %d\n", i);
		GL_CHECK(UpdateLightValues(game_instance->shader, game_instance->lights[i]));
	}

	// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
	float cameraPos[3] = {game_instance->camera.position.x, game_instance->camera.position.y, game_instance->camera.position.z};
	GL_CHECK(SetShaderValue(game_instance->shader, game_instance->shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3));

	//GL_CHECK(BeginTextureMode(render_target));
	GL_CHECK(ClearBackground((Color){20, 40, 100}));
	GL_CHECK(BeginMode3D(game_instance->camera));
	//BeginBlendMode(BLEND_ALPHA);

	for (int i = 0; i < game_instance->game_object_count; ++i) {
		game::GameObject *game_object = game_instance->game_objects[i];
		if (game_object != nullptr) {
			GL_CHECK(DrawModelEx(game_object->model, game_object->position, game_object->rotation, game_object->scale, WHITE));
		}
	}

	//EndBlendMode();
	GL_CHECK(EndMode3D());
	//GL_CHECK(EndTextureMode());

	GLenum error = glGetError(); if (error) {dmLogError("render error: %#04X\n", error);}

	restore_defold_state();
	return 0;
}

static int set_screen_size(lua_State *L) {
	screen_width = (int)lua_tonumber(L, 1);
	screen_height = (int)lua_tonumber(L, 2);
	game_instance->camera.aspect = (float)screen_width / (float)screen_height;
	//render_target.texture.width = screen_width;
	//render_target.texture.height = screen_height;
	return 0;
}

static int set_pause(lua_State *L) {
	is_pause = lua_toboolean(L, 1);
	return 0;
}

static int set_game_object_position(lua_State *L) {
	int game_object_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Vector3 *position = dmScript::CheckVector3(L, 2);
	game_instance->setGameObjectPosition(game_object_id, (Vector3){position->getX(), position->getY(), position->getZ()});
	return 0;
}

static int set_game_object_scale(lua_State *L) {
	int game_object_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Vector3 *scale = dmScript::CheckVector3(L, 2);
	game_instance->setGameObjectScale(game_object_id, (Vector3){scale->getX(), scale->getY(), scale->getZ()});
	return 0;
}

static int set_game_object_rotation(lua_State *L) {
	int game_object_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Quat *rotation = dmScript::CheckQuat(L, 2);
	game_instance->setGameObjectRotation(game_object_id, (Quaternion){rotation->getX(), rotation->getY(), rotation->getZ(), rotation->getW()});
	return 0;
}

static int set_game_object_model(lua_State *L) {
	int game_object_id = (int)lua_tonumber(L, 1);
	int model_id = (int)lua_tonumber(L, 2);
	game_instance->setGameObjectModel(game_object_id, model_id);
	return 0;
}

static int set_camera_position(lua_State *L) {
	int game_object_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Vector3 *position = dmScript::CheckVector3(L, 1);
	game_instance->setCameraPosition((Vector3){position->getX(), position->getY(), position->getZ()});
	return 0;
}

static int set_camera_target(lua_State *L) {
	Vectormath::Aos::Vector3 *target = dmScript::CheckVector3(L, 1);
	game_instance->setCameraTarget((Vector3){target->getX(), target->getY(), target->getZ()});
	return 0;
}

static int set_camera_up(lua_State *L) {
	Vectormath::Aos::Vector3 *up = dmScript::CheckVector3(L, 1);
	game_instance->setCameraUp((Vector3){up->getX(), up->getY(), up->getZ()});
	return 0;
}

static int create_road(lua_State *L) {
	int id = game_instance->createRoad();
	lua_pushinteger(L, id);
	return 1;
}

static int create_target(lua_State *L) {
	int id = game_instance->createTarget();
	lua_pushinteger(L, id);
	return 1;
}

static int create_light(lua_State *L) {
	LightType light_type = (LightType)lua_tonumber(L, 1);
	int id = game_instance->createLight(light_type);
	lua_pushinteger(L, id);
	return 1;
}

static int set_light_position(lua_State *L) {
	int light_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Vector3 *position = dmScript::CheckVector3(L, 2);
	game_instance->setLightPosition(light_id, (Vector3){position->getX(), position->getY(), position->getZ()});
	return 0;
}

static int set_light_target(lua_State *L) {
	int light_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Vector3 *target = dmScript::CheckVector3(L, 2);
	game_instance->setLightTarget(light_id, (Vector3){target->getX(), target->getY(), target->getZ()});
	return 0;
}

static int set_light_color(lua_State *L) {
	int light_id = (int)lua_tonumber(L, 1);
	Vectormath::Aos::Vector3 *color = dmScript::CheckVector3(L, 2);
	game_instance->setLightColor(light_id, (Vector3){color->getX(), color->getY(), color->getZ()});
	return 0;
}

static int set_light_enabled(lua_State *L) {
	int light_id = (int)lua_tonumber(L, 1);
	bool enabled = lua_toboolean(L, 2);
	game_instance->setLightEnabled(light_id, enabled);
	return 0;
}

static int clear_objects(lua_State *L) {
	game_instance->clearObjects();
	return 0;
}

static const luaL_reg extension_functions[] = {
	{"init", extension_init},
	{"render", extension_render},
	{"set_screen_size", set_screen_size},
	{"set_pause", set_pause},
	{"set_game_object_position", set_game_object_position},
	{"set_game_object_scale", set_game_object_scale},
	{"set_game_object_rotation", set_game_object_rotation},
	{"set_game_object_model", set_game_object_model},
	{"set_camera_position", set_camera_position},
	{"set_camera_target", set_camera_target},
	{"set_camera_up", set_camera_up},
	{"create_road", create_road},
	{"create_target", create_target},
	{"create_light", create_light},
	{"set_light_position", set_light_position},
	{"set_light_target", set_light_target},
	{"set_light_color", set_light_color},
	{"set_light_enabled", set_light_enabled},
	{"clear_objects", clear_objects},
	{0, 0}
};

dmExtension::Result APP_INITIALIZE(dmExtension::AppParams *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result APP_FINALIZE(dmExtension::AppParams *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result INITIALIZE(dmExtension::Params *params) {
	luaL_register(params->m_L, EXTENSION_NAME_STRING, extension_functions);
	lua_pop(params->m_L, 1);
	return dmExtension::RESULT_OK;
}

dmExtension::Result UPDATE(dmExtension::Params *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FINALIZE(dmExtension::Params *params) {
	delete game_instance;
	return dmExtension::RESULT_OK;
}

DECLARE_DEFOLD_EXTENSION