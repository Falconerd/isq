// Usage:
// ISQ_UI_RECT_RENDER(buffer, count)
// must be defined by the user.
// buffer: pointer to an array of rects
// count: number of rects in the array
// rect:
//   vec4 min, max;
//   vec4 color;


// This will set the amount of ui elements
// that can be stored without resizing.
// If you know ahead of time the max number
// of elements, you can optimize for space
// by setting this macro.
// The buffers are not reset to the initial
// size once they have been expanded, and they
// expand by doubling.
#ifndef ISQ_UI_INITIAL_BUFFER_CAPACITY
#define ISQ_UI_INITIAL_BUFFER_CAPACITY 32
#endif

// Override the default printf by using this
// macro.
#ifndef ISQ_PRINTF
#include <stdio.h>
#define ISQ_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

// Can provide alternatives to malloc and free
// by defining the following macros:
#ifndef ISQ_MALLOC
#include <stdlib.h>
#define ISQ_MALLOC(x) malloc(x)
#define ISQ_CALLOC(n, u) calloc(n, u)
#define ISQ_FREE(x) free(x)
#define ISQ_REALLOC(x, u) realloc(x, u)
#endif

// Not sure how to customize vectors yet...
// Probably need to use custom ones for now.
// Prefixed to avoid collisions.
typedef union isq_vec2 {
	struct { float x, y; };
} isq_vec2;

typedef union isq_vec3 {
	struct { float x, y, z; };
} isq_vec3;

typedef union isq_vec4 {
	struct { float x, y, z, w; };
	float data[4];
} isq_vec4;

// Header section.
#ifndef ISQ_INCLUDE_ISQ_UI_H
#define ISQ_INCLUDE_ISQ_UI_H

enum isq_ui_box_flags {
	ISQ_UI_BOX_FLAG_NONE = 0,
	ISQ_UI_BOX_FLAG_HOVERABLE = 1 << 0,
	ISQ_UI_BOX_FLAG_CLICKABLE = 1 << 1,
	ISQ_UI_BOX_FLAG_DRAW_BACKGROUND = 1 << 2,
	ISQ_UI_BOX_FLAG_DRAW_BORDER = 1 << 3,
	ISQ_UI_BOX_FLAG_DRAW_TEXT = 1 << 4,
	ISQ_UI_BOX_FLAG_FLEX_ROW = 1 << 5,
	ISQ_UI_BOX_FLAG_FLEX_COLUMN = 1 << 6,
	ISQ_UI_BOX_FLAG_FLEX_NOWRAP = 1 << 7,
};

enum isq_ui_size_type {
	ISQ_UI_SIZE_TYPE_NULL,
	ISQ_UI_SIZE_TYPE_PIXELS,
	ISQ_UI_SIZE_TYPE_PERCENT,
	ISQ_UI_SIZE_TYPE_TEXT_CONTENT,
};

struct isq_ui_size {
	enum isq_ui_size_type type;
	float value;
	float strictness;
};

union isq_ui_sizes {
	struct { struct isq_ui_size x, y; };
	struct isq_ui_size data[2];
};

struct isq_ui_vertex {
	isq_vec3 position;
	isq_vec4 color;
};

struct isq_ui_state {
	unsigned id;
	unsigned char clicked;
	unsigned char hovered;
};

// Call ONCE before using anything.
// width and height are the dimensions of the UI
// area - typically the window.
void isq_ui_init(float width, float height);

// Call once per frame before using the functions
// in this header.
void isq_ui_begin(float mouse_x, float mouse_y);
// Call after using the functions in this header.
void isq_ui_end(void);

// Returns id.
struct isq_ui_state isq_ui_create(enum isq_ui_box_flags flags);

// Set the current parent on the stack.
unsigned isq_ui_push_id(unsigned id);
// Set the current parent to the last created
// box.
unsigned isq_ui_push(void);
unsigned isq_ui_pop(void);

// The following functions all return 0 on
// success. They are simple setters.
unsigned isq_ui_flags(unsigned id, enum isq_ui_box_flags flags);
unsigned isq_ui_semantic_size(unsigned id, union isq_ui_sizes semantic_size);
unsigned isq_ui_position(unsigned id, float x, float y);
unsigned isq_ui_background_color(unsigned id, float r, float g, float b, float a);
unsigned isq_ui_border(unsigned id, float r, float g, float b, float a, float width);

unsigned isq_ui_last_id(void);

// Flexbox stuff TODO
// Flexbox is a layer built on top of isq_ui_box
// that allows you to define a flexible
// layout. It's based on CSS's flexbox.
struct isq_ui_state isq_ui_flexbox(enum isq_ui_box_flags flags);
struct isq_ui_state isq_ui_box(enum isq_ui_box_flags flags);

// Premade components.

// Buttons by default are clickable, hoverable,
// and draw a background. These can be cancelled
// out by passing the same flags in.
// For example:
// isq_ui_button(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND)
// will now NOT draw a background.
struct isq_ui_state isq_ui_button(const char *text, float *background_color, float *hover_color, enum isq_ui_box_flags flags);

#endif

// Implementation section.
#ifdef ISQ_UI_IMPLEMENTATION

struct isq_ui_box {
	// Per frame.
	enum isq_ui_box_flags flags;
	union isq_ui_sizes semantic_size;
	isq_vec2 position;
	isq_vec4 background_color;
	isq_vec4 border_color;
	float border_width;
	isq_vec4 text_color;

	struct isq_ui_box *parent;
	struct isq_ui_box *first_child;
	struct isq_ui_box *next_sibling;
	struct isq_ui_box *prev_sibling;
	struct isq_ui_box *last_child;

	// Computed.
	isq_vec4 computed_rect;

	float flex_size;
	unsigned flex_count;
};

struct isq_ui_mouse {
	isq_vec2 position;
	// add buttons states here
};

static isq_vec2 isq_ui_dimensions = {0};
static struct isq_ui_mouse isq_ui_mouse = {0};

static struct isq_ui_box *isq_ui_box_array = NULL;
static unsigned isq_ui_box_array_capacity = 0;
static unsigned isq_ui_box_array_count = 0;

static struct isq_ui_box *isq_ui_current_parent = NULL;

static struct isq_ui_vertex *isq_ui_vertex_buffer = NULL;
static unsigned isq_ui_vertex_buffer_capacity = 0;
static unsigned isq_ui_vertex_buffer_count = 0;

static struct isq_ui_box *isq_ui_box_array_get(unsigned id) {
	if (id >= isq_ui_box_array_count) {
		return NULL;
	}

	return &isq_ui_box_array[id];
}

static void isq_ui_enqueue_rect(isq_vec4 rect, isq_vec4 color)
{
	if (isq_ui_vertex_buffer_count == isq_ui_vertex_buffer_capacity) {
		isq_ui_vertex_buffer_capacity *= 2;
		isq_ui_vertex_buffer = ISQ_REALLOC(isq_ui_vertex_buffer, sizeof(struct isq_ui_vertex) * isq_ui_vertex_buffer_capacity);
	}

	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.x = rect.x;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.y = rect.y;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.z = 0;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].color = color;
	isq_ui_vertex_buffer_count++;

	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.x = rect.z;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.y = rect.y;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.z = 0;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].color = color;
	isq_ui_vertex_buffer_count++;

	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.x = rect.z;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.y = rect.w;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.z = 0;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].color = color;
	isq_ui_vertex_buffer_count++;

	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.x = rect.x;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.y = rect.w;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].position.z = 0;
	isq_ui_vertex_buffer[isq_ui_vertex_buffer_count].color = color;
	isq_ui_vertex_buffer_count++;
}

isq_vec4 isq_vec4_add(isq_vec4 a, isq_vec4 b)
{
	isq_vec4 result = {0};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
	return result;
}

isq_vec4 isq_vec4_scale(isq_vec4 a, float k)
{
	isq_vec4 result = {0};
	result.x = a.x * k;
	result.y = a.y * k;
	result.z = a.z * k;
	result.w = a.w * k;
	return result;
}

static void isq_ui_enqueue_border(isq_vec4 rect, isq_vec4 color, float width)
{
	// Top
	isq_vec4 border_rect = rect;
	border_rect.y = rect.w - width;
	isq_ui_enqueue_rect(border_rect, color);

	// Bottom
	border_rect.y = rect.y;
	border_rect.w = rect.y + width;
	isq_ui_enqueue_rect(border_rect, color);

	// Left
	border_rect.y = rect.y + width;
	border_rect.z = rect.x + width;
	border_rect.w = rect.w - width;
	isq_ui_enqueue_rect(border_rect, color);

	// Right
	border_rect.x = rect.z - width;
	border_rect.z = rect.z;
	isq_ui_enqueue_rect(border_rect, color);
}

static struct isq_ui_state isq_ui_interact(unsigned id)
{
	struct isq_ui_state state = { .id = id };
	struct isq_ui_box *box = isq_ui_box_array_get(id);

	if (box->flags & ISQ_UI_BOX_FLAG_HOVERABLE) {
		if (isq_ui_mouse.position.x >= box->computed_rect.x && isq_ui_mouse.position.y >= box->computed_rect.y &&
			isq_ui_mouse.position.x < box->computed_rect.z && isq_ui_mouse.position.y < box->computed_rect.w) {
			state.hovered = 1;
		}
	}

	return state;
}

static void isq_ui_render(void)
{
	for (unsigned i = 0; i < isq_ui_box_array_count; ++i) {
		struct isq_ui_box *box = isq_ui_box_array_get(i);

		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BACKGROUND) {
			isq_ui_enqueue_rect(box->computed_rect, box->background_color);
		}

		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BORDER) {
			isq_ui_enqueue_border(box->computed_rect, box->border_color, box->border_width);
		}
	}

	ISQ_UI_RECT_RENDER(isq_ui_vertex_buffer, isq_ui_vertex_buffer_count);
}

static float isq_ui_compute_width(unsigned id, isq_vec2 origin, isq_vec2 parent_size)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 0;

	if (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_PIXELS)
		return box->semantic_size.x.value;

	if (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_PERCENT)
		return parent_size.x * box->semantic_size.x.value;

	// ...
	return 0;
}

static float isq_ui_compute_height(unsigned id, isq_vec2 origin, isq_vec2 parent_size)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 0;

	if (box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_PIXELS)
		return box->semantic_size.y.value;

	if (box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_PERCENT)
		return parent_size.y * box->semantic_size.y.value;

	return 0;
}

static void isq_ui_compute_rect(unsigned id)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (box == NULL)
		return;

	if ((box->position.x == (float)0xdeadbeef && box->position.y == (float)0xdeadbeef) || (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_NULL && box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_NULL))
		return;

	float yoffset = 0;

	// TODO
	// Perhaps a better way of approaching
	// this is to calculate the width and
	// height of the box, then check all the
	// conditions and place it in the correct
	// spot.
	isq_vec2 origin = { 0, 0 };
	isq_vec2 parent_size = isq_ui_dimensions;

	if (box->parent) {
		// TODO: Add padding
		origin.x = box->parent->computed_rect.x;
		origin.y = box->parent->computed_rect.y;
		parent_size.x = box->parent->computed_rect.z - box->parent->computed_rect.x;
		parent_size.y = box->parent->computed_rect.w - box->parent->computed_rect.y;
	}

	float width = isq_ui_compute_width(id, origin, parent_size);
	float height = isq_ui_compute_height(id, origin, parent_size);

	isq_vec2 position = {origin.x + box->position.x, origin.y + box->position.y};

	if (box->parent && box->parent->flags & ISQ_UI_BOX_FLAG_FLEX_ROW) {
		if (height > box->parent->flex_size)
			box->parent->flex_size = height;

		if (box->prev_sibling) {
			position.x = box->prev_sibling->computed_rect.z;

			if (position.x >= box->parent->computed_rect.z) {
				position.x = box->parent->computed_rect.x;
				box->parent->flex_count += 1;
			}

			position.y += box->parent->flex_count * box->parent->flex_size;
		}
	}

	// TODO: Add other types of position... (percentage based, for example)
	box->computed_rect.x = position.x;
	box->computed_rect.y = position.y;

	box->computed_rect.z = box->computed_rect.x + width;
	box->computed_rect.w = box->computed_rect.y + height;
}

void isq_ui_init(float width, float height)
{
	isq_ui_dimensions.x = width;
	isq_ui_dimensions.y = height;

	isq_ui_box_array = ISQ_MALLOC(sizeof(struct isq_ui_box) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
	isq_ui_box_array_capacity = ISQ_UI_INITIAL_BUFFER_CAPACITY;
	isq_ui_vertex_buffer = ISQ_MALLOC(sizeof(struct isq_ui_vertex) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
	isq_ui_vertex_buffer_capacity = ISQ_UI_INITIAL_BUFFER_CAPACITY;
}

void isq_ui_begin(float mouse_x, float mouse_y)
{
	isq_ui_mouse.position.x = mouse_x;
	isq_ui_mouse.position.y = mouse_y;

	isq_ui_current_parent = NULL;
	isq_ui_box_array_count = 0;
	isq_ui_vertex_buffer_count = 0;
}

void isq_ui_end(void)
{
	isq_ui_render();
}

unsigned isq_ui_push(void)
{
	struct isq_ui_box *box = isq_ui_box_array_get(isq_ui_box_array_count - 1);
	if (box == NULL)
		return 1;

	box->parent = isq_ui_current_parent;
	isq_ui_current_parent = box;

	return 0;
}

unsigned isq_ui_push_id(unsigned id)
{
	if (isq_ui_current_parent == NULL) {
		isq_ui_current_parent = isq_ui_box_array_get(id);
		return 0;
	}

	struct isq_ui_box *parent = isq_ui_current_parent;
	struct isq_ui_box *child = isq_ui_box_array_get(id);
	if (parent == NULL || child == NULL)
		return 1;

	child->parent = parent;
	isq_ui_current_parent = child;

	return 0;
}

unsigned isq_ui_pop()
{
	if (isq_ui_current_parent == NULL)
		return 1;

	isq_ui_current_parent = isq_ui_current_parent->parent;

	return 0;
}

static struct isq_ui_box *prev_sibling(unsigned id)
{
	if (id == 0)
		return NULL;

	struct isq_ui_box *curr = isq_ui_box_array_get(id - 1);
	for (unsigned i = id - 1; i > 0; --i) {
		struct isq_ui_box *curr = isq_ui_box_array_get(i);
		if (curr->parent == isq_ui_current_parent)
			return curr;
	}

	return NULL;
}

struct isq_ui_state isq_ui_create(enum isq_ui_box_flags flags)
{
	// Expand the box array if needed.
	if (isq_ui_box_array_count == isq_ui_box_array_capacity) {
		isq_ui_box_array_capacity *= 2;
		isq_ui_box_array = ISQ_REALLOC(isq_ui_box_array, sizeof(struct isq_ui_box) * isq_ui_box_array_capacity);
	}

	unsigned index = isq_ui_box_array_count;
	struct isq_ui_box *box = &isq_ui_box_array[index];

	// Set to a magic value to detect
	// uninitialized values.
	box->position = (isq_vec2){ (float)0xdeadbeef, (float)0xdeadbeef };
	box->background_color = (isq_vec4){ 0, 0, 0, 0 };
	box->semantic_size = (union isq_ui_sizes){
		.x = { .type = ISQ_UI_SIZE_TYPE_NULL, .value = 0 },
		.y = { .type = ISQ_UI_SIZE_TYPE_NULL, .value = 0 },
	};
	box->flags = flags;
	box->parent = isq_ui_current_parent;

	box->first_child = NULL;

	box->prev_sibling = prev_sibling(index);
	if (box->prev_sibling) {
		box->prev_sibling->next_sibling = box;
	}

	box->last_child = NULL;

	box->flex_size = 0;
	box->flex_count = 0;

	isq_ui_box_array_count++;

	return isq_ui_interact(index);
}

unsigned isq_ui_flags(unsigned id, enum isq_ui_box_flags flags)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);

	if (box) {
		box->flags = flags;
		return 0;
	}

	return 1;
}

unsigned isq_ui_semantic_size(unsigned id, union isq_ui_sizes semantic_size)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->semantic_size = semantic_size;
	isq_ui_compute_rect(id);
	return 0;
}

unsigned isq_ui_position(unsigned id, float x, float y)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->position = (isq_vec2){ x, y };
	isq_ui_compute_rect(id);
	return 0;
}

unsigned isq_ui_background_color(unsigned id, float r, float g, float b, float a)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->background_color = (isq_vec4){ r, g, b, a };
	return 0;
}

unsigned isq_ui_border(unsigned id, float r, float g, float b, float a, float width)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->border_color = (isq_vec4){ r, g, b, a };
	box->border_width = width;
	return 0;
}

unsigned isq_ui_last_id(void)
{
	return isq_ui_box_array_count - 1;
}

struct isq_ui_state isq_ui_flexbox(enum isq_ui_box_flags flags)
{
	enum isq_ui_box_flags flex_direction = (flags & ISQ_UI_BOX_FLAG_FLEX_COLUMN) ? ISQ_UI_BOX_FLAG_FLEX_COLUMN : ISQ_UI_BOX_FLAG_FLEX_ROW;
	struct isq_ui_state state = isq_ui_create(flags | flex_direction);

	isq_ui_push_id(state.id);

	return state;
}

struct isq_ui_state isq_ui_box(enum isq_ui_box_flags flags)
{
	struct isq_ui_state state = isq_ui_create(flags);

	isq_ui_position(state.id, 0, 0);

	return state;
}

void isq_ui_text(const char *text)
{
	// TODO
}

struct isq_ui_state isq_ui_button(const char *text, float *background_color, float *hover_color, enum isq_ui_box_flags flags)
{
	enum isq_ui_box_flags default_flags = ISQ_UI_BOX_FLAG_DRAW_BACKGROUND | ISQ_UI_BOX_FLAG_HOVERABLE | ISQ_UI_BOX_FLAG_CLICKABLE;
	flags ^= default_flags;
	struct isq_ui_state state = isq_ui_box(flags);

	isq_ui_position(state.id, 0, 0);
	isq_ui_semantic_size(state.id, (union isq_ui_sizes){
		.x = { .type = ISQ_UI_SIZE_TYPE_TEXT_CONTENT },
		.y = { .type = ISQ_UI_SIZE_TYPE_TEXT_CONTENT },
	});

	if (state.hovered)
		isq_ui_background_color(state.id, hover_color[0], hover_color[1], hover_color[2], hover_color[3]);
	else
		isq_ui_background_color(state.id, background_color[0], background_color[1], background_color[2], background_color[3]);
	isq_ui_text(text);

	return state;
}

#endif

