#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>

unsigned long time_limit = 0;
static DWORD prev_time;

static const struct
{
    float x, y, z;
    DWORD normal;
    SHORT t0u, t0v;
    DWORD tangent, binormal, color;
    SHORT t1u, t1v;
}
quad[] =
{
    /* Front side */
    {   -1.0,   -1.0,   0.0, 0xff000000,    0,          0,          0xff000000, 0xff000000, 0xffffff00,     0,          0           },
    {   -1.0,    1.0,   0.0, 0xff000000,    0,          ~((SHORT)0),0xff000000, 0xff000000, 0xffffff00,     0,          ~((SHORT)0) },
    {    1.0,   -1.0,   0.0, 0xff000000,    ~((SHORT)0),  0,        0xff000000, 0xff000000, 0xffffff00,     ~((SHORT)0),0           },
    {    1.0,    1.0,   0.0, 0xff000000,    ~((SHORT)0),~((SHORT)0),0xff000000, 0xff000000, 0xffffff00,     ~((SHORT)0),~((SHORT)0) },

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
    "ps_3_0\n"
    "def c4, 0.212500006, 0.715399981, 0.0720999986, 0.00392156886\n"
    "def c5, 255.999985, 65535.9961, 16777215, 0.5\n"
    "def c6, 2, -1, 0, 1.00392163\n"
    "dcl_texcoord v0.x\n"
    "dcl_texcoord1 v1.xy\n"
    "dcl_texcoord2 v2.xy\n"
    "dcl_texcoord3_pp v3.xyz\n"
    "dcl_texcoord4_pp v4.xyz\n"
    "dcl_texcoord5_pp v5.xyz\n"
    "dcl_2d s0\n"
    "dcl_2d s1\n"
    "dcl_2d s2\n"
    "dcl_2d s3\n"
    "dcl_2d s4\n"
    "texld_pp r0, v1, s1\n"
    "mul_pp r1.xyz, r0, r0\n"
    "cmp_pp r0.xyz, c0.x, r0, r1\n"
    "texld_pp r1, v2, s0\n"
    "mul_pp r2.xyz, r1, r1\n"
    "cmp_pp r1.xyz, c0.x, r1, r2\n"
    "mul_pp oC2.xyz, r0, r1\n"
    "texld_pp r0, v1, s3\n"
    "dp3_pp oC1.z, r0, c4\n"
    "mul r0.x, c3.y, v0.x\n"
    "mul r0.xyz, r0.x, c5\n"
    "frc r1.xyz, r0\n"
    "add r0.xyz, r0, -r1\n"
    "mul r1.xyz, r0.xxyw, c6.zwww\n"
    "mad oC0.xyz, r0, c4.w, -r1\n"
    "mov_pp oC0.w, c2.x\n"
    "texld r0, v1, s2\n"
    "mad_pp r0.xyz, r0, c6.x, c6.y\n"
    "mul_pp r1.xyz, r0.y, v5\n"
    "mad_pp r0.xyw, r0.x, v4.xyzz, r1.xyzz\n"
    "mad_pp r0.xyz, r0.z, v3, r0.xyww\n"
    "nrm_pp r1.xyz, r0\n"
    "mad_pp r0.xyz, r1, c5.w, c5.w\n"
    "mov r0.w, c6.z\n"
    "texldl_pp r1, r0.yxww, s4\n"
    "mov_pp oC1.y, r0.z\n"
    "mov_pp oC1.x, r1.x\n"
    "mov oC1.w, c6.z\n"
    "mov oC2.w, c6.z\n";

const char *vs_txt =
    "vs_3_0\n"
    "def c8, -127, 0.00787401572, 0.000488519785, 0\n"
    "dcl_position v0\n"
    "dcl_normal v1\n"
    "dcl_binormal v2\n"
    "dcl_tangent v3\n"
    "dcl_texcoord v4\n"
    "dcl_texcoord1 v5\n"
    "dcl_position o0\n"
    "dcl_texcoord o1.x\n"
    "dcl_texcoord1 o2.xy\n"
    "dcl_texcoord2 o3.xy\n"
    "dcl_texcoord3 o4.xyz\n"
    "dcl_texcoord4 o5.xyz\n"
    "dcl_texcoord5 o6.xyz\n"
    "add r0.xyz, c8.x, v1\n"
    "mul r0.xyz, r0, c8.y\n"
    "dp3 r1.x, r0, c4\n"
    "dp3 r1.y, r0, c5\n"
    "dp3 r1.z, r0, c6\n"
    "dp3 r0.x, r1, r1\n"
    "rsq r0.x, r0.x\n"
    "mul o4.xyz, r0.x, r1\n"
    "dp4 r0.x, v0, c4\n"
    "dp4 r0.y, v0, c5\n"
    "dp4 r0.z, v0, c6\n"
    "dp4 r0.w, v0, c7\n"
    "dp4 o0.x, r0, c0\n"
    "dp4 o0.y, r0, c1\n"
    "dp4 o0.z, r0, c2\n"
    "dp4 r0.x, r0, c3\n"
    "mul o2.xy, c8.z, v4\n"
    "mul o3.xy, c8.z, v5\n"
    "add r0.yzw, c8.x, v3.xxyz\n"
    "mul r0.yzw, r0, c8.y\n"
    "dp3 r1.x, r0.yzww, c4\n"
    "dp3 r1.y, r0.yzww, c5\n"
    "dp3 r1.z, r0.yzww, c6\n"
    "dp3 r0.y, r1, r1\n"
    "rsq r0.y, r0.y\n"
    "mul o5.xyz, r0.y, r1\n"
    "add r0.yzw, c8.x, v2.xxyz\n"
    "mul r0.yzw, r0, c8.y\n"
    "dp3 r1.x, r0.yzww, c4\n"
    "dp3 r1.y, r0.yzww, c5\n"
    "dp3 r1.z, r0.yzww, c6\n"
    "dp3 r0.y, r1, r1\n"
    "rsq r0.y, r0.y\n"
    "mul o6.xyz, r0.y, r1\n"
    "mov o0.w, r0.x\n"
    "mov o1.x, r0.x\n";

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
	IDirect3DTexture9 *tex[4] = {NULL, NULL, NULL, NULL};
	const float half[] = {0.5f, 0.5f, 0.5f, 0.5f};
	unsigned int i;
    const float identity[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    IDirect3DSurface9 *rt1 = NULL, *rt2 = NULL;

    void *data;
    static const D3DVERTEXELEMENT9 decl_elements[] = {
        {0,  0, D3DDECLTYPE_FLOAT3,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
        {0, 12, D3DDECLTYPE_UBYTE4,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,    0},
        {0, 16, D3DDECLTYPE_SHORT2,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0},
        {0, 20, D3DDECLTYPE_UBYTE4,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,   0},
        {0, 24, D3DDECLTYPE_UBYTE4,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,  0},
        {0, 28, D3DDECLTYPE_D3DCOLOR,    D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0},
        {0, 32, D3DDECLTYPE_SHORT2,      D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1},
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

	hr = D3DXAssembleShader(ps_txt, strlen(ps_txt), NULL, NULL, 0, &ps_code, &msgs);
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

	for (i = 0; i < 256; i++)
	{
		dev->SetPixelShaderConstantF(i, half, 1);
	}
    dev->SetVertexShaderConstantF(0, identity, 4);
    dev->SetVertexShaderConstantF(4, identity, 4);

    hr = D3DXCreateTextureFromFileEx(dev, "tex0.dds", D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE,
            D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &tex[0]);
    if (FAILED(hr))
        goto err;
    hr = D3DXCreateTextureFromFileEx(dev, "tex1.dds", D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE,
            D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &tex[1]);
    if (FAILED(hr))
        goto err;
    hr = D3DXCreateTextureFromFileEx(dev, "tex2.dds", D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE,
            D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &tex[2]);
    if (FAILED(hr))
        goto err;
    hr = D3DXCreateTextureFromFileEx(dev, "tex3.dds", D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE,
            D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &tex[3]);
    if (FAILED(hr))
        goto err;
    hr = D3DXCreateTextureFromFileEx(dev, "tex4.dds", D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, D3DUSAGE_DYNAMIC,
            D3DFMT_FROM_FILE, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &tex[4]);
    if (FAILED(hr))
        goto err;

    for (i = 0; i < (sizeof(tex) / sizeof(*tex)); i++)
    {
        hr = dev->SetTexture(i, tex[i]);
        if (FAILED(hr))
            goto err;
    }
    
    hr = dev->CreateRenderTarget(presparm.BackBufferWidth, presparm.BackBufferHeight, D3DFMT_A8R8G8B8, presparm.MultiSampleType, presparm.MultiSampleQuality, FALSE, &rt1, NULL);
    if (FAILED(hr)) goto err;
    hr = dev->CreateRenderTarget(presparm.BackBufferWidth, presparm.BackBufferHeight, D3DFMT_A8R8G8B8, presparm.MultiSampleType, presparm.MultiSampleQuality, FALSE, &rt2, NULL);
    if (FAILED(hr)) goto err;

    hr = dev->SetRenderTarget(1, rt1);
    if (FAILED(hr)) goto err;
    hr = dev->SetRenderTarget(2, rt2);
    if (FAILED(hr)) goto err;

    rt1->Release();
    rt2->Release();
    vs->Release();
    ps->Release();
    decl->Release();
    cubebuffer->Release();
    for (i = 0; i < (sizeof(tex) / sizeof(*tex)); i++)
        tex[i]->Release();

    return dev;

err:
    printf("Init error\n");
    if (rt1) rt1->Release();
    if (rt2) rt2->Release();
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
