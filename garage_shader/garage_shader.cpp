#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>

unsigned long time_limit = 0;
static DWORD prev_time;

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
    "mov oT0, v0\n";

const char *ps_txt_const =
    "ps_2_0\n"
#if 0
    "def c0, 0.92341375, 0.34255252342, 1.51991844e-004, 1\n"
    "def c1, 0.347353422, 0.4633423421, 1.51991844e-003, 1\n"
    "def c2, 0.99609375, 0.00389099121, 1.51991844e-002, 1\n"
    "def c3, 0.53212311, 0.20389099121, 1.51991844e-001, 1\n"
    "def c4, 0.23452315, 0.00389099121, 1.452345122, 1\n"
    "def c5, 0.42345523, 0.01389099121, 2.33513123, 1\n"
    "def c6, 0.99609375, 0.20389099121, 1.51991844e-005, 1\n"
    "def c7, 0.99609375, 0.30389099121, 1.51991844e-005, 1\n"
    "def c8, 0.99609375, 0.40389099121, 1.51991844e-005, 1\n"
    "def c9, 0.25, -0.556640983, -0.0371089987, -0.654296994\n"
    "def c10, 0.173828006, 0.111327998, 0.0644529983, 255\n"
    "def c11, 0.00195299997, 0.0820309967, -0.0605470017, 0\n"
    "def c12, 0.220703006, -0.359375, -0.0625, -5\n"
    "def c13, 0.242188007, 0.126953006, -0.25, 0\n"
    "def c14, 0.0703129992, -0.0253909994, 0.148438007, 0\n"
    "def c15, -0.078125, 0.0136719998, -0.314453006, 0\n"
    "def c16, 0.117187999, -0.140625, -0.199219003, 0\n"
    "def c17, 2, -1, 0.499999583, 0.5\n"
    "def c18, 6.28318548, -3.14159274, 1, -1\n"
    "def c19, -1, -2, -3, -4\n"
    "def c20, 0, 1, 0.125, 0\n"
#endif
    "dcl t0\n"

	"dp4 r0.x, t0, c0\n"
	"dp4 r0.y, t0, c1\n"
	"dp4 r0.z, t0, c2\n"
	"dp4 r0.w, t0, c3\n"

	"dp4 r0.x, r0, c4\n"
	"dp4 r0.y, r0, c5\n"
	"dp4 r0.z, r0, c6\n"
	"dp4 r0.w, r0, c7\n"

	"dp4 r0.x, r0, c8\n"
	"dp4 r0.y, r0, c9\n"
	"dp4 r0.z, r0, c10\n"
	"dp4 r0.w, r0, c11\n"

	"dp4 r0.x, r0, c12\n"
	"dp4 r0.y, r0, c13\n"
	"dp4 r0.z, r0, c14\n"
	"dp4 r0.w, r0, c15\n"

	"dp4 r0.x, r0, c16\n"
	"dp4 r0.y, r0, c17\n"
	"dp4 r0.z, r0, c18\n"
	"dp4 r0.w, r0, c19\n"

	"add r0, r0, c20\n"

	"mov oC0, r0\n"
;

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
	IDirect3DTexture9 *tex[3] = {NULL, NULL, NULL};
	const float half[] = {0.5f, 0.5f, 0.5f, 0.5f};
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
    presparm.BackBufferHeight = 1080;
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

	hr = D3DXAssembleShader(ps_txt_const, strlen(ps_txt_const), NULL, NULL, 0, &ps_code, &msgs);
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

	for (i = 0; i < 256; i++)
	{
		dev->SetPixelShaderConstantF(i, half, 1);
	}

	for (i = 0; i < sizeof(tex) / sizeof(*tex); i++)
	{
		D3DLOCKED_RECT lr;
		unsigned int y;
		D3DSURFACE_DESC desc;

		hr = dev->CreateTexture(1024, 768, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &tex[i], NULL);
		if (FAILED(hr))
			goto err;

		hr = tex[i]->GetLevelDesc(0, &desc);
		if (FAILED(hr))
			goto err;
		hr = tex[i]->LockRect(0, &lr, NULL, 0);
		if (FAILED(hr))
			goto err;

		for (y = 0; y < desc.Height; y++)
		{
			memset(((BYTE *)lr.pBits) + lr.Pitch * y, 0xff, lr.Pitch);
		}

		tex[i]->UnlockRect(0);

		dev->SetTexture(i, tex[i]);
	}

	return dev;

err:
    printf("Init error\n");
	for (i = 0; i < sizeof(tex) / sizeof(*tex); i++)
		if (tex[i]) tex[i]->Release();
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
