#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void rect_render(void *, size_t);

#define ISQ_UI_RECT_RENDER(buffer, count) rect_render(buffer, count)
#define ISQ_UI_IMPLEMENTATION
#include "isq_ui.h"

#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;
typedef float f32;
typedef double f64;

typedef union {
	struct { f32 x, y; };
	struct { f32 u, v; };
	f32 data[2];
} vec2;

vec2 vec2_zero = { 0, 0 };

typedef union {
	struct { f32 x, y, z; };
	struct { f32 r, g, b; };
	f32 data[3];
} vec3;

typedef union {
	struct { f32 x, y, z, w; };
	struct { f32 r, g, b, a; };
	f32 data[4];
} vec4;

vec4 vec4_zero = { 0, 0, 0, 0 };

vec4 vec4_scale(vec4 v, f32 scale)
{
	return (vec4){ .x = v.x * scale, .y = v.y * scale, .z = v.z * scale, .w = v.w * scale };
}

typedef union {
	struct { vec4 x, y, z, w; };
	f32 data[4][4];
} mat4;

mat4 mat4_identity(void)
{
	return (mat4){
		.x = (vec4){ .x = 1, .y = 0, .z = 0, .w = 0 },
		.y = (vec4){ .x = 0, .y = 1, .z = 0, .w = 0 },
		.z = (vec4){ .x = 0, .y = 0, .z = 1, .w = 0 },
		.w = (vec4){ .x = 0, .y = 0, .z = 0, .w = 1 },
	};
}

mat4 mat4_translate(f32 x, f32 y, f32 z)
{
	mat4 m = mat4_identity();
	m.w.x = x;
	m.w.y = y;
	m.w.z = z;
	return m;
}

mat4 mat4_scale_aniso(mat4 m, f32 x, f32 y, f32 z)
{
	return (mat4){
		vec4_scale(m.x, x),
		vec4_scale(m.y, y),
		vec4_scale(m.z, z),
		m.w
	};
}

mat4 mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    mat4 m;

    m.x.x = 2.f/(r-l);
    m.x.y = m.x.z = m.x.w = 0;

    m.y.y = 2.f/(t-b);
    m.y.x = m.y.z = m.y.w = 0;

    m.z.z = -2.f/(f-n);
    m.z.x = m.z.y = m.z.w = 0;

    m.w.x = -(r+l)/(r-l);
    m.w.y = -(t+b)/(t-b);
    m.w.z = -(f+n)/(f-n);
    m.w.w = 1;

    return m;
}

#define WIDTH 1920
#define HEIGHT 1080

u32 rect_shader;
u32 rect_vao;
u32 rect_vbo;
u32 rect_ebo;

vec2 mouse_position;

enum {
	MAX_RECT_COUNT = 1000,
	MAX_VERTEX_COUNT = MAX_RECT_COUNT * 4,
	INDEX_COUNT = MAX_RECT_COUNT * 6,
};

static char *buffer_from_file(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	usize size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = ISQ_MALLOC(size + 1);
	if (!buffer) {
		fclose(file);
		return NULL;
	}

	fread(buffer, size, 1, file);
	fclose(file);
	buffer[size] = 0;

	return buffer;
}

static u32 shader_create(const char *vert_path, const char *frag_path)
{
	char *vert_source = buffer_from_file(vert_path);
	char *frag_source = buffer_from_file(frag_path);

	if (!vert_source || !frag_source)
		return 0;

	char log[512] = {0};
	int success = 0;

	u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, (const char * const *)&vert_source, NULL);
	glCompileShader(vert_shader);
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vert_shader, 512, NULL, log);
		printf("ERROR COMPILING SHADER: %s\n", log);
		return 0;
	}

	u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, (const char * const *)&frag_source, NULL);
	glCompileShader(frag_shader);
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(frag_shader, 512, NULL, log);
		printf("ERROR COMPILING SHADER: %s\n", log);
		return 0;
	}

	u32 shader = glCreateProgram();
	glAttachShader(shader, vert_shader);
	glAttachShader(shader, frag_shader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, log);
		printf("ERROR LINKING SHADER: %s\n", log);
		return 0;
	}

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return shader;
}

void rect_render(void *buffer, usize count)
{
	// TODO: DELETE THIS
	struct isq_ui_vertex *v = (struct isq_ui_vertex*)buffer;

	glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(struct isq_ui_vertex), buffer);

	glUseProgram(rect_shader);
	glBindVertexArray(rect_vao);
	glDrawElements(GL_TRIANGLES, (count / 4) * 6, GL_UNSIGNED_INT, NULL);
}

void cursor_callback(GLFWwindow *window, f64 x, f64 y)
{
	mouse_position.x = (f32)x;
	mouse_position.y = (f32)y;
}

int main(void)
{
	const char *err_log = calloc(1, 512);

	if (!glfwInit()) {
		int err_no = glfwGetError(&err_log);
		printf("ERROR [%d]: %s.\n", err_no, err_log);
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create window
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "ISQ UI Test", NULL, NULL);
	if (!window) {
		int err_no = glfwGetError(&err_log);
		printf("ERROR [%d]: %s.\n", err_no, err_log);
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwSetCursorPosCallback(window, cursor_callback);

	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		int err_no = glfwGetError(&err_log);
		printf("ERROR [%d]: %s.\n", err_no, err_log);
		glfwTerminate();
		return EXIT_FAILURE;
	}

	rect_shader = shader_create("rect.vert", "rect.frag");

	glGenVertexArrays(1, &rect_vao);
	glBindVertexArray(rect_vao);

	u32 indices[INDEX_COUNT];
	for (usize i = 0, offset = 0; i < INDEX_COUNT; i += 6, offset += 4) {
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;
		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;
	}

	glGenBuffers(1, &rect_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct isq_ui_vertex) * MAX_VERTEX_COUNT, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct isq_ui_vertex), (void*)offsetof(struct isq_ui_vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct isq_ui_vertex), (void*)offsetof(struct isq_ui_vertex, color));

	glGenBuffers(1, &rect_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glUseProgram(rect_shader);

	mat4 projection = mat4_ortho(0, WIDTH, HEIGHT, 0, -1, 1);
	glUniformMatrix4fv(glGetUniformLocation(rect_shader, "u_projection"), 1, GL_FALSE, &projection.data[0][0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vec2 root_pos = { 100, 100 };

	isq_ui_init(WIDTH, HEIGHT);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2, 0.2, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			root_pos.x += 0.5;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			root_pos.x -= 0.5;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			root_pos.y -= 0.5;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			root_pos.y += 0.5;
		}

		isq_ui_begin(mouse_position.x, mouse_position.y);

		u32 id = isq_ui_flexbox(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND).id;
		isq_ui_position(id, root_pos.x, root_pos.y);
		isq_ui_semantic_size(id, (union isq_ui_sizes){
			.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 500 },
			.y = { .type = ISQ_UI_SIZE_TYPE_PERCENT, .value = 0.9 },
		});
		isq_ui_background_color(id, 1, 1, 1, 1);

		for (int i = 0; i < 60; ++i) {
			struct isq_ui_state state = isq_ui_box(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND | ISQ_UI_BOX_FLAG_HOVERABLE | ISQ_UI_BOX_FLAG_DRAW_BORDER);
			id = state.id;
			isq_ui_semantic_size(id, (union isq_ui_sizes){
				.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 50 },
				.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 50 },
			});

			isq_ui_border(id, 1, 1, 1, 0.2, 3);

			f32 r = (f32)(i % 75) / 100.f;
			f32 g = (f32)(i % 50) / 100.f;
			f32 b = (f32)(i % 25) / 100.f;

			if (state.hovered) {
				r = 1;
				g = 0;
				b = 0;
			}

			isq_ui_background_color(id, r, g, b, 1);
#if 1
			isq_ui_push();

			id = isq_ui_box(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND).id;
			isq_ui_semantic_size(id, (union isq_ui_sizes){
				.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 5 },
				.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 5 },
			});
			isq_ui_background_color(id, 1, 1, 1, 0.7);

			id = isq_ui_box(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND).id;
			isq_ui_position(id, 10, 10);
			isq_ui_semantic_size(id, (union isq_ui_sizes){
				.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 5 },
				.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 5 },
			});
			isq_ui_background_color(id, 0, 1, 0, 0.4);

			isq_ui_pop();
#endif
		}

		isq_ui_end();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
