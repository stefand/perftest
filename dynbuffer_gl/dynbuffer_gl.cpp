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
#endif

static GLuint vbo;
unsigned int bufsize;
static GLuint prog;

static const struct cube
{
    float x, y, z;
    float x1, y1, z1;
    unsigned int color;
}
cube[] =
{
    /* Front side */
    {   -0.5,   -0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,   -0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {   -0.5,    0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,    0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000  },

    /* Back side */
    {   -0.5,   -0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,   -0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {   -0.5,    0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,    0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000  },

    /* left side */
    {   -0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {   -0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {   -0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {   -0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000  },

    /* right side */
    {    0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000  },

    /* Bottom */
    {   -0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {   -0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000  },
    {    0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000  },

    /* Top */
    {   -0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
};

static const unsigned int num_cubes = 1000;
const unsigned int vertices_per_cube = sizeof(cube) / sizeof(*cube);

static const char *vshader =
    "attribute vec4 attrib0;\n"
    "attribute vec4 attrib1;\n"
    "attribute vec4 attrib2;\n"
    "varying vec4 color;\n"
    "void main()\n"
    "{\n"
    "    vec4 pos = attrib0;\n"
    "    pos.xyz = pos.xyz - vec3(10.0) + attrib1.xyz * vec3(2.0);\n"
    "    pos *= vec4(0.05, 0.05, 0.05, 1.0);\n"
    "    pos *= gl_ModelViewProjectionMatrix;\n"
    "    gl_Position = pos;\n"
    "    color = attrib2;\n"
    "}\n";

static const char *fragshader =
    "varying vec4 color;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = color;\n"
    "}\n";

static void write_instance(struct cube *instance)
{
    static unsigned int i;
    unsigned int face;
    static unsigned int x, y, z;
    static unsigned int color;

    i++;
    if (i == num_cubes)
    {
        i = 0;
        x = 0;
        y = 0;
        z = 0;
        color = 0;
    }

    for(face = 0; face < vertices_per_cube; face++)
    {
        instance[face].x = cube[face].x;
        instance[face].y = cube[face].y;
        instance[face].z = cube[face].z;

        instance[face].x1 = (float) x;
        instance[face].y1 = (float) y;
        instance[face].z1 = (float) z;
        switch(color)
        {
            case 9:
                color = 0;
                /* Drop through */
            case 0:
                instance[face].color = 0x00ff0000;
                break;
            case 1:
                instance[face].color = 0x0000ff00;
                break;
            case 2:
                instance[face].color = 0x000000ff;
                break;
            case 3:
                instance[face].color = 0x00ffff00;
                break;
            case 4:
                instance[face].color = 0x00ff00ff;
                break;
            case 5:
                instance[face].color = 0x0000ffff;
                break;
            case 6:
                instance[face].color = 0x00ffffff;
                break;
            case 7:
                instance[face].color = 0x0080ff00;
                break;
            case 8:
                instance[face].color = 0x00ff8000;
                break;
        }
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

static int init(void)
{
    GLuint frag, vert;
    GLsizei len;
    char log[4096];
    bufsize = num_cubes * vertices_per_cube * sizeof(struct cube) / 4;

#ifdef WIN32
    glewInit();
#endif

    glGenBuffersARB(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bufsize, NULL, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct cube), (void *) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct cube), (void *) 12);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct cube), (void *) 24);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

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
    return 0;
}

#ifdef WIN32
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

void display(void)
{
    unsigned int i, offset = 0;;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    struct cube *cube_data;

    for(i = 0; i < num_cubes; i++)
    {
        GLbitfield access = GL_MAP_WRITE_BIT;

        if (offset + sizeof(cube) > bufsize) offset = 0;
        if (offset) access |= GL_MAP_UNSYNCHRONIZED_BIT;
        else access |= GL_MAP_INVALIDATE_BUFFER_BIT;

        cube_data = (struct cube *)glMapBufferRange(GL_ARRAY_BUFFER, offset, sizeof(cube), access);
        write_instance(cube_data);
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glDrawArrays(GL_TRIANGLE_STRIP, offset / sizeof(cube) * vertices_per_cube +  0, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, offset / sizeof(cube) * vertices_per_cube +  4, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, offset / sizeof(cube) * vertices_per_cube +  8, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, offset / sizeof(cube) * vertices_per_cube + 12, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, offset / sizeof(cube) * vertices_per_cube + 16, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, offset / sizeof(cube) * vertices_per_cube + 20, 4);

        offset += sizeof(cube);
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
       return 1;
    }
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}
