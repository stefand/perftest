#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>
#include <ctime>

static HMODULE d3d9dll, d3dx9dll;
static unsigned long time_limit = 0;
IDirect3D9 *(WINAPI *pDirect3DCreate9)(UINT SDKVersion);
D3DXMATRIX *(WINAPI *pD3DXMatrixRotationAxis)(D3DXMATRIX *, const D3DXVECTOR3 *, FLOAT);
D3DXMATRIX *(WINAPI *pD3DXMatrixScaling)(D3DXMATRIX *, FLOAT, FLOAT, FLOAT);
D3DXMATRIX *(WINAPI *pD3DXMatrixTranslation)(D3DXMATRIX *, FLOAT, FLOAT, FLOAT);
D3DXMATRIX *(WINAPI *pD3DXMatrixMultiply)(D3DXMATRIX *, const D3DXMATRIX *, const D3DXMATRIX *);

static const struct cube
{
    float x, y, z;
    float x1, y1, z1;
    DWORD color;
}
cube[] =
{
    /* Front side */
    {   -0.5,   -0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,   -0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,    0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,   -0.5,   0.0,    0.0,    0.0,    0x00000000 },

    /* Back side */
    {   -0.5,   -0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,   -0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,    0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,    0.5,   0.0,    0.0,    0.0,    0x00000000 },

    /* left side */
    {   -0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },

    /* right side */
    {    0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },

    /* Bottom */
    {   -0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,   -0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,   -0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },

    /* Top */
    {   -0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,    -0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {   -0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
    {    0.5,    0.5,     0.5,   0.0,    0.0,    0.0,    0x00000000 },
};

static const unsigned int num_cubes = 1000;
static DWORD prev_time;
const unsigned int vertices_per_cube = sizeof(cube) / sizeof(*cube);

static IDirect3DDevice9 *device;

static const DWORD ps_code[] = {
    0xffff0200,                                                                 /* ps_2_0                   */
    0x0200001f, 0x80000000, 0x900f0000,                                         /* dcl v0                   */
    0x02000001, 0x800f0800, 0x90e40000,                                         /* mov oC0, v0              */
    0x0000ffff                                                                  /* end                      */
};

static const DWORD vs_code[] = {
    0xfffe0200,                                                                 /* vs_2_0                   */
    0x05000051, 0xa00f0004, 0xc1200000, 0x40000000, 0x3d4ccccd, 0x00000000,     /* def c4, -10, 2, 0.05, 0  */
    0x0200001f, 0x80000000, 0x900f0000,                                         /* dcl_position v0          */
    0x0200001f, 0x8000000a, 0x900f0001,                                         /* dcl_color v1             */
    0x0200001f, 0x80020000, 0x900f0002,                                         /* dcl_position2 v2         */
    0x03000002, 0x80070000, 0x90e40000, 0xa0000004,                             /* add r0.xyz, v0, c4.x     */
    0x04000004, 0x80070000, 0x90e40002, 0xa0550004, 0x80e40000,                 /* mad r0.xyz, v2, c4.y, r0 */
    0x03000005, 0x80070000, 0x80e40000, 0xa0aa0004,                             /* mul r0.xyz, r0, c4.z     */
    0x02000001, 0x80080000, 0x90ff0000,                                         /* mov r0.w, v0.w           */
    0x03000009, 0xc0010000, 0x80e40000, 0xa0e40000,                             /* dp4 oPos.x, r0, c0       */
    0x03000009, 0xc0020000, 0x80e40000, 0xa0e40001,                             /* dp4 oPos.y, r0, c1       */
    0x03000009, 0xc0040000, 0x80e40000, 0xa0e40002,                             /* dp4 oPos.z, r0, c2       */
    0x03000009, 0xc0080000, 0x80e40000, 0xa0e40003,                             /* dp4 oPos.w, r0, c3       */
    0x02000001, 0xd00f0000, 0x90e40001,                                         /* mov oD0, v1              */
    0x0000ffff                                                                  /* end                      */
};

static void init_instances(struct cube *instances)
{
    unsigned int i, face, j;
    unsigned int x = 0, y = 0, z = 0;
    unsigned int color = 0;

    for(i = 0; i < num_cubes; i++)
    {
        for(face = 0; face < vertices_per_cube; face++)
        {
            j = i * vertices_per_cube + face;
            instances[j].x = cube[face].x;
            instances[j].y = cube[face].y;
            instances[j].z = cube[face].z;

            instances[j].x1 = (float) x;
            instances[j].y1 = (float) y;
            instances[j].z1 = (float) z;
            switch(color)
            {
                case 9:
                    color = 0;
                    /* Drop through */
                case 0:
                    instances[j].color = 0x00ff0000;
                    break;
                case 1:
                    instances[j].color = 0x0000ff00;
                    break;
                case 2:
                    instances[j].color = 0x000000ff;
                    break;
                case 3:
                    instances[j].color = 0x00ffff00;
                    break;
                case 4:
                    instances[j].color = 0x00ff00ff;
                    break;
                case 5:
                    instances[j].color = 0x0000ffff;
                    break;
                case 6:
                    instances[j].color = 0x00ffffff;
                    break;
                case 7:
                    instances[j].color = 0x0080ff00;
                    break;
                case 8:
                    instances[j].color = 0x00ff8000;
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
}

static HWND create_window(void)
{
    WNDCLASS wc = {0};
    HWND ret;
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = L"cube_perftest_wc";
    RegisterClass(&wc);

    ret = CreateWindow(L"cube_perftest_wc", L"ddadda",
                        WS_CAPTION , 100, 100, 640, 480, 0, 0, 0, 0);
    ShowWindow(ret, SW_SHOW);

    return ret;
}

HWND window;

static IDirect3DDevice9 *create_device()
{
    HRESULT hr;
    IDirect3D9 *d3d9;
    IDirect3DDevice9 *dev = NULL;
    D3DPRESENT_PARAMETERS presparm;
    IDirect3DVertexBuffer9 *cubebuffer = NULL;
    IDirect3DVertexDeclaration9 *decl = NULL;
    IDirect3DVertexShader9 *vs = NULL;
    IDirect3DPixelShader9 *ps = NULL;
    D3DXMATRIX mat, scale, translate;
    D3DXVECTOR3 axis;
    IDirect3DTexture9 *dummy_texture;
    unsigned int i;

    void *data;
    static const D3DVERTEXELEMENT9 decl_elements[] = {
        {0, 0,  D3DDECLTYPE_FLOAT3,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,   0},
        {0, 12, D3DDECLTYPE_FLOAT3,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,   2},
        {0, 24, D3DDECLTYPE_D3DCOLOR,    D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,      0},
        D3DDECL_END()
    };

    /* It seems Microsoft's 64 bit d3d9.lib and d3dx9.lib are missing some exports. */
    d3d9dll = LoadLibraryA("d3d9");
    if (!d3d9dll)
    {
        printf("d3d9.dll not loaded\n");
        return NULL;
    }
    d3dx9dll = LoadLibraryA("d3dx9_43");
    if (!d3dx9dll)
    {
        printf("d3dx9_43.dll not loaded\n");
        return NULL;
    }

    /* Urk, can't I have (void *) to any pointer assignments in C++ ? */
    pDirect3DCreate9 =
        (IDirect3D9 * (WINAPI *)(UINT))
        GetProcAddress(d3d9dll, "Direct3DCreate9");
    pD3DXMatrixRotationAxis =
        (D3DXMATRIX * (WINAPI *)(D3DXMATRIX *, const D3DXVECTOR3 *, FLOAT))
        GetProcAddress(d3dx9dll, "D3DXMatrixRotationAxis");
    pD3DXMatrixScaling =
        (D3DXMATRIX * (WINAPI *)(D3DXMATRIX *, FLOAT, FLOAT, FLOAT))
        GetProcAddress(d3dx9dll, "D3DXMatrixScaling");
    pD3DXMatrixTranslation =
        (D3DXMATRIX * (WINAPI *)(D3DXMATRIX *, FLOAT, FLOAT, FLOAT))
        GetProcAddress(d3dx9dll, "D3DXMatrixTranslation");
    pD3DXMatrixMultiply =
        (D3DXMATRIX * (WINAPI *)(D3DXMATRIX *, const D3DXMATRIX *, const D3DXMATRIX *))
        GetProcAddress(d3dx9dll, "D3DXMatrixMultiply");

    window = create_window();
    d3d9 = pDirect3DCreate9(D3D_SDK_VERSION);
    if(!d3d9) return NULL;

    memset(&presparm, 0, sizeof(presparm));
    presparm.BackBufferCount = 1;
    presparm.BackBufferFormat = D3DFMT_X8R8G8B8;
    presparm.BackBufferWidth = 640;
    presparm.BackBufferHeight = 480;
    presparm.AutoDepthStencilFormat = D3DFMT_D24X8;
    presparm.EnableAutoDepthStencil = TRUE;
    presparm.SwapEffect = D3DSWAPEFFECT_DISCARD;
    presparm.Windowed = TRUE;
    presparm.hDeviceWindow = window;
    presparm.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &presparm, &dev);
    d3d9->Release();
    if(FAILED(hr)) goto err;

    hr = dev->CreateVertexBuffer(sizeof(cube) * num_cubes, 0, 0, D3DPOOL_MANAGED, &cubebuffer, NULL);
    if(FAILED(hr)) goto err;
    cubebuffer->Lock(0, 0, &data, 0);
    init_instances((struct cube *) data);
    cubebuffer->Unlock();
    dev->SetStreamSource(0, cubebuffer, 0, sizeof(*cube));

    hr = dev->CreateVertexShader(vs_code, &vs);
    if (FAILED(hr)) goto err;
    hr = dev->CreatePixelShader(ps_code, &ps);
    if (FAILED(hr)) goto err;

    hr = dev->CreateVertexDeclaration(decl_elements, &decl);
    if (FAILED(hr)) goto err;

    dev->SetVertexDeclaration(decl);
    dev->SetVertexShader(vs);
    dev->SetPixelShader(ps);

    axis.x = 1.0;
    axis.y = 1.0;
    axis.z = 0.0;
    D3DXMatrixIdentity(&mat);
    pD3DXMatrixRotationAxis(&mat, &axis, 45);
    pD3DXMatrixScaling(&scale, 1.0, 1.0, 0.5);
    pD3DXMatrixTranslation(&translate, 0.0, 0.0, 0.5);
    pD3DXMatrixMultiply(&mat, &mat, &scale);
    pD3DXMatrixMultiply(&mat, &mat, &translate);
    float m2[4][4] = {
        {   mat.m[0][0],    mat.m[1][0],    mat.m[2][0],    mat.m[3][0]     },
        {   mat.m[0][1],    mat.m[1][1],    mat.m[2][1],    mat.m[3][1]     },
        {   mat.m[2][2],    mat.m[1][2],    mat.m[2][2],    mat.m[3][2]     },
        {   mat.m[0][3],    mat.m[1][3],    mat.m[2][3],    mat.m[3][3]     },
    };
    dev->SetVertexShaderConstantF(0, *m2, 4);

    vs->Release();
    ps->Release();
    decl->Release();
    cubebuffer->Release();

    /* Assign dummy textures to catch the performance impact of preloading unused textures */
    for (i = 0; i < 8; i++)
    {
        hr = dev->CreateTexture(16, 16, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &dummy_texture, NULL);
        if (FAILED(hr)) goto err;
        hr = dev->SetTexture(i, dummy_texture);
        dummy_texture->Release();
        if (FAILED(hr)) goto err;
    }

    return dev;

err:
    printf("Init error\n");
    if (vs) vs->Release();
    if (ps) ps->Release();
    if (decl) decl->Release();
    if (cubebuffer) cubebuffer->Release();
    if (dev)
    {
        dev->SetVertexDeclaration(NULL);
        dev->SetPixelShader(NULL);
        dev->SetVertexShader(NULL);
        dev->SetStreamSource(0, NULL, 0, 0);
        dev->SetStreamSource(1, NULL, 0, 0);
        dev->Release();
    }
    return NULL;
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

void draw_loop()
{
    unsigned int i;
    MSG msg;
    HRESULT hr;
    IDirect3DSwapChain9 *swapchain;
    DWORD start;
    unsigned long frames = ~0UL;

    device->GetSwapChain(0, &swapchain);

    /* Some HW needs some time to initialize d3d, especially if fullscreen
     * mode is used. Wait a bit before starting the benchmark. */
    Sleep(5000);

    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    start = GetTickCount();
    while(!time_limit || (start + time_limit > GetTickCount()))
    {
        hr = device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x1a1a1a1a, 1.0, 0);
        if(FAILED(hr)) printf("Clear failed\n");
        while(PeekMessageW(&msg, window, 0, 0, PM_REMOVE) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        device->BeginScene();
        for(i = 0; i < 1000; i++)
        {
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * vertices_per_cube +  0, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * vertices_per_cube +  4, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * vertices_per_cube +  8, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * vertices_per_cube + 12, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * vertices_per_cube + 16, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * vertices_per_cube + 20, 2);
        }
        device->EndScene();
        hr = swapchain->Present(NULL, NULL, NULL, NULL, 0);
        if(FAILED(hr))
        {
            printf("present failed\n");
            break;
        }

        if(!time_limit) print_fps();
        else
        {
            /* Don't time the first frame. AMD cards on Windows need a while to switch display modes
             * or slow first-time draws.
             */
            if(frames == 0) start = GetTickCount();
            frames++;
        }
    }
    printf("frames per second: %f\n", 1000.0 * ((float) frames) / ((float) time_limit));
    swapchain->Release();
}

int main(int argc, char*argv[])
{
    long ref;
    if(argc > 1) time_limit = atol(argv[1]);
    if(time_limit)
    {
        printf("Running for %lu seconds\n", time_limit);
        time_limit *= 1000;
    }
	device = create_device();
    if(!device) return 1;

    draw_loop();
    ref = device->Release();
    printf("refcount %ld\n", ref);
}

