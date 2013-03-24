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
        #include <OpenGL/gl.h>
        #include <GLUT/glut.h>
    #else
        #include <GL/glew.h>
        #include <GL/gl.h>
        #include <GL/glut.h>
    #endif
    #define min(a,b) ((a)<(b)?(a):(b))
#endif

static const struct
{
    float x, y, z;
}
quad[] =
{
    /* Front side */
    {   -1.0,   -1.0,   0.0 },
    {   -1.0,    1.0,   0.0 },
    {    1.0,   -1.0,   0.0 },
    {    1.0,    1.0,   0.0 },

};

const char *vshader =
	"#version 120\n"
	"attribute vec4 vs_in0;\n"
	"void main()\n"
	"{\n"
	"	 gl_Position.xyzw = (vs_in0.xyzw);\n"
	"	 gl_TexCoord[0].xyzw = (vs_in0.xyzw);\n"
	"}\n";

#define USE_CONST 0
const char *fragshader =
	"#version 120\n"
#if USE_CONST
	"const vec4 ps_c0 = vec4(0.92341375, 0.34255252342, 1.51991844e-004, 1);\n"
	"const vec4 ps_c1 = vec4(0.347353422, 0.4633423421, 1.51991844e-003, 1);\n"
	"const vec4 ps_c2 = vec4(0.99609375, 0.00389099121, 1.51991844e-002, 1);\n"
	"const vec4 ps_c3 = vec4(0.53212311, 0.20389099121, 1.51991844e-001, 1);\n"
	"const vec4 ps_c4 = vec4(0.23452315, 0.00389099121, 1.452345122, 1);\n"
	"const vec4 ps_c5 = vec4(0.42345523, 0.01389099121, 2.33513123, 1);\n"
	"const vec4 ps_c6 = vec4(0.99609375, 0.20389099121, 1.51991844e-005, 1);\n"
	"const vec4 ps_c7 = vec4(0.99609375, 0.30389099121, 1.51991844e-005, 1);\n"
	"const vec4 ps_c8 = vec4(0.99609375, 0.40389099121, 1.51991844e-005, 1);\n"
	"const vec4 ps_c9 = vec4(0.25, -0.556640983, -0.0371089987, -0.654296994);\n"
	"const vec4 ps_c10 = vec4(0.173828006, 0.111327998, 0.0644529983, 255);\n"
	"const vec4 ps_c11 = vec4(0.00195299997, 0.0820309967, -0.0605470017, 0);\n"
	"const vec4 ps_c12 = vec4(0.220703006, -0.359375, -0.0625, -5);\n"
	"const vec4 ps_c13 = vec4(0.242188007, 0.126953006, -0.25, 0);\n"
	"const vec4 ps_c14 = vec4(0.0703129992, -0.0253909994, 0.148438007, 0);\n"
	"const vec4 ps_c15 = vec4(-0.078125, 0.0136719998, -0.314453006, 0);\n"
	"const vec4 ps_c16 = vec4(0.117187999, -0.140625, -0.199219003, 0);\n"
	"const vec4 ps_c17 = vec4(2, -1, 0.499999583, 0.5);\n"
	"const vec4 ps_c18 = vec4(6.28318548, -3.14159274, 1, -1);\n"
	"const vec4 ps_c19 = vec4(-1, -2, -3, -4);\n"
	"const vec4 ps_c20 = vec4(0, 1, 0.125, 0);\n"
#else
	"uniform vec4 ps_c0;\n"
	"uniform vec4 ps_c1;\n"
	"uniform vec4 ps_c2;\n"
	"uniform vec4 ps_c3;\n"
	"uniform vec4 ps_c4;\n"
	"uniform vec4 ps_c5;\n"
	"uniform vec4 ps_c6;\n"
	"uniform vec4 ps_c7;\n"
	"uniform vec4 ps_c8;\n"
	"uniform vec4 ps_c9;\n"
	"uniform vec4 ps_c10;\n"
	"uniform vec4 ps_c11;\n"
	"uniform vec4 ps_c12;\n"
	"uniform vec4 ps_c13;\n"
	"uniform vec4 ps_c14;\n"
	"uniform vec4 ps_c15;\n"
	"uniform vec4 ps_c16;\n"
	"uniform vec4 ps_c17;\n"
	"uniform vec4 ps_c18;\n"
	"uniform vec4 ps_c19;\n"
	"uniform vec4 ps_c20;\n"
#endif
	"vec4 R0;\n"
	"void main()\n"
	"{\n"
	"	 R0.x = (dot(gl_TexCoord[0].xyzw, ps_c0.xyzw));\n"
	"	 R0.y = (dot(gl_TexCoord[0].xyzw, ps_c1.xyzw));\n"
	"	 R0.z = (dot(gl_TexCoord[0].xyzw, ps_c2.xyzw));\n"
	"	 R0.w = (dot(gl_TexCoord[0].xyzw, ps_c3.xyzw));\n"
	"	 R0.x = (dot(R0.xyzw, ps_c4.xyzw));\n"
	"	 R0.y = (dot(R0.xyzw, ps_c5.xyzw));\n"
	"	 R0.z = (dot(R0.xyzw, ps_c6.xyzw));\n"
	"	 R0.w = (dot(R0.xyzw, ps_c7.xyzw));\n"
	"	 R0.x = (dot(R0.xyzw, ps_c8.xyzw));\n"
	"	 R0.y = (dot(R0.xyzw, ps_c9.xyzw));\n"
	"	 R0.z = (dot(R0.xyzw, ps_c10.xyzw));\n"
	"	 R0.w = (dot(R0.xyzw, ps_c11.xyzw));\n"
	"	 R0.x = (dot(R0.xyzw, ps_c12.xyzw));\n"
	"	 R0.y = (dot(R0.xyzw, ps_c13.xyzw));\n"
	"	 R0.z = (dot(R0.xyzw, ps_c14.xyzw));\n"
	"	 R0.w = (dot(R0.xyzw, ps_c15.xyzw));\n"
	"	 R0.x = (dot(R0.xyzw, ps_c16.xyzw));\n"
	"	 R0.y = (dot(R0.xyzw, ps_c17.xyzw));\n"
	"	 R0.z = (dot(R0.xyzw, ps_c18.xyzw));\n"
	"	 R0.w = (dot(R0.xyzw, ps_c19.xyzw));\n"
	"	 R0.xyzw = (R0.xyzw + ps_c20.xyzw);\n"
	"	 gl_FragData[0].xyzw = (R0.xyzw);\n"
	"}\n";

#if 0
	"	 R0.x = (dot(gl_TexCoord[0].xyzw, vec4(0.92341375, 0.34255252342, 1.51991844e-004, 1)));\n"
	"	 R0.y = (dot(gl_TexCoord[0].xyzw, vec4(0.347353422, 0.4633423421, 1.51991844e-003, 1)));\n"
	"	 R0.z = (dot(gl_TexCoord[0].xyzw, vec4(0.99609375, 0.00389099121, 1.51991844e-002, 1)));\n"
	"	 R0.w = (dot(gl_TexCoord[0].xyzw, vec4(0.53212311, 0.20389099121, 1.51991844e-001, 1)));\n"
	"	 R0.x = (dot(R0.xyzw, vec4(0.23452315, 0.00389099121, 1.452345122, 1)));\n"
	"	 R0.y = (dot(R0.xyzw, vec4(0.42345523, 0.01389099121, 2.33513123, 1)));\n"
	"	 R0.z = (dot(R0.xyzw, vec4(0.99609375, 0.20389099121, 1.51991844e-005, 1)));\n"
	"	 R0.w = (dot(R0.xyzw, vec4(0.99609375, 0.30389099121, 1.51991844e-005, 1)));\n"
	"	 R0.x = (dot(R0.xyzw, vec4(0.99609375, 0.40389099121, 1.51991844e-005, 1)));\n"
	"	 R0.y = (dot(R0.xyzw, vec4(0.25, -0.556640983, -0.0371089987, -0.654296994)));\n"
	"	 R0.z = (dot(R0.xyzw, vec4(0.173828006, 0.111327998, 0.0644529983, 255)));\n"
	"	 R0.w = (dot(R0.xyzw, vec4(0.00195299997, 0.0820309967, -0.0605470017, 0)));\n"
	"	 R0.x = (dot(R0.xyzw, vec4(0.220703006, -0.359375, -0.0625, -5)));\n"
	"	 R0.y = (dot(R0.xyzw, vec4(0.242188007, 0.126953006, -0.25, 0)));\n"
	"	 R0.z = (dot(R0.xyzw, vec4(0.0703129992, -0.0253909994, 0.148438007, 0)));\n"
	"	 R0.w = (dot(R0.xyzw, vec4(-0.078125, 0.0136719998, -0.314453006, 0)));\n"
	"	 R0.x = (dot(R0.xyzw, vec4(0.117187999, -0.140625, -0.199219003, 0)));\n"
	"	 R0.y = (dot(R0.xyzw, vec4(2, -1, 0.499999583, 0.5)));\n"
	"	 R0.z = (dot(R0.xyzw, vec4(6.28318548, -3.14159274, 1, -1)));\n"
	"	 R0.w = (dot(R0.xyzw, vec4(-1, -2, -3, -4)));\n"
	"	 R0.xyzw = (R0.xyzw + vec4(0, 1, 0.125, 0));\n"
	"	 gl_FragData[0].xyzw = (R0.xyzw);\n"
#endif

GLuint vbo, prog;

static int init(void)
{
    GLuint frag, vert;
    GLsizei len;
    char log[4096];
	const float constants[] =
	{
        0.92341375f, 0.34255252342f, 1.51991844e-004f, 1.0f,
	    0.347353422f, 0.4633423421f, 1.51991844e-003f, 1.0f,
	    0.99609375f, 0.00389099121f, 1.51991844e-002f, 1.0f,
        0.53212311f, 0.20389099121f, 1.51991844e-001f, 1.0f,
	    0.23452315f, 0.00389099121f, 1.452345122f, 1.0f,
	    0.42345523f, 0.01389099121f, 2.33513123f, 1.0f,
	    0.99609375f, 0.20389099121f, 1.51991844e-005f, 1.0f,
	    0.99609375f, 0.30389099121f, 1.51991844e-005f, 1.0f,
	    0.99609375f, 0.40389099121f, 1.51991844e-005f, 1.0f,
	    0.25f, -0.556640983f, -0.0371089987f, -0.654296994f,
	    0.173828006f, 0.111327998f, 0.0644529983f, 255.0f,
	    0.00195299997f, 0.0820309967f, -0.0605470017f, 0.0f,
	    0.220703006f, -0.359375f, -0.0625f, -5.0f,
	    0.242188007f, 0.126953006f, -0.25f, 0.0f,
	    0.0703129992f, -0.0253909994f, 0.148438007f, 0.0f,
	    -0.078125f, 0.0136719998f, -0.314453006f, 0.0f,
	    0.117187999f, -0.140625f, -0.199219003f, 0.0f,
	    2.0f, -1.0f, 0.499999583f, 0.5f,
	    6.28318548f, -3.14159274f, 1.0f, -1.0f,
	    -1.0f, -2.0f, -3.0f, -4.0f,
	    0.0f, 1.0f, 0.125f, 0.0f
	};

#ifndef __APPLE__
	glewInit();
#endif

    glGenBuffersARB(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*quad), (void *) 0);
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
    glBindAttribLocation(prog, 0, "vs_in0");
    glGetProgramInfoLog(prog, sizeof(log), &len, log);
    if (len > 0)
    {
        printf("Program log:\n");
        printf("%s\n", log);
    }

    glUseProgram(prog);
#if !USE_CONST
	{
        unsigned int i;
        for (i = 0; i < 21; i++)
        {
            GLuint uniform;
            char name[16];
            sprintf(name, "ps_c%u", i);
            uniform = glGetUniformLocation(prog, name);
            glUniform4fv(uniform, 1, constants + 4*i);
        }
    }
#endif

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

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
	glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glutSwapBuffers();
    print_fps();
    if (glGetError() != GL_NO_ERROR)
    {
        printf("Draw failed\n");
    }
}

int main(int argc, char** argv)
{
    const unsigned int width = 1440, height=900;

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (width, height);
    glutCreateWindow (argv[0]);
    glutFullScreen();
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
