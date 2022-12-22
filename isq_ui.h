// Usage:
// ISQ_UI_RENDER_RECT(buffer, count)
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

#ifndef ISQ_STRLEN
#include <string.h>
#define ISQ_STRLEN(s) strlen(s)
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

// Header section.
#ifndef ISQ_INCLUDE_ISQ_UI_H
#define ISQ_INCLUDE_ISQ_UI_H

// Not sure how to customize vectors yet...
// Probably need to use custom ones for now.
// Prefixed to avoid collisions.
typedef union isq_vec2 {
	struct { float x, y; };
	struct { float u, v; };
} isq_vec2;

typedef union isq_vec3 {
	struct { float x, y, z; };
} isq_vec3;

typedef union isq_vec4 {
	struct { float x, y, z, w; };
	struct { float x1, y1, x2, y2; };
	struct { float r, g, b, a; };
	struct { float top, right, bottom, left; };
	float data[4];
} isq_vec4;

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
	ISQ_UI_BOX_FLAG_TEXT_NOWRAP = 1 << 8, // Default behaviour.
	ISQ_UI_BOX_FLAG_TEXT_CENTER = 1 << 9,
	ISQ_UI_BOX_FLAG_TEXT_WRAP_WORD = 1 << 10,
	ISQ_UI_BOX_FLAG_POSITION_ABSOLUTE = 1 << 11,
	ISQ_UI_BOX_FLAG_SCROLL_HORIZONTAL = 1 << 12,
	ISQ_UI_BOX_FLAG_SCROLL_VERTICAL = 1 << 13,
	ISQ_UI_BOX_FLAG_DRAW_SCROLLBAR = 1 << 14,
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

// x, y, z, u, v, texture_index, r, g, b, a
struct isq_ui_vertex {
	isq_vec3 position;
	isq_vec2 uvs;
	float texture_index;
	isq_vec4 color;
};

struct isq_ui_state {
	unsigned id;
	unsigned char clicked;
	unsigned char hovered;
};

struct isq_ui_font {
	void *character_data;
	unsigned texture_index;
	unsigned size;
};

// Copy of stbtt_aligned_quad.
struct isq_ui_aligned_quad {
	float x0, y0, s0, t0;
	float x1, y1, s1, t1;
};

struct isq_ui_box_style {
	isq_vec4 background_color;
	isq_vec4 hover_color;
	isq_vec4 border_color;
	isq_vec4 text_color;
	isq_vec4 padding;
	float border_width;
	float border_radius;
	struct isq_ui_font font;
	float flex_gap;
};

// Style used by default unless specifically
// overridden by the user.
struct isq_ui_style {
	struct isq_ui_box_style box;
	struct isq_ui_box_style button;
};

// Call ONCE before using anything.
// width and height are the dimensions of the UI
// area - typically the window.
void isq_ui_init(float width, float height, struct isq_ui_style *style);

// Call once per frame before using the functions
// in this header.
void isq_ui_begin(float mouse_x, float mouse_y, int left_down, float scroll_delta);
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
unsigned isq_ui_pop_all(void);

// The following functions all return 0 on
// success. They are simple setters.
unsigned isq_ui_flags(unsigned id, enum isq_ui_box_flags flags);
unsigned isq_ui_flags_add(unsigned id, enum isq_ui_box_flags flags);
unsigned isq_ui_flags_remove(unsigned id, enum isq_ui_box_flags flags);
unsigned isq_ui_semantic_size(unsigned id, union isq_ui_sizes semantic_size);
unsigned isq_ui_position(unsigned id, float x, float y);
unsigned isq_ui_background_color(unsigned id, float r, float g, float b, float a);
unsigned isq_ui_border(unsigned id, float r, float g, float b, float a, float width);
unsigned isq_ui_padding(unsigned id, float top, float right, float bottom, float left);
unsigned isq_ui_parent(unsigned id, unsigned parent_id);
unsigned isq_ui_font(unsigned id, void *character_data, unsigned size, unsigned texture_index);
unsigned isq_ui_text_color(unsigned id, float r, float g, float b, float a);

// Set size in pixels.
unsigned isq_ui_size(unsigned id, float w, float h);

// Getters, gross!
void isq_ui_get_size(unsigned id, float *width, float *height);
void isq_ui_get_position(unsigned id, float *x, float *y);

unsigned isq_ui_last_id(void);

// Flexbox is a layer built on top of isq_ui_box
// that allows you to define a flexible
// layout. It's based on CSS's flexbox.
struct isq_ui_state isq_ui_flexbox(enum isq_ui_box_flags flags);
struct isq_ui_state isq_ui_box(enum isq_ui_box_flags flags);

// Premade components.

// Buttons by default are clickable, hoverable,
// and draw a background.
// Styles can be changed by using the various
// isq_ui_button_style... functions.
struct isq_ui_state isq_ui_button(const char *text);

// Just a box that displays text. Has word
// wrapping by default.
// Fills the entire of its parent.
// Scrolls vertically if the text is too long.
struct isq_ui_state isq_ui_text_display(const char *text);

#endif

// Implementation section.
//#define ISQ_UI_IMPLEMENTATION
#ifdef ISQ_UI_IMPLEMENTATION

#ifndef ISQ_UI_RENDER_RECT
#error "ISQ_UI_RENDER_RECT must be defined"
#endif

/*
#ifndef ISQ_UI_TEXT_RENDER
#error "ISQ_UI_TEXT_RENDER must be defined"
#endif
*/

#ifndef ISQ_UI_BAKED_QUAD_TYPE
#error "ISQ_UI_BAKED_QUAD_TYPE must be defined"
#endif

#ifndef ISQ_UI_BAKED_QUAD
#error "ISQ_UI_BAKED_QUAD must be defined"
#endif

#define ISQ_UI_MAGIC_NUMBERF (float)0xdeadbeef
#define ISQ_UI_MAGIC_NUMBERV4 (isq_vec4){ISQ_UI_MAGIC_NUMBERF, ISQ_UI_MAGIC_NUMBERF, ISQ_UI_MAGIC_NUMBERF, ISQ_UI_MAGIC_NUMBERF}

struct isq_ui_box {
	unsigned id;
	// Per frame.
	enum isq_ui_box_flags flags;
	union isq_ui_sizes semantic_size;
	isq_vec2 position;

	struct isq_ui_box_style style;

	const char *text;
	float text_width_in_pixels;
	unsigned text_line_count;

	struct isq_ui_box *parent;
	struct isq_ui_box *first_child;
	struct isq_ui_box *next_sibling;
	struct isq_ui_box *prev_sibling;
	struct isq_ui_box *last_child;

	// Computed.
	isq_vec4 computed_rect;

	float flex_size;
	unsigned flex_count;

	float scroll_offset;
	float scroll_offset_max;
};

struct isq_ui_mouse {
	isq_vec2 position;
	// add buttons states here
	int left_down;
	float scroll_delta;
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

static isq_vec4 isq_ui_default_uvs = {0, 0, 1, 1};

static struct isq_ui_style isq_ui_style = {0};

static unsigned isq_ui_used_bytes = 0;

static float isq_ui_scroll_multiplier = 30;

static struct isq_ui_box *isq_ui_box_array_get(unsigned id) {
	if (id >= isq_ui_box_array_count) {
		return NULL;
	}

	return &isq_ui_box_array[id];
}

static void isq_ui_enqueue_rect(isq_vec4 rect, isq_vec4 uvs, isq_vec4 color, float texture_index)
{
	if (isq_ui_vertex_buffer_count == isq_ui_vertex_buffer_capacity) {
		isq_ui_vertex_buffer_capacity *= 2;
		isq_ui_vertex_buffer = ISQ_REALLOC(isq_ui_vertex_buffer, sizeof(struct isq_ui_vertex) * isq_ui_vertex_buffer_capacity);
	}

	struct isq_ui_vertex *vertex = &isq_ui_vertex_buffer[isq_ui_vertex_buffer_count++];

	vertex->position.x = rect.x;
	vertex->position.y = rect.y;
	vertex->position.z = 0;
	vertex->color = color;
	vertex->texture_index = texture_index;
	vertex->uvs.u = uvs.x;
	vertex->uvs.v = uvs.y;

	vertex = &isq_ui_vertex_buffer[isq_ui_vertex_buffer_count++];

	vertex->position.x = rect.z;
	vertex->position.y = rect.y;
	vertex->position.z = 0;
	vertex->color = color;
	vertex->texture_index = texture_index;
	vertex->uvs.u = uvs.z;
	vertex->uvs.v = uvs.y;

	vertex = &isq_ui_vertex_buffer[isq_ui_vertex_buffer_count++];

	vertex->position.x = rect.z;
	vertex->position.y = rect.w;
	vertex->position.z = 0;
	vertex->color = color;
	vertex->texture_index = texture_index;
	vertex->uvs.u = uvs.z;
	vertex->uvs.v = uvs.w;
	
	vertex = &isq_ui_vertex_buffer[isq_ui_vertex_buffer_count++];

	vertex->position.x = rect.x;
	vertex->position.y = rect.w;
	vertex->position.z = 0;
	vertex->color = color;
	vertex->texture_index = texture_index;
	vertex->uvs.u = uvs.x;
	vertex->uvs.v = uvs.w;
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
	isq_ui_enqueue_rect(border_rect, isq_ui_default_uvs, color, 0);

	// Bottom
	border_rect.y = rect.y;
	border_rect.w = rect.y + width;
	isq_ui_enqueue_rect(border_rect, isq_ui_default_uvs, color, 0);

	// Left
	border_rect.y = rect.y + width;
	border_rect.z = rect.x + width;
	border_rect.w = rect.w - width;
	isq_ui_enqueue_rect(border_rect, isq_ui_default_uvs, color, 0);

	// Right
	border_rect.x = rect.z - width;
	border_rect.z = rect.z;
	isq_ui_enqueue_rect(border_rect, isq_ui_default_uvs, color, 0);
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

	if (box->flags & ISQ_UI_BOX_FLAG_CLICKABLE && isq_ui_mouse.left_down > 0) {
		if (isq_ui_mouse.position.x >= box->computed_rect.x && isq_ui_mouse.position.y >= box->computed_rect.y &&
			isq_ui_mouse.position.x < box->computed_rect.z && isq_ui_mouse.position.y < box->computed_rect.w) {
			state.clicked = 1;
		}
	}

	if (box->flags & ISQ_UI_BOX_FLAG_SCROLL_VERTICAL && isq_ui_mouse.scroll_delta != 0) {
		box->scroll_offset -= isq_ui_mouse.scroll_delta * isq_ui_scroll_multiplier;

		if (box->scroll_offset < 0)
			box->scroll_offset = 0;
		if (box->scroll_offset > box->scroll_offset_max)
			box->scroll_offset = box->scroll_offset_max;
	}

	return state;
}

// TODO: 
// - Word wrapping.
// - Text alignment.
// - Allow alphabets besides English.
// - Rounded corners?
static void isq_ui_render(void)
{
	for (unsigned i = 0; i < isq_ui_box_array_count; ++i) {
		struct isq_ui_box *box = isq_ui_box_array_get(i);

		bool cutoff_top = false;
		float cutoff_size = 0;

		// Don't show boxes past parent.
		if (box->parent && box->parent->flags & ISQ_UI_BOX_FLAG_SCROLL_VERTICAL) {
			// Scroll offset.
			box->computed_rect.y -= box->parent->scroll_offset;
			box->computed_rect.w -= box->parent->scroll_offset;

			// Don't display if scrolled off screen.
			if (box->computed_rect.y > box->parent->computed_rect.w)
				continue;
			
			if (box->computed_rect.w < box->parent->computed_rect.y)
				continue;

			// Clamp to parent.
			if (box->computed_rect.w > box->parent->computed_rect.w) {
				box->computed_rect.w = box->parent->computed_rect.w;
			}

			if (box->computed_rect.y < box->parent->computed_rect.y) {
				cutoff_size = box->parent->computed_rect.y - box->computed_rect.y;
				box->computed_rect.y = box->parent->computed_rect.y;
				cutoff_top = true;
			}
		}

		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BACKGROUND)
			isq_ui_enqueue_rect(box->computed_rect, isq_ui_default_uvs, box->style.background_color, 0);

		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BORDER)
			isq_ui_enqueue_border(box->computed_rect, box->style.border_color, box->style.border_width);

		// Only draw text if it exsits. 
		if (box->text) {
			const char *text = box->text;

			isq_vec2 pos = {box->computed_rect.x + box->style.padding.left, box->computed_rect.y + box->style.padding.top};
			ISQ_UI_BAKED_QUAD_TYPE q;

			while (text && *text) {
				if (*text < 32) {
					++text;
					continue;
				}

				ISQ_UI_BAKED_QUAD(box->style.font.character_data, 512, 512, *text-32, &pos.x, &pos.y, &q, 1);

				++text;

				isq_vec4 text_rect = (isq_vec4){q.x0, q.y0 + box->style.font.size * 0.75, q.x1, q.y1 + box->style.font.size * 0.75};
				isq_vec4 text_uvs = (isq_vec4){q.s0, q.t0, q.s1, q.t1};

				if (box->parent && box->parent->flags & ISQ_UI_BOX_FLAG_SCROLL_VERTICAL) {
					if (text_rect.w > box->parent->computed_rect.w) {
						float size = text_rect.w - text_rect.y;
						float pct = (box->parent->computed_rect.w - text_rect.y) / size;

						text_rect.w = box->parent->computed_rect.w;
						text_uvs.w = q.t0 + (q.t1 - q.t0) * pct;

						if (text_rect.w > box->parent->computed_rect.w)
							text_rect.w = box->parent->computed_rect.w;
						
						if (text_rect.y > box->parent->computed_rect.w)
							text_rect.y = box->parent->computed_rect.w;
					}

					if (cutoff_top) {
						float size = text_rect.w - text_rect.y;
						float pct = 1.0;

						printf("size: %f pct: %f\n", size, pct);

						text_rect.y -= cutoff_size;
						text_rect.w -= cutoff_size;

						if (text_rect.y < box->parent->computed_rect.y)
							text_rect.y = box->parent->computed_rect.y;

						if (text_rect.w < box->parent->computed_rect.y)
							text_rect.w = box->parent->computed_rect.y;

						// TODO: Update uvs to match new size.
						text_uvs.w = q.t0 + (q.t1 - q.t0) * pct;
					}
				}

				isq_ui_enqueue_rect(text_rect, text_uvs, box->style.text_color, box->style.font.texture_index);
			}
		}
	}

	ISQ_UI_RENDER_RECT(isq_ui_vertex_buffer, isq_ui_vertex_buffer_count);
}

static float get_text_width_in_pixels(struct isq_ui_font font, const char *text)
{
	float width_start = ISQ_UI_MAGIC_NUMBERF;
	isq_vec2 pos = {0};
	ISQ_UI_BAKED_QUAD_TYPE q;

	while (text && *text) {
		if (*text < 32) {
			// TODO: Next line, line count.
			++text;
			continue;
		}

		ISQ_UI_BAKED_QUAD(font.character_data, 512, 512, *text-32, &pos.x, &pos.y, &q, 1);

		if (width_start == ISQ_UI_MAGIC_NUMBERF)
			width_start = q.x0;

		++text;
	}

	return q.x1 - width_start;
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

	if (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_TEXT_CONTENT) {
		float text_width = get_text_width_in_pixels(box->style.font, box->text);
		return text_width + box->style.padding.left + box->style.padding.right;
	}

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

	if (box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_TEXT_CONTENT) {
		return box->style.font.size + box->style.padding.top + box->style.padding.bottom;
		//return box->style.font.size * box->text_line_count + box->style.padding.top + box->style.padding.bottom;
	}

	return 0;
}

static void isq_ui_compute_rect(unsigned id)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (box == NULL)
		return;

	if ((box->position.x == (float)ISQ_UI_MAGIC_NUMBERF && box->position.y == (float)ISQ_UI_MAGIC_NUMBERF) || (box->semantic_size.x.type == ISQ_UI_SIZE_TYPE_NULL && box->semantic_size.y.type == ISQ_UI_SIZE_TYPE_NULL))
		return;

	float yoffset = 0;

	isq_vec2 origin = { 0, 0 };
	isq_vec2 parent_size = isq_ui_dimensions;

	if (box->parent) {
		origin.x = box->parent->computed_rect.x + box->parent->style.padding.left;
		origin.y = box->parent->computed_rect.y + box->parent->style.padding.top;
		parent_size.x = box->parent->computed_rect.z - box->parent->computed_rect.x - box->parent->style.padding.left - box->parent->style.padding.right;
		parent_size.y = box->parent->computed_rect.w - box->parent->computed_rect.y - box->parent->style.padding.top - box->parent->style.padding.bottom;
	}

	float width = isq_ui_compute_width(id, origin, parent_size) + box->style.padding.left + box->style.padding.right;
	float height = isq_ui_compute_height(id, origin, parent_size) + box->style.padding.top + box->style.padding.bottom;

	box->scroll_offset_max = height;

	isq_vec2 position = {origin.x + box->position.x, origin.y + box->position.y};

	if (box->flags & ISQ_UI_BOX_FLAG_POSITION_ABSOLUTE) {
		position.x = box->position.x;
		position.y = box->position.y;
	} else if (box->parent && box->parent->flags & ISQ_UI_BOX_FLAG_FLEX_ROW) {
		if (height > box->parent->flex_size)
			box->parent->flex_size = height;

		if (box->prev_sibling) {
			position.x = box->prev_sibling->computed_rect.z + box->parent->style.flex_gap;

			if (position.x >= box->parent->computed_rect.z) {
				position.x = box->parent->computed_rect.x;
				box->parent->flex_count += 1;
			}

			position.y += box->parent->flex_count * box->parent->flex_size;
		}
	} else if (box->parent && box->parent->flags & ISQ_UI_BOX_FLAG_FLEX_COLUMN) {
		if (width > box->parent->flex_size)
			box->parent->flex_size = width;

		if (box->prev_sibling) {
			position.y = box->prev_sibling->computed_rect.w + box->parent->style.flex_gap;

			if (position.y >= box->parent->computed_rect.w && !(box->parent->flags & ISQ_UI_BOX_FLAG_FLEX_NOWRAP)) {
				position.y = box->parent->computed_rect.y;
				box->parent->flex_count += 1;
			}

			position.x += box->parent->flex_count * box->parent->flex_size;
		}
	}


	// TODO: Add other types of position... (percentage based, for example)
	box->computed_rect.x = position.x;
	box->computed_rect.y = position.y;

	box->computed_rect.z = box->computed_rect.x + width - box->style.padding.left - box->style.padding.right;
	box->computed_rect.w = box->computed_rect.y + height - box->style.padding.top - box->style.padding.bottom;
}

void isq_ui_init(float width, float height, struct isq_ui_style *style)
{
	isq_ui_dimensions.x = width;
	isq_ui_dimensions.y = height;

	isq_ui_style = *style;

	isq_ui_box_array = ISQ_MALLOC(sizeof(struct isq_ui_box) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
	isq_ui_box_array_capacity = ISQ_UI_INITIAL_BUFFER_CAPACITY;
	isq_ui_vertex_buffer = ISQ_MALLOC(sizeof(struct isq_ui_vertex) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
	isq_ui_vertex_buffer_capacity = ISQ_UI_INITIAL_BUFFER_CAPACITY;

	memset(isq_ui_box_array, 0, sizeof(struct isq_ui_box) * ISQ_UI_INITIAL_BUFFER_CAPACITY);
}

void isq_ui_begin(float mouse_x, float mouse_y, int left_down, float scroll_delta)
{
	isq_ui_mouse.position.x = mouse_x;
	isq_ui_mouse.position.y = mouse_y;
	isq_ui_mouse.left_down = left_down;
	isq_ui_mouse.scroll_delta = scroll_delta;

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

unsigned isq_ui_pop(void)
{
	if (isq_ui_current_parent == NULL)
		return 1;

	isq_ui_current_parent = isq_ui_current_parent->parent;

	return 0;
}

unsigned isq_ui_pop_all(void)
{
	isq_ui_current_parent = NULL;
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
	box->id = index;

	// Set to a magic value to detect
	// uninitialized values.
	box->position = (isq_vec2){ ISQ_UI_MAGIC_NUMBERF, ISQ_UI_MAGIC_NUMBERF };

	box->style.background_color = isq_ui_style.box.background_color;
	box->style.border_color = isq_ui_style.box.border_color;
	box->style.padding = isq_ui_style.box.padding;
	box->style.text_color = isq_ui_style.box.text_color;
	box->style.border_radius = isq_ui_style.box.border_radius;
	box->style.border_width = isq_ui_style.box.border_width;
	box->style.font.character_data = isq_ui_style.box.font.character_data;
	box->style.font.size = isq_ui_style.box.font.size;
	box->style.font.texture_index = isq_ui_style.box.font.texture_index;
	box->style.flex_gap = isq_ui_style.box.flex_gap;

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
	box->text = NULL;

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

unsigned isq_ui_flags_add(unsigned id, enum isq_ui_box_flags flags)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);

	if (box) {
		box->flags = flags | box->flags;
		return 0;
	}

	return 1;
}

unsigned isq_ui_flags_remove(unsigned id, enum isq_ui_box_flags flags)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);

	if (box) {
		box->flags = ~flags & box->flags;
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

unsigned isq_ui_size(unsigned id, float w, float h)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->semantic_size = (union isq_ui_sizes){
		.x = { .value = w, .type = ISQ_UI_SIZE_TYPE_PIXELS },
		.y = { .value = h, .type = ISQ_UI_SIZE_TYPE_PIXELS },
	};
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

	box->style.background_color = (isq_vec4){ r, g, b, a };
	return 0;
}

unsigned isq_ui_border(unsigned id, float r, float g, float b, float a, float width)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->style.border_color = (isq_vec4){ r, g, b, a };
	box->style.border_width = width;
	return 0;
}

unsigned isq_ui_padding(unsigned id, float top, float right, float bottom, float left)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->style.padding = (isq_vec4){ top, right, bottom, left };
	isq_ui_compute_rect(id);
	return 0;
}

unsigned isq_ui_parent(unsigned id, unsigned parent_id)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	if (parent_id == -1) {
		box->parent = NULL;
		return 0;
	}

	struct isq_ui_box *parent = isq_ui_box_array_get(parent_id);

	box->parent = parent;
	return 0;
}

unsigned isq_ui_font(unsigned id, void *character_data, unsigned size, unsigned texture_index)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->style.font.character_data = character_data;
	box->style.font.size = size;
	box->style.font.texture_index = texture_index;
	return 0;
}

unsigned isq_ui_text_color(unsigned id, float r, float g, float b, float a)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return 1;

	box->style.text_color = (isq_vec4){ r, g, b, a };
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

void isq_ui_text(unsigned id, const char *text)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return;

	box->text = text;
}

struct isq_ui_state isq_ui_button(const char *text)
{
	struct isq_ui_state state = isq_ui_box(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND | ISQ_UI_BOX_FLAG_DRAW_BORDER | ISQ_UI_BOX_FLAG_HOVERABLE | ISQ_UI_BOX_FLAG_CLICKABLE);

	// for debug
	struct isq_ui_box *box = isq_ui_box_array_get(state.id);

	isq_ui_text(state.id, text);
	isq_ui_position(state.id, 0, 0);
	isq_ui_semantic_size(state.id, (union isq_ui_sizes){
		.x = { .type = ISQ_UI_SIZE_TYPE_TEXT_CONTENT },
		.y = { .type = ISQ_UI_SIZE_TYPE_TEXT_CONTENT },
	});
	isq_ui_border(state.id, isq_ui_style.button.border_color.r, isq_ui_style.button.border_color.g, isq_ui_style.button.border_color.b, isq_ui_style.button.border_color.a, 1);
	isq_ui_padding(state.id, 5, 5, 5, 5);

	if (state.hovered)
		isq_ui_background_color(state.id, isq_ui_style.button.hover_color.r, isq_ui_style.button.hover_color.g, isq_ui_style.button.hover_color.b, isq_ui_style.button.hover_color.a);
	else
		isq_ui_background_color(state.id, isq_ui_style.button.background_color.r, isq_ui_style.button.background_color.g, isq_ui_style.button.background_color.b, isq_ui_style.button.background_color.a);

	return state;
}

struct isq_ui_state isq_ui_text_display(const char *text)
{
	struct isq_ui_state state = isq_ui_box(ISQ_UI_BOX_FLAG_DRAW_BACKGROUND | ISQ_UI_BOX_FLAG_DRAW_BORDER);

	isq_ui_text(state.id, text);
	isq_ui_position(state.id, 0, 0);
	isq_ui_semantic_size(state.id , (union isq_ui_sizes){
		.x = { .type = ISQ_UI_SIZE_TYPE_PERCENT, .value = 1 },
		.y = { .type = ISQ_UI_SIZE_TYPE_PERCENT, .value = 1 },
	});
	isq_ui_background_color(state.id, 0, 0, 0, 0);
	isq_ui_border(state.id, 1, 1, 1, 0.15, 1);
	isq_ui_padding(state.id, 0, 0, 0, 0);

	return state;
}

void isq_ui_get_size(unsigned id, float *width, float *height)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return;

	*width = box->computed_rect.z - box->computed_rect.x;
	*height = box->computed_rect.w - box->computed_rect.y;
}

void isq_ui_get_position(unsigned id, float *x, float *y)
{
	struct isq_ui_box *box = isq_ui_box_array_get(id);
	if (!box)
		return;

	*x = box->computed_rect.x;
	*y = box->computed_rect.y;
}

#endif

#if 0
static void isq_ui_render(void)
{
	for (unsigned i = 0; i < isq_ui_box_array_count; ++i) {
		struct isq_ui_box *box = isq_ui_box_array_get(i);

		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BACKGROUND)
			isq_ui_enqueue_rect(box->computed_rect, isq_ui_default_uvs, box->style.background_color, 0);

		if (box->flags & ISQ_UI_BOX_FLAG_DRAW_BORDER)
			isq_ui_enqueue_border(box->computed_rect, box->style.border_color, box->style.border_width);

		// Only draw text if it exsits. 
		if (box->text) {
			const char *text = box->text;

			isq_vec2 pos = {box->computed_rect.x + box->style.padding.left, box->computed_rect.y + box->style.padding.top};
			float top = pos.y + box->style.font.size;
			float bot = pos.y;

			unsigned line_count = text ? 1 : 0;
			float longest_line_width = ISQ_UI_MAGIC_NUMBERF;
			float width_start = ISQ_UI_MAGIC_NUMBERF;

			ISQ_UI_BAKED_QUAD_TYPE q;

			while (text && *text) {
				if (*text < 32) {
					if (*text == '\n') {
						++line_count;
						pos.x = box->computed_rect.x + box->style.padding.left;
						pos.y += box->style.font.size;

						if (q.x1 - width_start > longest_line_width || longest_line_width == ISQ_UI_MAGIC_NUMBERF) {
							longest_line_width = q.x1 - width_start;
						}
						width_start = ISQ_UI_MAGIC_NUMBERF;
					}
					++text;
					continue;
				}

				ISQ_UI_BAKED_QUAD(box->style.font.character_data, 512, 512, *text-32, &pos.x, &pos.y, &q, 1);

				// TODO: Actual word wrapping
				if (q.x1 > box->computed_rect.z && box->flags & ISQ_UI_BOX_FLAG_TEXT_WRAP_WORD) {
					pos.x = box->computed_rect.x;
					pos.y += box->style.font.size;
					++line_count;
					ISQ_UI_BAKED_QUAD(box->style.font.character_data, 512, 512, *text-32, &pos.x, &pos.y, &q, 1);
				}

				if (width_start == ISQ_UI_MAGIC_NUMBERF) {
					width_start = q.x0;
				}

				++text;

				isq_vec4 text_rect = (isq_vec4){q.x0, q.y0 + box->style.font.size * 0.75, q.x1, q.y1 + box->style.font.size * 0.75};
				isq_vec4 text_uvs = (isq_vec4){q.s0, q.t0, q.s1, q.t1};
				isq_ui_enqueue_rect(text_rect, text_uvs, box->style.text_color, box->style.font.texture_index);
			}

			if (longest_line_width == ISQ_UI_MAGIC_NUMBERF)
				longest_line_width = q.x1 - width_start;

			box->text_width_in_pixels = longest_line_width;
			box->text_line_count = line_count;
		}
	}

	ISQ_UI_RENDER_RECT(isq_ui_vertex_buffer, isq_ui_vertex_buffer_count);
}
#endif
