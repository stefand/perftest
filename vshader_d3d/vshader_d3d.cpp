#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>

unsigned long time_limit = 0;

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

static const unsigned int load_instances = 1000;
static const unsigned int draw_instances = 1000;
static DWORD prev_time;

static IDirect3DDevice9 *device;
IDirect3DVertexShader9 *shaders[load_instances];

static const DWORD ps_code[] = {
    0xffff0200,                                                                 /* ps_2_0                       */
    0x0200001f, 0x80000000, 0x900f0000,                                         /* dcl v0                       */
    0x02000001, 0x800f0800, 0x90e40000,                                         /* mov oC0, v0                  */
    0x0000ffff                                                                  /* end                          */
};

static const DWORD vs_code_template[] = {
    0xfffe0200,                                                                 /* vs_2_0                       */
    0x05000051, 0xa00f0005, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     /* def c5, 0.0, 0.0, 0.0, 0.0   */
    0x05000051, 0xa00f0006, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     /* def c6, 0.0, 0.0, 0.0, 0.0   */
    0x05000051, 0xa00f0004, 0xc1200000, 0x40000000, 0x3d4ccccd, 0x00000000,     /* def c4, -10, 2, 0.05, 0      */
    0x0200001f, 0x80000000, 0x900f0000,                                         /* dcl_position v0              */
    0x03000002, 0x80070000, 0x90e40000, 0xa0000004,                             /* add r0.xyz, v0, c4.x         */
    0x02000001, 0x800f0001, 0xa0e40005,                                         /* mov r1, c5                   */
    0x04000004, 0x80070000, 0x80e40001, 0xa0550004, 0x80e40000,                 /* mad r0.xyz, r1, c4.y, r0     */
    0x03000005, 0x80070000, 0x80e40000, 0xa0aa0004,                             /* mul r0.xyz, r0, c4.z         */
    0x02000001, 0x80080000, 0x90ff0000,                                         /* mov r0.w, v0.w               */
    0x03000009, 0xc0010000, 0x80e40000, 0xa0e40000,                             /* dp4 oPos.x, r0, c0           */
    0x03000009, 0xc0020000, 0x80e40000, 0xa0e40001,                             /* dp4 oPos.y, r0, c1           */
    0x03000009, 0xc0040000, 0x80e40000, 0xa0e40002,                             /* dp4 oPos.z, r0, c2           */
    0x03000009, 0xc0080000, 0x80e40000, 0xa0e40003,                             /* dp4 oPos.w, r0, c3           */
    0x02000001, 0xd00f0000, 0xa0e40006,                                         /* mov oD0, c6                  */
    0x0000ffff                                                                  /* end                          */
};

#define POS_OFFSET 3
#define COL_OFFSET 9

static void init_shaders(IDirect3DDevice9 *dev)
{
    unsigned int i;
    unsigned int x = 0, y = 0, z = 0;
    unsigned int color = 0;
    DWORD vs_code[sizeof(vs_code_template)/sizeof(*vs_code_template)];
    HRESULT hr;
    float *pos = (float *)(vs_code + POS_OFFSET);
    float *col =  (float *)(vs_code + COL_OFFSET);

    memcpy(vs_code, vs_code_template, sizeof(vs_code_template));

    dev->BeginScene();
    for(i = 0; i < load_instances; i++)
    {
        pos[0] = (float) x;
        pos[1] = (float) y;
        pos[2] = (float) z;
        switch(color)
        {
            case 9:
                color = 0;
                /* Drop through */
            case 0:
                col[0] = 1.0;
                col[1] = 0.0;
                col[2] = 0.0;
                break;
            case 1:
                col[0] = 0.0;
                col[1] = 1.0;
                col[2] = 0.0;
                break;
            case 2:
                col[0] = 0.0;
                col[1] = 0.0;
                col[2] = 1.0;
                break;
            case 3:
                col[0] = 1.0;
                col[1] = 1.0;
                col[2] = 0.0;
                break;
            case 4:
                col[0] = 1.0;
                col[1] = 0.0;
                col[2] = 1.0;
                break;
            case 5:
                col[0] = 0.0;
                col[1] = 1.0;
                col[2] = 1.0;
                break;
            case 6:
                col[0] = 1.0;
                col[1] = 1.0;
                col[2] = 1.0;
                break;
            case 7:
                col[0] = 0.5;
                col[1] = 1.0;
                col[2] = 0.0;
                break;
            case 8:
                col[0] = 1.0;
                col[1] = 0.5;
                col[2] = 0.0;
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

        hr = dev->CreateVertexShader(vs_code, &shaders[i]);
        if(FAILED(hr))
        {
            printf("Failed to create vertex shader %u\n", i);
            exit(0);
        }
        dev->SetVertexShader(shaders[i]);
        hr = dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,  0, 2);
        if(FAILED(hr))
        {
            printf("Prepare draw with shader %u failed\n", i);
            exit(0);
        }
    }
    dev->EndScene();
}

static HWND create_window(void)
{
    WNDCLASSW wc = {0};
    HWND ret;
    wc.lpfnWndProc = DefWindowProcW;
    wc.lpszClassName = L"cube_perftest_wc";
    RegisterClassW(&wc);

    ret = CreateWindowW(L"cube_perftest_wc", L"ddadda",
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
    IDirect3DPixelShader9 *ps = NULL;
    D3DXMATRIX mat, scale, translate;
    D3DXVECTOR3 axis;

    void *data;
    static const D3DVERTEXELEMENT9 decl_elements[] = {
        {0, 0, D3DDECLTYPE_FLOAT3,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,   0},
        D3DDECL_END()
    };

    window = create_window();
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
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

    hr = dev->CreateVertexBuffer(sizeof(cube), 0, 0, D3DPOOL_MANAGED, &cubebuffer, NULL);
    if(FAILED(hr)) goto err;
    cubebuffer->Lock(0, 0, &data, 0);
    memcpy(data, cube, sizeof(cube));
    cubebuffer->Unlock();
    dev->SetStreamSource(0, cubebuffer, 0, sizeof(*cube));

    hr = dev->CreatePixelShader(ps_code, &ps);
    if (FAILED(hr)) goto err;

    hr = dev->CreateVertexDeclaration(decl_elements, &decl);
    if (FAILED(hr)) goto err;

    dev->SetVertexDeclaration(decl);
    dev->SetPixelShader(ps);

    axis.x = 1.0;
    axis.y = 1.0;
    axis.z = 0.0;
    D3DXMatrixIdentity(&mat);
    D3DXMatrixRotationAxis(&mat, &axis, 45);
    D3DXMatrixScaling(&scale, 1.0, 1.0, 0.5);
    D3DXMatrixTranslation(&translate, 0.0, 0.0, 0.5);
    D3DXMatrixMultiply(&mat, &mat, &scale);
    D3DXMatrixMultiply(&mat, &mat, &translate);
    float m2[4][4] = {
        {   mat.m[0][0],    mat.m[1][0],    mat.m[2][0],    mat.m[3][0]     },
        {   mat.m[0][1],    mat.m[1][1],    mat.m[2][1],    mat.m[3][1]     },
        {   mat.m[2][2],    mat.m[1][2],    mat.m[2][2],    mat.m[3][2]     },
        {   mat.m[0][3],    mat.m[1][3],    mat.m[2][3],    mat.m[3][3]     },
    };
    dev->SetVertexShaderConstantF(0, *m2, 4);

    ps->Release();
    decl->Release();
    cubebuffer->Release();

    init_shaders(dev);

    return dev;

err:
    printf("Init error\n");
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
        for(i = 0; i < draw_instances; i++)
        {
            device->SetVertexShader(shaders[i]);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP,  0, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP,  4, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP,  8, 1);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 12, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 16, 2);
            device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 20, 2);
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
            frames++;
            if(frames == 0) start = GetTickCount();
        }
    }
    printf("frames per second: %f\n", 1000.0 * ((float) frames) / ((float) time_limit));
    swapchain->Release();
    for(i = 0; i < draw_instances; i++)
    {
        shaders[i]->Release();
    }
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

