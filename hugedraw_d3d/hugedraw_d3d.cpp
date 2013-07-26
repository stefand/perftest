#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>

unsigned long time_limit = 0;
static DWORD prev_time;

static const unsigned int mesh_width = 256, mesh_height = 256;
struct mesh
{
    float x, y, z;
    float r, g, b, a;
    DWORD dummy;
};

static IDirect3DDevice9 *device;

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

static HWND window;

const char *ps_txt =
    "ps_2_0\n"
    "dcl t0\n"
    "dcl t1\n"
    "dcl v0\n"
    "mov r0, v0\n"
    "mul r0, r0, t0\n"
    "mul r0, r0, t1\n"
    "mov oC0, r0\n";

const char *vs_txt =
    "vs_2_0\n"
    "def c0, 1, 0, 0, 0\n"
    "dcl_position v0\n"
    "dcl_color v1\n"
    "dcl_color1 v2\n"
    "dcl_tangent v3\n"
    "dcl_binormal v4\n"
    "mov oT0, v1\n"
    "mov oD0, v2\n"
    "mov r0, v3\n"
    "mov r0.zw, v4.xxxy\n"
    "sub oT1, c0.x, r0\n"
    "mad oPos, v0, c0.xxxy, c0.yyyx\n";

void create_mesh(struct mesh *data)
{
    unsigned int x, y, vtx;
    struct mesh *cur;
    
    for (y = 0; y < mesh_height; y++)
    {
        for (x = 0; x < mesh_width; x++)
        {
            cur = &data[(y * mesh_width + x) * 6];

            for (vtx = 0; vtx < 6; vtx++)
            {
                cur[vtx].dummy = 0xffffffff;
                cur[vtx].r = 1.0f - ((float) y) / mesh_height;
                cur[vtx].g = 1.0f - ((float) x) / mesh_width;
                cur[vtx].b = ((float) y) / mesh_width;
                cur[vtx].z = 0.5f;
            }

            cur[0].x = ((((float) x) / mesh_width) - 0.5f) * 2.0f;
            cur[0].y = ((((float) y) / mesh_height) - 0.5f) * 2.0f;
            cur[2].x = (((((float) x) + 1.0f) / mesh_width) - 0.5f) * 2.0f;
            cur[2].y = ((((float) y) / mesh_height) - 0.5f) * 2.0f;
            cur[1].x = (((((float) x) + 1.0f) / mesh_width) - 0.5f) * 2.0f;
            cur[1].y = (((((float) y) + 1.0f) / mesh_height) - 0.5f) * 2.0f;

            cur[3].x = ((((float) x) / mesh_width) - 0.5f) * 2.0f;
            cur[3].y = ((((float) y) / mesh_height) - 0.5f) * 2.0f;
            cur[4].x = (((((float) x) + 1.0f) / mesh_width) - 0.5f) * 2.0f;
            cur[4].y = (((((float) y) + 1.0f) / mesh_height) - 0.5f) * 2.0f;
            cur[5].x = ((((float) x) / mesh_width) - 0.5f) * 2.0f;
            cur[5].y = (((((float) y) + 1.0f) / mesh_height) - 0.5f) * 2.0f;
#if 0
            printf("%1.0fx%1.0f  %1.0fx%1.0f\n", cur[0].x, cur[0].y, cur[1].x, cur[1].y);
            printf("       %1.0fx%1.0f\n", cur[2].x, cur[2].y);
            printf("%1.0fx%1.0f       \n", cur[3].x, cur[3].y);
            printf("%1.0fx%1.0f  %1.0fx%1.0f\n", cur[4].x, cur[4].y, cur[5].x, cur[5].y);
#endif
        }
    }
}

static IDirect3DDevice9 *create_device()
{
    HRESULT hr;
    IDirect3D9 *d3d9;
    IDirect3DDevice9 *dev = NULL;
    D3DPRESENT_PARAMETERS presparm;
    IDirect3DVertexBuffer9 *meshbuffer = NULL, *buf2 = NULL, *buf3 = NULL;
    IDirect3DVertexDeclaration9 *decl = NULL;
	IDirect3DVertexShader9 *vs = NULL;
	IDirect3DPixelShader9 *ps = NULL;
	ID3DXBuffer *vs_code = NULL, *ps_code = NULL, *msgs = NULL;
    unsigned int bufsize;

    void *data;
    static const D3DVERTEXELEMENT9 decl_elements[] = {
        { 0,  0, D3DDECLTYPE_FLOAT3,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
        { 0, 12, D3DDECLTYPE_FLOAT4,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0},
        { 0, 28, D3DDECLTYPE_D3DCOLOR,    D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     1},
        { 1,  0, D3DDECLTYPE_SHORT2,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,  0},
        {11,  0, D3DDECLTYPE_SHORT2,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,   0},
        D3DDECL_END()
    };

    window = create_window();
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if(!d3d9) return NULL;

    memset(&presparm, 0, sizeof(presparm));
    presparm.BackBufferCount = 1;
    presparm.BackBufferFormat = D3DFMT_X8R8G8B8;
    presparm.BackBufferWidth = 1024;
    presparm.BackBufferHeight = 768;
    presparm.AutoDepthStencilFormat = D3DFMT_D24X8;
    presparm.EnableAutoDepthStencil = FALSE;
    presparm.SwapEffect = D3DSWAPEFFECT_DISCARD;
    presparm.Windowed = TRUE;
    presparm.hDeviceWindow = window;
    presparm.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
        &presparm, &dev);
    d3d9->Release();
    if(FAILED(hr)) goto err;

    hr = dev->CreateVertexBuffer(sizeof(struct mesh) * mesh_width * mesh_height * 6, 0, 0, D3DPOOL_DEFAULT, &meshbuffer, NULL);
    if(FAILED(hr)) goto err;
    meshbuffer->Lock(0, 0, &data, 0);
    create_mesh((struct mesh *)data);
    meshbuffer->Unlock();
    dev->SetStreamSource(0, meshbuffer, 0, sizeof(struct mesh));

    bufsize = sizeof(USHORT) * 2 * mesh_width * mesh_height * 6;
    hr = dev->CreateVertexBuffer(bufsize, 0, 0, D3DPOOL_DEFAULT, &buf2, NULL);
    if(FAILED(hr)) goto err;
    buf2->Lock(0, 0, &data, 0);
    memset(data, 0, bufsize);
    buf2->Unlock();
    dev->SetStreamSource(1, buf2, 0, sizeof(USHORT) * 2);

    bufsize = sizeof(USHORT) * 2 * mesh_width * mesh_height * 6;
    hr = dev->CreateVertexBuffer(bufsize, 0, 0, D3DPOOL_DEFAULT, &buf3, NULL);
    if(FAILED(hr)) goto err;
    buf3->Lock(0, 0, &data, 0);
    memset(data, 0, bufsize);
    buf3->Unlock();
    dev->SetStreamSource(11, buf3, 0, sizeof(USHORT) * 2);

	hr = D3DXAssembleShader(vs_txt, strlen(vs_txt), NULL, NULL, 0, &vs_code, &msgs);
	if (FAILED(hr))
	{
		printf("vs msgs: %s\n", msgs->GetBufferPointer());
		goto err;
	}

	hr = dev->CreateVertexShader((DWORD *)vs_code->GetBufferPointer(), &vs);
    if (FAILED(hr)) goto err;

	hr = D3DXAssembleShader(ps_txt, strlen(ps_txt), NULL, NULL, 0, &ps_code, &msgs);
	if (FAILED(hr))
	{
		printf("ps msgs: %s\n", msgs->GetBufferPointer());
		goto err;
	}

	hr = dev->CreatePixelShader((DWORD *)ps_code->GetBufferPointer(), &ps);
    if (FAILED(hr)) goto err;

	hr = dev->CreateVertexDeclaration(decl_elements, &decl);
    if (FAILED(hr)) goto err;

    dev->SetVertexShader(vs);
    dev->SetPixelShader(ps);
    dev->SetVertexDeclaration(decl);

    vs->Release();
    ps->Release();
    decl->Release();
    meshbuffer->Release();

    return dev;

err:
    printf("Init error\n");
	if (vs_code) vs_code->Release();
	if (ps_code) ps_code->Release();
	if (msgs) msgs->Release();
    if (vs) vs->Release();
    if (ps) ps->Release();
    if (decl) decl->Release();
    if (meshbuffer) meshbuffer->Release();
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

static void draw_loop()
{
    MSG msg;
    HRESULT hr;
    IDirect3DSwapChain9 *swapchain;
    DWORD start;
    unsigned long frames = ~0UL;

    device->GetSwapChain(0, &swapchain);
 
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
    start = GetTickCount();
    while(!time_limit || (start + time_limit > GetTickCount()))
    {
        hr = device->Clear(0, NULL, D3DCLEAR_TARGET, 0x1a1a1a1a, 1.0, 0);
        if(FAILED(hr)) printf("Clear failed\n");
        while(PeekMessageW(&msg, window, 0, 0, PM_REMOVE) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        device->BeginScene();
        device->DrawPrimitive(D3DPT_TRIANGLELIST,  0, mesh_width * mesh_height * 2);
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
