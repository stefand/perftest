#include <stdio.h>
#include <windows.h>

#include <gl/glew.h>
#include <gl/glut.h>

static DWORD prev_time;
static GLuint vbo;
static GLuint prog;

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
    {    0.5,    0.5,   -0.5 },

    /* Back side */
    {   -0.5,   -0.5,    0.5 },
    {    0.5,   -0.5,    0.5 },
    {   -0.5,    0.5,    0.5 },
    {    0.5,    0.5,    0.5 },

    /* left side */
    {   -0.5,   -0.5,    -0.5 },
    {   -0.5,    0.5,    -0.5 },
    {   -0.5,   -0.5,     0.5 },
    {   -0.5,    0.5,     0.5 },

    /* right side */
    {    0.5,   -0.5,    -0.5 },
    {    0.5,    0.5,    -0.5 },
    {    0.5,   -0.5,     0.5 },
    {    0.5,    0.5,     0.5 },

    /* Bottom */
    {   -0.5,   -0.5,    -0.5 },
    {    0.5,   -0.5,    -0.5 },
    {   -0.5,   -0.5,     0.5 },
    {    0.5,   -0.5,     0.5 },

    /* Top */
    {   -0.5,    0.5,    -0.5 },
    {    0.5,    0.5,    -0.5 },
    {   -0.5,    0.5,     0.5 },
    {    0.5,    0.5,     0.5 },
};

struct instancedata
{
    float pos[3];
    unsigned int color;
};

static struct instancedata instances[1000];
static const unsigned int num_cubes = 1000;

static const char *vshader =
    "attribute vec4 attrib0;\n"
    "attribute vec4 attrib1;\n"
    "attribute vec4 attrib2;\n"
    "varying vec4 color;\n"
    "void main()\n"
    "{\n"
    "    vec4 pos = attrib0;\n"
    "    pos.xyz = pos.xyz - vec3(10.0) + attrib2.xyz * vec3(2.0);\n"
    "    pos *= vec4(0.05, 0.05, 0.05, 1.0);\n"
    "    pos *= gl_ModelViewProjectionMatrix;\n"
    "    gl_Position = pos;\n"
    "    color = attrib1;\n"
    "}\n";

static const char *fragshader =
    "varying vec4 color;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = color;\n"
    "}\n";

static void init_instances()
{
    unsigned int i;
    unsigned int x = 0, y = 0, z = 0;
    unsigned int color = 0;

    for(i = 0; i < (sizeof(instances) / sizeof(*instances)); i++)
    {
        instances[i].pos[0] = (float) x;
        instances[i].pos[1] = (float) y;
        instances[i].pos[2] = (float) z;
        switch(color)
        {
            case 9:
                color = 0;
                /* Drop through */
            case 0:
                instances[i].color = 0x00ff0000;
                break;
            case 1:
                instances[i].color = 0x0000ff00;
                break;
            case 2:
                instances[i].color = 0x000000ff;
                break;
            case 3:
                instances[i].color = 0x00ffff00;
                break;
            case 4:
                instances[i].color = 0x00ff00ff;
                break;
            case 5:
                instances[i].color = 0x0000ffff;
                break;
            case 6:
                instances[i].color = 0x00ffffff;
                break;
            case 7:
                instances[i].color = 0x0080ff00;
                break;
            case 8:
                instances[i].color = 0x00ff8000;
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
    }
}

static int init(void)
{
    GLuint frag, vert;
    GLsizei len;
    char log[4096];

    glewInit();
    init_instances();

    glGenBuffersARB(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*cube), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vshader, 0);
    glCompileShader(vert);
    glGetShaderInfoLog(vert, sizeof(log), &len, log);
    if (len > 0)
    {
        printf("Vertex shader log:\n");
        printf("%s\n", log);
    }

    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragshader, 0);
    glCompileShader(frag);
    glGetShaderInfoLog(frag, sizeof(log), &len, log);
    if (len > 0)
    {
        printf("Fragment shader log:\n");
        printf("%s\n", log);
    }

    prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);
    glBindAttribLocation(prog, 0, "attrib0");
    glBindAttribLocation(prog, 1, "attrib1");
    glBindAttribLocation(prog, 2, "attrib2");
    glGetProgramInfoLog(prog, sizeof(log), &len, log);
    if (len > 0)
    {
        printf("Program log:\n");
        printf("%s\n", log);
    }
    glUseProgram(prog);

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(45, 1, 1, 0);

    if (glGetError() != GL_NO_ERROR)
    {
        printf("Setup failed\n");
        return 1;
    }
    prev_time = GetTickCount();
    return 0;
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

void display(void)
{
    unsigned int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(i = 0; i < num_cubes; i++)
    {
        glVertexAttrib4Nubv(1, (GLubyte *) &instances[i].color);
        glVertexAttrib3fv(2, instances[i].pos);

        glDrawArrays(GL_TRIANGLE_STRIP,  0, 4);
        glDrawArrays(GL_TRIANGLE_STRIP,  4, 4);
        glDrawArrays(GL_TRIANGLE_STRIP,  8, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
    }

    glutSwapBuffers();
    print_fps();
    if (glGetError() != GL_NO_ERROR)
    {
        printf("Draw failed\n");
    }
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
       Sleep(100000);
       return 1;
    }
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}
