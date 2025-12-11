#include "graph.h"
#include "barnes_hut.h"
#include "screen.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <omp.h>

#define WIDTH 1500
#define HEIGHT 900

uint32_t hash_color(uint32_t id)
{
    uint32_t h = id * 2654435761u; // Knuth's multiplicative hash
    uint8_t r = (h >> 16) & 0xFF;
    uint8_t g = (h >> 8) & 0xFF;
    uint8_t b = h & 0xFF;
    return (r << 16) | (g << 8) | b;
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow(argv[1],
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB888,
                                         SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    int running = 1;
    SDL_Event e;

    FILE *f = fopen(argv[1], "r");
    graph *g = graph_parse(f);
    fclose(f);

    for (int i = 0; i < g->n; i++)
        g->W[i] = 1;

    printf("%lld %lld\n", g->n, g->m);

    screen *s = screen_init(HEIGHT, WIDTH);
    barnes_hut *bh = barnes_hut_init(g);

    uint32_t *Colors = malloc(sizeof(uint32_t) * g->n);

    for (int u = 0; u < g->n; u++)
    {
        Colors[u] = 0x2c2c2c;
    }

    int mbd = 0, selected = -1;
    int draw_edges = 0;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == 1)
            {
                screen_mouse_down(s, e.motion.x, e.motion.y);
                mbd = 1;

                float x = -s->root_x + ((float)e.motion.x / s->zoom);
                float y = -s->root_y + ((float)e.motion.y / s->zoom);

                float min_d = 1e6;
                int v = -1;

                for (int u = 0; u < g->n; u++)
                {
                    if (!g->A[u])
                        continue;
                    float dx = bh->X[u] - x, dy = bh->Y[u] - y;
                    float d = sqrtf(dx * dx + dy * dy) * s->zoom;

                    if (d < min_d)
                    {
                        min_d = d;
                        v = u;
                    }
                }

                if (v >= 0)
                {
                    float rx = sqrtf((float)g->W[v] / M_PI);
                    if (min_d <= rx * s->zoom + 2)
                    {
                        selected = v;
                        bh->Tabu[v] = 1;
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == 1)
            {
                screen_mouse_up(s);
                mbd = 0;

                if (selected >= 0)
                {
                    bh->Tabu[selected] = 0;
                    selected = -1;
                }
            }
            else if (e.type == SDL_MOUSEMOTION && s->drag > 0)
            {
                screen_mouse_move(s, e.motion.x, e.motion.y);
                bh->rest_l = s->sliders[0];
                bh->k_spring = (s->sliders[1] * 4.0f) / 200.0f;
                bh->k_repel = (s->sliders[2] * 4.0f) / 200.0f;
                bh->k_gravity = (s->sliders[3] * 4.0f) / 200.0f;
                bh->theta = (s->sliders[4] * 4.0f) / 200.0f;
            }
            else if (e.type == SDL_MOUSEMOTION && mbd)
            {
                if (selected >= 0)
                {
                    bh->X[selected] += (float)e.motion.xrel / s->zoom;
                    bh->Y[selected] += (float)e.motion.yrel / s->zoom;
                }
                else
                {
                    s->root_x += (float)e.motion.xrel / s->zoom;
                    s->root_y += (float)e.motion.yrel / s->zoom;
                }
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                if (e.wheel.y > 0 && s->zoom < 10.0f)
                    s->zoom *= 1.1;
                else if (e.wheel.y < 0 && s->zoom > 0.01f)
                    s->zoom *= 0.9;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e)
            {
                draw_edges = !draw_edges;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q)
            {
                running = 0;
            }
            else if (e.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        double t0 = omp_get_wtime();
        barnes_hut_step(bh, g);
        double t1 = omp_get_wtime();
        screen_render_frame(s, g, Colors, bh->X, bh->Y, draw_edges);
        double t2 = omp_get_wtime();

        printf("\r%5.3lf %5.3lf", t1 - t0, t2 - t1);
        fflush(stdout);

        SDL_UpdateTexture(tex, NULL, s->Pixels, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    printf("\n");

    graph_free(g);
    barnes_hut_free(bh);
    screen_free(s);

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}