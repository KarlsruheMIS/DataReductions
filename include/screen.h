#pragma once

#include <graph.h>
#include <stdint.h>

#define SLIDERS 5

typedef struct
{
    int height, width;
    uint32_t *Pixels;

    float zoom;
    float root_x, root_y;

    int sliders[SLIDERS]; // 0 to 100
    int drag;
} screen;

screen *screen_init(int height, int width);

void screen_free(screen *s);

void screen_draw_line(screen *s, int x0, int y0, int x1, int y1, uint32_t color);

void screen_draw_circle(screen *s, int xm, int ym, int r, uint32_t color);

void screen_draw_circle_filled(screen *s, int xm, int ym, int r, uint32_t draw_color, uint32_t fill_color);

void screen_render_frame(screen *s, graph *g, uint32_t *Colors, float *X, float *Y, int draw_edges);

void screen_mouse_down(screen *s, int x, int y);

void screen_mouse_up(screen *s);

void screen_mouse_move(screen *s, int x, int y);
