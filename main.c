#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void rect_render(void *, size_t);

#define ISQ_UI_RECT_RENDER(buffer, count) rect_render(buffer, count)
#define ISQ_UI_IMPLEMENTATION
#include "isq_ui.h"

#define WIDTH 1920
#define HEIGHT 1080

u32 rect_shader;
u32 rect_vao;
u32 rect_vbo;
u32 rect_ebo;

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
	glShaderSource(vert_shader, 1, &vert_source, NULL);
	glCompileShader(vert_shader);
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vert_shader, 512, NULL, log);
		printf("ERROR COMPILING SHADER: %s\n", log);
		return 0;
	}

	u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_source, NULL);
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

		isq_ui_begin();
		isq_ui_dimensions(WIDTH, HEIGHT);

		u32 id = isq_ui_create(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND);

		isq_ui_semantic_size(id, (union isq_ui_sizes){
			.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 800 },
			.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 800 },
		});
		isq_ui_position(id, (vec2){ root_pos.x, root_pos.y });
		isq_ui_background_color(id, (vec4){ 1, 1, 1, 1 });

		isq_ui_push(id);

		id = isq_ui_create(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND);
		isq_ui_semantic_size(id, (union isq_ui_sizes){
			.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 600 },
			.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 600 },
		});
		isq_ui_position(id, (vec2){ 30, 30 });
		isq_ui_background_color(id, (vec4){ 1, 0, 0, 1 });

		isq_ui_push(id);

		id = isq_ui_create(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND);
		isq_ui_semantic_size(id, (union isq_ui_sizes){
			.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 100 },
			.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 100 },
		});
		isq_ui_position(id, (vec2){ 300, 30 });
		isq_ui_background_color(id, (vec4){ 0, 1, 0, 1 });

		id = isq_ui_create(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND);
		isq_ui_semantic_size(id, (union isq_ui_sizes){
			.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 100 },
			.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 100 },
		});
		isq_ui_position(id, (vec2){ 80, 30 });
		isq_ui_background_color(id, (vec4){ 0, 0, 1, 1 });

		isq_ui_pop();

		isq_ui_end();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
