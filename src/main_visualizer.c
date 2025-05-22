#include <GL/glut.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#include "graph.h"
#include "visualizer.h"
#include "reducer.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "triangle.h"
#include "v_shape.h"
#include "neighborhood_removal.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "twin.h"
#include "domination.h"
#include "critical_set.h"
#include "unconfined.h"
#include "extended_domination.h"
#include "weighted_funnel.h"

#define STEP_TIME 0.03
#define NODE_SIZE 1

const unsigned int W = 1920;
const unsigned int H = 1080;

unsigned int threads = 16;

double zoom = 0.5;
double cx = OUTER_DIM / 2, cy = OUTER_DIM / 2;
int node_size = NODE_SIZE;

int selected_node = -1;
int mx, my;

graph *g;
force_layout *f;
int *independent_set, *active;

int *color;
int show_coloring = 0, show_weights = 0, show_solution = 0;
int run_simulation = 1, do_reductions = 0, alt_reductions = 0, pause = 0;
int center_node = -1;
int zoom_out = 0;
long long max_weight = 0, min_weight = INT64_MAX;

int random_idle = 0;
int zoom_dir = 0;

reducer *red;
reduction_log *r_log;

int rule = 0, node = 0;
int remaining = 0;

int colors[10][3] = {{255, 0, 0},
                     {0, 255, 0},
                     {0, 0, 255},
                     {255, 255, 0},
                     {128, 128, 128},
                     {86, 180, 233},
                     {230, 159, 0},
                     {128, 0, 0},
                     {255, 165, 0},
                     {128, 0, 128}};

unsigned char *image;

double time_ref;

double compute_time = 0.0, steps = 0.0;

static inline void set_pixel(int x, int y, int r, int g, int b)
{
    int p = (y * W + x) * 3;
    image[p] = r;
    image[p + 1] = g;
    image[p + 2] = b;
}

static inline void draw_node(int x, int y, int r, int g, int b)
{
    for (int i = x - node_size; i < x + node_size; i++)
    {
        for (int j = y - node_size; j < y + node_size; j++)
        {
            double d = sqrt((i - x) * (i - x) + (j - y) * (j - y));
            if (d <= node_size - 1 && i >= 0 && i < W && j >= 0 && j < H)
                set_pixel(i, j, r, g, b);
            else if (d <= node_size && i >= 0 && i < W && j >= 0 && j < H)
                set_pixel(i, j, 0, 0, 0);
        }
    }
}

static inline void draw_edge(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0, dy = y1 - y0;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    double ix = dx / (double)steps, iy = dy / (double)steps;
    double x = x0, y = y0;
    for (int i = 0; i < steps; i++)
    {
        x += ix;
        y += iy;
        if (x <= 0.0 || x >= W || y <= 0.0 || y >= H)
            break;
        set_pixel(x, y, 200, 200, 200);
    }
}

static inline void translate_point(float *x, float *y)
{
    *x -= cx;
    *y -= cy;
    *x *= zoom;
    *y *= zoom;
    *x += W / 2;
    *y += H / 2;
}

static inline void translate_point_reverse(float *x, float *y)
{
    *x -= W / 2;
    *y = H - *y;
    *y -= H / 2;
    *x /= zoom;
    *y /= zoom;
    *x += cx;
    *y += cy;
}

void display()
{
    omp_set_num_threads(threads);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

#pragma omp parallel for schedule(static, 256)
    for (int i = 0; i < W * H * 3; i++)
        image[i] = (unsigned char)255;

#pragma omp parallel for schedule(static, 256)
    for (int u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;
        for (int i = 0; i < g->D[u]; i++)
        {
            int v = g->V[u][i];
            if (!g->A[v])
                continue;

            float x0 = f->V[u].x, y0 = f->V[u].y;
            float x1 = f->V[v].x, y1 = f->V[v].y;
            translate_point(&x0, &y0);
            translate_point(&x1, &y1);
            draw_edge(roundf(x0), roundf(y0), roundf(x1), roundf(y1));
        }
    }

    // TODO, make par optional
#pragma omp parallel for schedule(static, 256)
    for (int i = 0; i < g->n; i++)
    {
        if (!g->A[i])
            continue;
        float x = f->V[i].x, y = f->V[i].y;
        translate_point(&x, &y);
        if (x < W && x >= 0.0 && y < H && y >= 0.0)
        {
            if (show_weights)
            {
                double f = (double)(g->W[i] - min_weight) / (double)(max_weight - min_weight);
                int c = f * 255;
                draw_node(roundf(x), roundf(y), 255 - c, 255 - c, c);
            }
            else if (show_solution)
            {
                if (independent_set[i] == 0)
                    draw_node(roundf(x), roundf(y), 100, 100, 100);
                else if (independent_set[i] == 1)
                    draw_node(roundf(x), roundf(y), 0, 255, 0);
                else if (independent_set[i] == -1)
                    draw_node(roundf(x), roundf(y), 255, 0, 0);
            }
            else
            {
                draw_node(roundf(x), roundf(y), 100, 100, 100);
            }
        }
    }

    glRasterPos2d(-1, -1);
    glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, image);

    glColor4f(1.0, 1.0, 1.0, 0.8);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(-1.0f, 0.6f);
    glVertex2f(1.0f, 0.6f);
    glVertex2f(1.0f, 1.0f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.75f);
    glVertex2f(-1.0f, -0.9f);
    glVertex2f(1.0f, -0.9f);
    glVertex2f(1.0f, -0.75f);
    glEnd();

    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.5);
    glScalef(0.0006, 0.0006, 0.0006);
    glTranslatef(-1500, 1400, 0);

    static char *title = "Data Reductions in Combinatorial Optimization\0";
    for (char *p = title; *p != '\0'; p++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

    glTranslatef(-2300, -300, 0);
    static char *title2 = "for Independence Problems\0";
    for (char *p = title2; *p != '\0'; p++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

    glTranslatef(-1500, -2500, 0);
    static char *title3 = "Ernestine Grossmann\0";
    for (char *p = title3; *p != '\0'; p++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glPopMatrix();

    glutSwapBuffers();
}

long long it = 0;

void idle()
{
    omp_set_num_threads(threads);

    if (random_idle)
    {
        if (center_node >= 0)
        {
            float _x = 0.0, _y = 0.0;
            if (do_reductions)
            {
                _x = f->V[center_node].x;
                _y = f->V[center_node].y;
            }
            else
            {
                _x = OUTER_DIM / 2;
                _y = OUTER_DIM / 2;
            }
            float fx = _x - cx, fy = _y - cy;
            float d = sqrtf(fx * fx + fy * fy);
            if (d < 1.0)
            {
                cx = _x;
                cy = _y;
            }
            else
            {
                cx += (fx / d) * 0.2;
                cy += (fy / d) * 0.2;
            }
        }

        if (!do_reductions)
        {
            zoom /= 1.001f;
            if (zoom < 0.5)
                zoom = 0.5;
            node_size = (sqrt(zoom) * 1.5) + NODE_SIZE;
        }
        else
        {
            zoom *= 1.001f;
            if (zoom > 3.0)
                zoom = 3.0;
            node_size = (sqrt(zoom) * 1.5) + NODE_SIZE;
        }

        if ((it % (1 << 8)) == 0)
        {
            if (zoom <= 0.5 && g->nr == g->n)
            {
                do_reductions = 1;
            }

            if (zoom <= 0.5 && g->nr != g->n)
            {
                reducer_restore_graph(g, r_log, 0);
                reducer_queue_all(red, g);
            }
        }

        if ((it % (1 << 10)) == 0 || !g->A[center_node])
        {

            it = 0;
            if (g->nr > 0)
            {
                center_node = rand() % g->n;
                while (!g->A[center_node])
                    center_node = rand() % g->n;
            }
            // printf("Changing center to %d\n", center_node);
        }
        it++;
    }

    if (do_reductions)
    {
        node_id old_n = g->n, old_nr = g->nr, old_m = g->m;
        for (int i = 0; i < 10; i++)
            reducer_reduce_step(red, g, r_log);
        if (g->n > old_n)
        {
            for (node_id u = old_n; u < g->n; u++)
            {
                if (!g->A[u])
                    continue;

                float x = 0.0f, y = 0.0f;
                for (node_id i = 0; i < g->D[u]; i++)
                {
                    node_id v = g->V[u][i];
                    x += f->V[v].x;
                    y += f->V[v].y;
                }
                f->V[u].x = x / (float)g->D[u];
                f->V[u].y = y / (float)g->D[u];
            }
        }

        if (old_m == g->m && old_nr == g->nr)
        {
            do_reductions = 0;
        }
    }

    if (run_simulation)
        force_layout_step(f, g);

    // If nothing changed, don't update
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (button == 3)
    {
        zoom *= 1.1;
        if (zoom > 1000.0)
            zoom = 1000.0;
        node_size = (sqrt(zoom) * 2) + NODE_SIZE;
    }
    else if (button == 4)
    {
        zoom /= 1.1;
        if (zoom < 0.5)
            zoom = 0.5;
        node_size = (sqrt(zoom) * 2) + NODE_SIZE;
    }
    else if (button == 0)
    {
        if (state == 0)
        {
            mx = x, my = y;
            float _x = x, _y = y;
            translate_point_reverse(&_x, &_y);

#pragma omp parallel for
            for (int u = 0; u < g->n; u++)
            {
                if (!g->A[u])
                    continue;
                float fx = f->V[u].x - _x, fy = f->V[u].y - _y;
                float d = sqrtf(fx * fx + fy * fy);
                if (d < 0.5 + 2.0 / zoom)
                    selected_node = u;
            }
            if (selected_node >= 0)
                f->V[selected_node].locked = 1;

            // printf("%d\n", selected_node);
        }
        else
        {
            if (selected_node >= 0)
                f->V[selected_node].locked = 0;
            selected_node = -1;
        }
    }

    // printf("%d %d %d %d %lf\n", button, state, x, y, zoom);
}

void mouse_move(int _x, int _y)
{
    if (selected_node >= 0)
    {
        f->V[selected_node].x += (double)(_x - mx) / zoom;
        f->V[selected_node].y += (double)(my - _y) / zoom;

        mx = _x;
        my = _y;
    }
    else
    {
        center_node = -1;

        cx += (double)(mx - _x) / zoom;
        cy += (double)(_y - my) / zoom;

        mx = _x;
        my = _y;
    }
}

void keypress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
        show_weights = !show_weights;
        break;
    case 'q':
        exit(0);
        break;
    case 'r':
        do_reductions = 1;
        random_idle = 1;
        break;
    case 'u':
        do_reductions = 0;
        reducer_restore_graph(g, r_log, 0);
        reducer_queue_all(red, g);
        break;

    default:
        break;
    }
}

int main(int argc, char **argv)
{
    omp_set_num_threads(threads);

    // drawing_threads = atoi(argv[1]);

    node_size = (sqrt(zoom) * 2) + NODE_SIZE;

    FILE *file = fopen(argv[1], "r");
    g = graph_parse(file);
    fclose(file);

    red = reducer_init(g, 11,
                       degree_zero,
                       degree_one,
                       neighborhood_removal,
                       triangle,
                       v_shape,
                       twin,
                       domination,
                       extended_domination,
                       simplicial_vertex_with_weight_transfer,
                       weighted_funnel,
                       unconfined);
    r_log = reducer_init_reduction_log(g);

    f = force_layout_init(g);

    independent_set = malloc(sizeof(int) * g->n);
    active = malloc(sizeof(int) * g->n);

    for (int i = 0; i < g->n; i++)
        independent_set[i] = 0;

    for (int i = 0; i < g->n; i++)
        active[i] = 1;

    for (int i = 0; i < g->n; i++)
    {
        if (g->W[i] > max_weight)
            max_weight = g->W[i];
        if (g->W[i] < min_weight)
            min_weight = g->W[i];
    }
    remaining = g->n;

    printf("%lld %lld\n", g->n, g->m);

    image = malloc(sizeof(unsigned char) * H * W * 3);

    time_ref = omp_get_wtime();

    for (int i = 0; i < 2000; i++)
    {
        force_layout_step(f, g);
    }

    threads = 4;

    printf("%.5lf\n", omp_get_wtime() - time_ref);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(W, H);
    glutCreateWindow("Graph");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_move);
    glutKeyboardFunc(keypress);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutFullScreen();
    glutMainLoop();

    graph_free(g);
    force_layout_free(f);
    free(image);

    return 0;
}