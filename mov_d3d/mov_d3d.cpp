#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>

unsigned long time_limit = 0;
static DWORD prev_time;

/* A triangle list makes it easy to draw the entire cube in one draw, eliminating the drawprim
 * overhead as much as possible. The drawprim overhead is subject of a different test
 */
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

static const char *vs_txt =
    "vs_2_0\n"
    "dcl_position v0\n"
    "mov oPos, v0\n"
    "mov oD0, v0\n";

static const char *ps_txt_mov =
    "ps_3_0\n"
	"dcl_texcoord v0\n"
	"mov r0, c0\n"

	"mov r2, c1\n"
	"mov r3, c2\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c4\n"
	"mov r3, c5\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c7\n"
	"mov r3, c8\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c10\n"
	"mov r3, c11\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c13\n"
	"mov r3, c14\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c16\n"
	"mov r3, c17\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c19\n"
	"mov r3, c20\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

	"mov r2, c22\n"
	"mov r3, c23\n"
	"mov r1.x, r3\n"
	"mov r1.y, r3\n"
	"mov r1.z, r3\n"
	"mov r1.w, r3\n"
	"add r0, r0, r1\n"

    "mov oC0, r0\n";

#if 0
static const char *ps_txt_mov_opt =
    "ps_3_0\n"
	"dcl_texcoord v0\n"
	"mov r0, c0\n"
	"add r0, r0, c2\n"
	"add r0, r0, c5\n"
	"add r0, r0, c8\n"
	"add r0, r0, c11\n"
	"add r0, r0, c14\n"
	"add r0, r0, c17\n"
	"add r0, r0, c20\n"
	"add r0, r0, c23\n"
    "mov oC0, r0\n";
#endif

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
	ID3DXBuffer *vs_code = NULL, *ps_code = NULL, *msgs = NULL;
	const float zero[] = {0.0f, 0.0f, 0.0f, 0.0f};
	const float one[] = {1.0f, 1.0f, 1.0f, 1.0f};
	unsigned int i;

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
    presparm.BackBufferWidth = 1920; /* FIXME */
    presparm.BackBufferHeight = 1200;
    presparm.AutoDepthStencilFormat = D3DFMT_D24X8;
    presparm.EnableAutoDepthStencil = FALSE;
    presparm.SwapEffect = D3DSWAPEFFECT_DISCARD;
    presparm.Windowed = FALSE;
    presparm.hDeviceWindow = window;
    presparm.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
        &presparm, &dev);
    d3d9->Release();
    if(FAILED(hr)) goto err;

    hr = dev->CreateVertexBuffer(sizeof(quad), 0, 0, D3DPOOL_MANAGED, &cubebuffer, NULL);
    if(FAILED(hr)) goto err;
    cubebuffer->Lock(0, 0, &data, 0);
    memcpy(data, quad, sizeof(quad));
    cubebuffer->Unlock();
    dev->SetStreamSource(0, cubebuffer, 0, sizeof(*quad));

	hr = D3DXAssembleShader(vs_txt, strlen(vs_txt), NULL, NULL, 0, &vs_code, &msgs);
	if (FAILED(hr))
	{
		printf("vs msgs: %s\n", msgs->GetBufferPointer());
		goto err;
	}

	hr = dev->CreateVertexShader((DWORD *)vs_code->GetBufferPointer(), &vs);
    if (FAILED(hr)) goto err;

	hr = D3DXAssembleShader(ps_txt_mov, strlen(ps_txt_mov), NULL, NULL, 0, &ps_code, &msgs);
	if (FAILED(hr))
	{
		printf("ps msgs: %s\n", msgs->GetBufferPointer());
		goto err;
	}

	hr = dev->CreatePixelShader((DWORD *) ps_code->GetBufferPointer(), &ps);
    if (FAILED(hr)) goto err;

	hr = dev->CreateVertexDeclaration(decl_elements, &decl);
    if (FAILED(hr)) goto err;

    dev->SetVertexShader(vs);
    dev->SetPixelShader(ps);
    dev->SetVertexDeclaration(decl);

    vs->Release();
    ps->Release();
    decl->Release();
    cubebuffer->Release();

	dev->SetPixelShaderConstantF(0, one, 1);
	for (i = 1; i < 256; i++)
	{
		dev->SetPixelShaderConstantF(i, zero, 1);
	}

	return dev;

err:
    printf("Init error\n");
	if (vs_code) vs_code->Release();
	if (ps_code) ps_code->Release();
	if (msgs) msgs->Release();
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
        device->DrawPrimitive(D3DPT_TRIANGLESTRIP,  0, 2);
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
