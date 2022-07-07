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

// You can override these types by defining
// ISQ_INTTYPES_DEFINED and providing
// alternatives.
#ifndef ISQ_INTTYPES_DEFINED
#define ISQ_INTTYPES_DEFINED
#include <stdint.h>
#include <stdlib.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;
#endif

// You can override these types by defining
// ISQ_FLOATTYPES_DEFINED and providing
// alternatives.
#ifndef ISQ_FLOATTYPES_DEFINED
#define ISQ_FLOATTYPES_DEFINED
typedef float f32;
typedef double f64;
#endif

// You can override this type by defining
// ISQ_VEC2_DEFINED and providing
// an alternative.
#ifndef ISQ_VEC2_DEFINED
#define ISQ_VEC2_DEFINED
typedef union {
	struct { f32 x, y; };
	struct { f32 u, v; };
	f32 data[2];
} vec2;

vec2 vec2_zero = { 0, 0 };
#endif

// You can override this type by defining
// ISQ_VEC3_DEFINED and providing
// an alternative.
#ifndef ISQ_VEC3_DEFINED
#define ISQ_VEC3_DEFINED
typedef union {
	struct { f32 x, y, z; };
	struct { f32 r, g, b; };
	f32 data[3];
} vec3;
#endif

// You can override this type by defining
// ISQ_VEC4_DEFINED and providing
// an alternative.
#ifndef ISQ_VEC4_DEFINED
#define ISQ_VEC4_DEFINED
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
#endif

#ifndef ISQ_MAT4_DEFINED
#define ISQ_MAT4_DEFINED
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
#endif

// Header section.
#ifndef __ISQ_INCLUDE_ISQ_UI_H__
#define __ISQ_INCLUDE_ISQ_UI_H__

enum isq_ui_box_flags {
	ISQ_UI_BOX_FLAG_NONE = 0,
	ISQ_UI_BOX_FLAG_HOVERABLE = 1 << 1,
	ISQ_UI_BOX_FLAG_CLICKABLE = 1 << 2,
	ISQ_UI_BOX_FLAG_DRAW_BACKGROUND = 1 << 3,
	ISQ_UI_BOX_FLAG_DRAW_BORDER = 1 << 4,
	ISQ_UI_BOX_FLAG_DRAW_TEXT = 1 << 5,
};

enum isq_ui_size_type {
	ISQ_UI_SIZE_TYPE_NULL,
	ISQ_UI_SIZE_TYPE_PIXELS,
	ISQ_UI_SIZE_TYPE_PERCENT,
	ISQ_UI_SIZE_TYPE_TEXT_CONTENT,
	ISQ_UI_SIZE_TYPE_INVALID
};

struct isq_ui_size {
	enum isq_ui_size_type type;
	f32 value;
	f32 strictness;
};

union isq_ui_sizes {
	struct { struct isq_ui_size x, y; };
	struct isq_ui_size data[2];
};

struct isq_ui_vertex {
	vec3 position;
	vec4 color;
};

// Call before using any of the functions in this header.
void isq_ui_begin(void);
// Call after using the functions in this header.
void isq_ui_end(void);

// Returns id.
u32 isq_ui_create(enum isq_ui_box_flags flags);

// Set the current parent on the stack.
u8 isq_ui_push(u32 id);

// Set the current parent to the last pushed parent.
u8 isq_ui_pop(void);

// The following functions all return 0 on
// success. They are simple setters.
u8 isq_ui_flags(u32 id, enum isq_ui_box_flags flags);
u8 isq_ui_semantic_size(u32 id, union isq_ui_sizes semantic_size);
u8 isq_ui_position(u32 id, vec2 position);
u8 isq_ui_background_color(u32 id, vec4 color);
// Set the dimensions (usually window size).
u8 isq_ui_dimensions(f32 width, f32 height);

#endif

// Implementation section.
#ifdef ISQ_UI_IMPLEMENTATION

struct isq_ui_box {
	// Per frame.
	enum isq_ui_box_flags flags;
	union isq_ui_sizes semantic_size;
	vec2 position;
	vec4 background_color;
	vec4 border_color;
	vec4 text_color;

	struct isq_ui_box *parent;
	struct isq_ui_box *first_child;
	struct isq_ui_box *next_sibling;
	struct isq_ui_box *prev_sibling;
	struct isq_ui_box *last_child;

	// Computed.
	vec4 computed_rect;
};

static u8 isq_ui_initialized = 0;
static f32 isq_ui_width = 0;
static f32 isq_ui_height = 0;

static struct isq_ui_box *isq_ui_box_array = NULL;
static u32 isq_ui_box_array_capacity = 0;
static u32 isq_ui_box_array_count = 0;

static struct isq_ui_box *isq_ui_current_parent = NULL;

static struct isq_ui_vertex *isq_ui_vertex_buffer = NULL;
static u32 isq_ui_vertex_buffer_capacity = 0;
static u32 isq_ui_vertex_buffer_count = 0;

static void isq_ui_init(void)
{
	isq_ui_initialized = 1;

	isq_ui_box_array = ISQ_MALLOC(sizeof(struct isq_ui_box) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
	isq_ui_box_array_capacity = ISQ_UI_INITIAL_BUFFER_CAPACITY;
	isq_ui_vertex_buffer = ISQ_MALLOC(sizeof(struct isq_ui_vertex) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
	isq_ui_vertex_buffer_capacity = ISQ_UI_INITIAL_BUFFER_CAPACITY;
}

u8 isq_ui_dimensions(f32 width, f32 height)
{
	isq_ui_width = width;
	isq_ui_height = height;
}

static struct isq_ui_box *isq_ui_box_array_get(u32 id) {
	if (id >= isq_ui_box_array_count) {
		return NULL;
	}

	return &isq_ui_box_array[id];
}

static void isq_ui_enqueue_rect(vec4 rect, vec4 color)
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

static void isq_ui_render(void)
{
	for (u32 i = 0; i < isq_ui_box_array_count; ++i) {
		struct isq_ui_box *box = isq_ui_box_array_get(i);
		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BACKGROUND) {
			isq_ui_enqueue_rect(box->computed_rect, box->background_color);
		}
	}

	ISQ_UI_RECT_RENDER(isq_ui_vertex_buffer, isq_ui_vertex_buffer_count);
}

void isq_ui_compute_rect(u32 id)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (box == NULL)
		return;

	if ((box->position.x == 0xdeadbeef && box->position.y == 0xdeadbeef) || (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_NULL && box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_NULL))
		return;

	if (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_PIXELS) {
		if (box->parent) {
			box->computed_rect.x = box->parent->computed_rect.x + box->position.x;
			box->computed_rect.z = box->computed_rect.x + box->semantic_size.x.value;
		} else {
			box->computed_rect.x = box->position.x;
			box->computed_rect.z = box->position.x + box->semantic_size.x.value;
		}
	}

	if (box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_PIXELS) {
		if (box->parent) {
			box->computed_rect.y = box->parent->computed_rect.y + box->position.y;
			box->computed_rect.w = box->computed_rect.y + box->semantic_size.y.value;
		} else {
			box->computed_rect.y = box->position.y;
			box->computed_rect.w = box->position.y + box->semantic_size.y.value;
		}
	}
}

void isq_ui_begin(void)
{
	if (!isq_ui_initialized)
		isq_ui_init();

	isq_ui_current_parent = NULL;
	isq_ui_box_array_count = 0;
	isq_ui_vertex_buffer_count = 0;
}

void isq_ui_end(void)
{
	isq_ui_render();
}

u8 isq_ui_push(u32 id)
{
	if (isq_ui_current_parent == NULL) {
		isq_ui_current_parent = isq_ui_box_array_get(id);
		return 0;
	} else {
		struct isq_ui_box *parent = isq_ui_current_parent;
		struct isq_ui_box *child = isq_ui_box_array_get(id);
		if (parent == NULL || child == NULL)
			return 1;

		child->parent = parent;
		isq_ui_current_parent = child;
		return 0;
	};

	return 1;
}

u8 isq_ui_pop()
{
	if (isq_ui_current_parent == NULL)
		return 1;

	isq_ui_current_parent = isq_ui_current_parent->parent;

	return 0;
}

u32 isq_ui_create(enum isq_ui_box_flags flags)
{
	// Expand the box array if needed.
	if (isq_ui_box_array_count == isq_ui_box_array_capacity) {
		isq_ui_box_array_capacity *= 2;
		isq_ui_box_array = ISQ_REALLOC(isq_ui_box_array, sizeof(struct isq_ui_box) * isq_ui_box_array_capacity);
	}

	u32 index = isq_ui_box_array_count;

	// Create the box.
	struct isq_ui_box *box = &isq_ui_box_array[index];

	// Set to a magic value to detect uninitialized values.
	box->position = (vec2){ 0xdeadbeef, 0xdeadbeef };
	box->background_color = vec4_zero;
	box->semantic_size = (union isq_ui_sizes){
		.x = { .type = ISQ_UI_SIZE_TYPE_NULL, .value = 0 },
		.y = { .type = ISQ_UI_SIZE_TYPE_NULL, .value = 0 },
	};
	box->flags = flags;
	box->parent = isq_ui_current_parent;
	box->first_child = NULL;
	box->next_sibling = NULL;
	box->prev_sibling = NULL;
	box->last_child = NULL;

	isq_ui_box_array_count++;

	return index;
}

u8 isq_ui_flags(u32 id, enum isq_ui_box_flags flags)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);

	if (box) {
		box->flags = flags;
		return 0;
	}

	return 1;
}

u8 isq_ui_semantic_size(u32 id, union isq_ui_sizes semantic_size)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->semantic_size = semantic_size;
	isq_ui_compute_rect(id);
	return 0;
}

u8 isq_ui_position(u32 id, vec2 position)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->position = position;
	isq_ui_compute_rect(id);
	return 0;
}

u8 isq_ui_background_color(u32 id, vec4 color)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->background_color = color;
	return 0;
}

#endif
