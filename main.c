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

void rect_render(void *buffer, usize count)
{
	glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(struct isq_ui_vertex), buffer);

	struct isq_ui_vertex *v = (struct isq_ui_vertex*)buffer;

	glUseProgram(rect_shader);
	glBindVertexArray(rect_vao);
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
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

	glUseProgram(rect_shader);

	mat4 projection = mat4_ortho(0, WIDTH, 0, HEIGHT, -1, 1);
	glUniformMatrix4fv(glGetUniformLocation(rect_shader, "u_projection"), 1, GL_FALSE, &projection.data[0][0]);

	mat4 model = mat4_identity();
	glUniformMatrix4fv(glGetUniformLocation(rect_shader, "u_model"), 1, GL_FALSE, &model.data[0][0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2, 0.2, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		isq_ui_begin();

		u32 id = isq_ui_box_create(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND);

		isq_ui_box_set_semantic_size(id, (union isq_ui_sizes){
			.x = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 100 },
			.y = { .type = ISQ_UI_SIZE_TYPE_PIXELS, .value = 100 },
		});
		isq_ui_box_set_position(id, (vec2){ 100, 100 });
		isq_ui_box_set_background_color(id, (vec4){ 1, 1, 1, 1 });

		isq_ui_end();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
