#include <stdio.h>
#include <string.h>

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
#endif
static unsigned long frames;

static GLuint vbo;

/* A triangle list makes it easy to draw the entire cube in one draw, eliminating the drawprim
 * overhead as much as possible. The drawprim overhead is subject of a different test
 */
static const struct
{
    float x, y, z;
}
cube[] =
{
    /* Front side */
    {   -0.5,   -0.5,   -0.5 },
    {    0.5,   -0.5,   -0.5 },
    {   -0.5,    0.5,   -0.5 },
    {    0.5,   -0.5,   -0.5 },
    {   -0.5,    0.5,   -0.5 },
    {    0.5,    0.5,   -0.5 },

    /* Back side */
    {   -0.5,   -0.5,    0.5 },
    {    0.5,   -0.5,    0.5 },
    {   -0.5,    0.5,    0.5 },
    {    0.5,   -0.5,    0.5 },
    {   -0.5,    0.5,    0.5 },
    {    0.5,    0.5,    0.5 },

    /* left side */
    {   -0.5,   -0.5,    -0.5 },
    {   -0.5,    0.5,    -0.5 },
    {   -0.5,   -0.5,     0.5 },
    {   -0.5,    0.5,    -0.5 },
    {   -0.5,   -0.5,     0.5 },
    {   -0.5,    0.5,     0.5 },

    /* right side */
    {    0.5,   -0.5,    -0.5 },
    {    0.5,    0.5,    -0.5 },
    {    0.5,   -0.5,     0.5 },
    {    0.5,    0.5,    -0.5 },
    {    0.5,   -0.5,     0.5 },
    {    0.5,    0.5,     0.5 },

    /* Bottom */
    {   -0.5,   -0.5,    -0.5 },
    {    0.5,   -0.5,    -0.5 },
    {   -0.5,   -0.5,     0.5 },
    {    0.5,   -0.5,    -0.5 },
    {   -0.5,   -0.5,     0.5 },
    {    0.5,   -0.5,     0.5 },

    /* Top */
    {   -0.5,    0.5,    -0.5 },
    {    0.5,    0.5,    -0.5 },
    {   -0.5,    0.5,     0.5 },
    {    0.5,    0.5,    -0.5 },
    {   -0.5,    0.5,     0.5 },
    {    0.5,    0.5,     0.5 },
};

static GLuint prog[1000];
static const unsigned int num_cubes = 1000;

static const char *vshader =
    "attribute vec4 attrib0;\n"
    "vec3 pos2 = vec3(%f, %f, %f);\n"
    "vec4 col = vec4(%f, %f, %f, %f);\n"
    "varying vec4 color;\n"
    "void main()\n"
    "{\n"
    "    vec4 pos = attrib0;\n"
    "    pos.xyz = pos.xyz - vec3(10.0) + pos2 * vec3(2.0);\n"
    "    pos *= vec4(0.05, 0.05, 0.05, 1.0);\n"
    "    pos *= gl_ModelViewProjectionMatrix;\n"
    "    gl_Position = pos;\n"
    "    color = col;\n"
    "}\n";

static const char *fragshader =
    "varying vec4 color;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = color;\n"
    "}\n";

#ifdef WIN32
static void print_fps()
{
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
static void print_fps()
{
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

void display(void)
{
    unsigned int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(i = 0; i < num_cubes; i++)
    {
        glUseProgram(prog[i]);

        glDrawArrays(GL_TRIANGLES,  0, 36);
    }

    glutSwapBuffers();
    print_fps();
    if (glGetError() != GL_NO_ERROR)
    {
        printf("Draw failed\n");
    }
}

static void init_shaders(GLuint frag)
{
    unsigned int i;
    unsigned int x = 0, y = 0, z = 0;
    unsigned int color = 0;
    float xf, yf, zf, rf, gf, bf;
    char *code = (char *) malloc(strlen(vshader) + 128);
    GLuint vert;
    GLsizei len;
    char log[4096];

    for(i = 0; i < (sizeof(prog) / sizeof(*prog)); i++)
    {
        printf("Preparing shader %u\n", i);
        xf = (float) x;
        yf = (float) y;
        zf = (float) z;
        switch(color)
        {
            case 9:
                color = 0;
                /* Drop through */
            case 0:
                rf = 1.0;
                gf = 0.0;
                bf = 0.0;
                break;
            case 1:
                rf = 0.0;
                gf = 1.0;
                bf = 0.0;
                break;
            case 2:
                rf = 0.0;
                gf = 0.0;
                bf = 1.0;
                break;
            case 3:
                rf = 1.0;
                gf = 1.0;
                bf = 0.0;
                break;
            case 4:
                rf = 1.0;
                gf = 0.0;
                bf = 1.0;
                break;
            case 5:
                rf = 0.0;
                gf = 1.0;
                bf = 1.0;
                break;
            case 6:
                rf = 1.0;
                gf = 1.0;
                bf = 1.0;
                break;
            case 7:
                rf = 0.5;
                gf = 1.0;
                bf = 0.0;
                break;
            case 8:
                rf = 1.0;
                gf = 0.5;
                bf = 0.0;
                break;
        }
        x++;
        if(x == 10)
        {
            x = 0;
            y++;
            if(y == 10)
            {
                y = 0;
                z++;
            }
        }
        color++;

        sprintf(code, vshader, xf, yf, zf, rf, gf, bf);

        vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &code, 0);
        glCompileShader(vert);
        glGetShaderInfoLog(vert, sizeof(log), &len, log);
        if (len > 0)
        {
            printf("Vertex shader log:\n");
            printf("%s\n", log);
        }

        prog[i] = glCreateProgram();
        glAttachShader(prog[i], vert);
        glAttachShader(prog[i], frag);
        glLinkProgram(prog[i]);
        glDeleteShader(vert);
        glBindAttribLocation(prog[i], 0, "attrib0");
        glGetProgramInfoLog(prog[i], sizeof(log), &len, log);
        if (len > 0)
        {
            printf("Program log:\n");
            printf("%s\n", log);
        }
    }
    free(code);

    display();
    frames = 0;
    memset(&prev_time, 0, sizeof(prev_time));
}

static int init(void)
{
    GLuint frag;
    GLsizei len;
    char log[4096];

#ifdef WIN32
    glewInit();
#endif
    glGenBuffersARB(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*cube), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragshader, 0);
    glCompileShader(frag);
    glGetShaderInfoLog(frag, sizeof(log), &len, log);
    if (len > 0)
    {
        printf("Fragment shader log:\n");
        printf("%s\n", log);
    }

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(45, 1, 1, 0);

    init_shaders(frag);
    glDeleteShader(frag);

    if (glGetError() != GL_NO_ERROR)
    {
        printf("Setup failed\n");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (640, 480);
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
