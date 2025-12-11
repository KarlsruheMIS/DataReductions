#include "screen.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

screen *screen_init(int height, int width)
{
    screen *s = malloc(sizeof(screen));

    s->height = height;
    s->width = width;

    s->Pixels = malloc(sizeof(uint32_t) * height * width);

    for (int i = 0; i < height * width; i++)
        s->Pixels[i] = 0;

    s->zoom = 0.5f;
    s->root_x = 0;
    s->root_y = 0;

    for (int i = 0; i < SLIDERS; i++)
        s->sliders[i] = 50;
    s->drag = 0;

    return s;
}

void screen_free(screen *s)
{
    free(s->Pixels);

    free(s);
}

void screen_draw_line(screen *s, int x0, int y0, int x1, int y1, uint32_t color)
{
    if (x0 < 0 || x0 >= s->width || y0 < 0 || y0 >= s->height ||
        x1 < 0 || x1 >= s->width || y1 < 0 || y1 >= s->height)
        return;

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1)
    {
        s->Pixels[y0 * s->width + x0] = color;

        if (x0 == x1 && y0 == y1)
            break;

        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void screen_draw_circle_filled(screen *s, int xm, int ym, int r, uint32_t draw_color, uint32_t fill_color)
{
    if (xm - r < 0 || xm + r >= s->width || ym - r < 0 || ym + r >= s->height)
        return;

    for (int x = xm - r; x < xm + r; x++)
    {
        for (int y = ym - r; y < ym + r; y++)
        {
            int dx = x - xm, dy = y - ym;
            float d = sqrtf(dx * dx + dy * dy);
            if (d <= (float)r - 1.0f)
            {
                s->Pixels[(y * s->width) + x] = fill_color;
            }
            else if (d <= (float)r)
            {
                s->Pixels[(y * s->width) + x] = draw_color;
            }
        }
    }
}

void screen_render_frame(screen *s, graph *g, uint32_t *Colors, float *X, float *Y, int draw_edges)
{
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < s->height * s->width; i++)
        {
            s->Pixels[i] = 0xffffff;
        }

#pragma omp for
        for (int u = 0; u < g->n; u++)
        {
            if (!g->A[u])
                continue;

            int ux = (X[u] + s->root_x) * s->zoom, uy = (Y[u] + s->root_y) * s->zoom;

            if (!draw_edges)
                continue;

            for (int i = 0; i < g->D[u]; i++)
            {
                int v = g->V[u][i];

                int vx = (X[v] + s->root_x) * s->zoom, vy = (Y[v] + s->root_y) * s->zoom;

                if (u < v)
                    screen_draw_line(s, ux, uy, vx, vy, 0x00);
            }
        }

        for (int u = 0; u < g->n; u++)
        {
            if (!g->A[u])
                continue;

            int ux = (X[u] + s->root_x) * s->zoom, uy = (Y[u] + s->root_y) * s->zoom;
            int ru = 1; // sqrt((double)g->W[u] / M_PI);
            screen_draw_circle_filled(s, ux, uy, (s->zoom * ru) + 2, Colors[u], Colors[u]);
        }

        for (int i = 0; i < SLIDERS; i++)
        {
            screen_draw_line(s, 25, 25 * (i + 1), 125, 25 * (i + 1), 0x00);
            screen_draw_circle_filled(s, 25 + s->sliders[i], 25 * (i + 1), 5, 0x00, 0xffffff);
        }
    }
}

void screen_mouse_down(screen *s, int x, int y)
{
    for (int i = 0; i < SLIDERS; i++)
    {
        float dx = x - (25 + s->sliders[i]), dy = y - (25 * (i + 1));
        float d = sqrtf(dx * dx + dy * dy);

        if (d <= 5.0f)
        {
            s->drag = i + 1;
        }
    }
}

void screen_mouse_up(screen *s)
{
    s->drag = 0;
}

void screen_mouse_move(screen *s, int x, int y)
{
    if (s->drag == 0)
        return;

    int p = s->drag - 1;

    s->sliders[p] = x - 25;
    s->sliders[p] = s->sliders[p] < 0 ? 0 : s->sliders[p];
    s->sliders[p] = s->sliders[p] > 100 ? 100 : s->sliders[p];
}