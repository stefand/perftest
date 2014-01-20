#include <stdio.h>

#if defined(WIN32)
    #include <windows.h>
    #include <gl/glew.h>
    #include <gl/glut.h>
    static DWORD prev_time;
#else
    #include <sys/time.h>
    static struct timeval prev_time;
    #ifdef __APPLE__
        #define  GLUT_RGB                           0x0000
        #define  GLUT_DOUBLE                        0x0002
        #define  GLUT_ACCUM                         0x0004
        #define  GLUT_DEPTH                         0x0010
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
        #include <GL/glut.h>
    #endif
    #define min(a,b) ((a)<(b)?(a):(b))
#endif

unsigned long time_limit = 0;

static int init(void)
{
#ifdef WIN32
    glewInit();
#endif
    glEnable(GL_SCISSOR_TEST);
    return 0;
}

#ifdef WIN32
static unsigned long start_time;

static unsigned long get_runtime()
{
    if (!start_time)
    {
        start_time = GetTickCount();
        return 0;
    }

    return GetTickCount() - start_time;
}

static void print_fps()
{
    static unsigned long frames;
    DWORD time = GetTickCount();

    frames++;
    /* every 1.5 seconds */
    if (time - prev_time > 1500)
    {
        printf("approx %.2ffps\n", 1000.0 * frames / (time - prev_time));
        prev_time = time;
        frames = 0;
    }
}
#else
static unsigned int start_time_set;
static struct timeval start;

static unsigned long get_runtime()
{
    struct timeval now;
    if (!start_time_set)
    {
        gettimeofday(&start, NULL);
        start_time_set = 1;
        return 0;
    }

    gettimeofday(&now, NULL);

    return (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
}

static void print_fps()
{
    static unsigned long frames;
    struct timeval now;
    unsigned long diff;

    gettimeofday(&now, NULL);
    diff = (now.tv_sec - prev_time.tv_sec) * 1000000 + now.tv_usec - prev_time.tv_usec;

    frames++;
    /* every 1.5 seconds */
    if (diff > 1500000)
    {
        printf("approx %.2ffps\n", 1000000.0 * frames / diff);
        prev_time = now;
        frames = 0;
    }
}
#endif

const unsigned int width = 640, height=480;

void display(void)
{
    unsigned int x, y;
    const unsigned int step = 6;
    static unsigned int frames;

    frames++;

    for(y = 0; y < height; y+=step)
    {
        float g = ((float) y) / (height - 1);
        for(x = 0; x < width; x+=step)
        {
             float r = ((float) x) / (width - 1);

             glClearColor(r, g, 0.0, 0.0);
             glScissor(x, y, min(step, width - x), min(step, height - y));
             glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    glutSwapBuffers();

    if (time_limit)
    {
        if (get_runtime() > time_limit)
        {
            printf("frames per second: %f\n", 1000.0 * ((float)frames) / ((float)time_limit));
            exit(0);
        }
    }
    else
    {
        print_fps();
    }

    if (glGetError() != GL_NO_ERROR)
    {
        printf("Draw failed\n");
    }
}

int main(int argc, char** argv)
{
    if (argc > 1) time_limit = atol(argv[1]);
    if (time_limit)
    {
        printf("Running for %lu seconds\n", time_limit);
        time_limit *= 1000;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (width, height);
    glutCreateWindow (argv[0]);
    if(init() != 0)
    {
       printf("Init error\n");
       return 1;
    }
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}
