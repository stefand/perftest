/* Stupid mingw g++ */
#include <algorithm>
using namespace std;

#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <d3dx9.h>

#include "data.h"

unsigned long time_limit = 0;
static DWORD prev_time;

static IDirect3DDevice9 *device;
static HWND window;

static HWND create_window(void)
{
    WNDCLASSW wc = {0};
    HWND ret;
    wc.lpfnWndProc = DefWindowProcW;
    wc.lpszClassName = L"cube_perftest_wc";
    RegisterClassW(&wc);

    ret = CreateWindowW(L"cube_perftest_wc", L"ddadda",
                        WS_CAPTION , 100, 100, 1024, 768, 0, 0, 0, 0);
    ShowWindow(ret, SW_SHOW);

    return ret;
}

static const char *blit_vs_txt =
    "vs_3_0\n"
    "dcl_position v0\n"
    "dcl_texcoord v1\n"
    "dcl_position o0\n"
    "dcl_texcoord o1\n"
    "mov o0, v0\n"
    "mov o1, v1\n";

static const char *blit_ps_txt =
    "ps_3_0\n"
    "dcl_texcoord v0\n"
    "dcl_2d s0\n"
    "texld r0, v0, s0\n"
    "mov oC0, r0\n";

static IDirect3DVertexShader9 *blit_vs;
static IDirect3DPixelShader9 *blit_ps;
static IDirect3DVertexDeclaration9 *blit_decl;

static const D3DVERTEXELEMENT9 blit_decl_data[] =
{
    { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, 
    { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};

static const float blit_vtx[] =
{
    -1.0f,      1.0f,     0.0f,     0.0f,     0.0f,
    -1.0f,     -1.0f,     0.0f,     0.0f,     1.0f,
     1.0f,      1.0f,     0.0f,     1.0f,     0.0f,
     1.0f,     -1.0f,     0.0f,     1.0f,     1.0f,
};

static IDirect3DDevice9 *create_device()
{
    HRESULT hr;
    IDirect3D9 *d3d9;
    IDirect3DDevice9 *dev = NULL;
    D3DPRESENT_PARAMETERS presparm;
    ID3DXBuffer *shader_code, *msgs;

    window = create_window();
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if(!d3d9) return NULL;

    memset(&presparm, 0, sizeof(presparm));
    presparm.BackBufferCount = 1;
    presparm.BackBufferFormat = D3DFMT_X8R8G8B8;
    presparm.BackBufferWidth = 1024;
    presparm.BackBufferHeight = 768;
    presparm.AutoDepthStencilFormat = D3DFMT_D24S8;
    presparm.EnableAutoDepthStencil = FALSE;
    presparm.SwapEffect = D3DSWAPEFFECT_DISCARD;
    presparm.Windowed = TRUE;
    presparm.hDeviceWindow = window;
    presparm.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
        &presparm, &dev);
    d3d9->Release();
    if(FAILED(hr)) goto err;

    hr = init_data(dev);
    if (FAILED(hr))
        goto err;

    hr = dev->CreateVertexDeclaration(blit_decl_data, &blit_decl);
    if (FAILED(hr))
        goto err;
	hr = D3DXAssembleShader(blit_vs_txt, strlen(blit_vs_txt), NULL, NULL, 0, &shader_code, NULL);
	if (FAILED(hr))
	{
		printf("vs msgs: XXX\n");
		goto err;
	}
	hr = dev->CreateVertexShader((DWORD *)shader_code->GetBufferPointer(), &blit_vs);
    if (FAILED(hr)) goto err;
    shader_code->Release();
	hr = D3DXAssembleShader(blit_ps_txt, strlen(blit_ps_txt), NULL, NULL, 0, &shader_code, &msgs);
	if (FAILED(hr))
	{
        printf("ps msgs: %s\n", msgs->GetBufferPointer());
        msgs->Release();
		goto err;
	}
	hr = dev->CreatePixelShader((DWORD *)shader_code->GetBufferPointer(), &blit_ps);
    if (FAILED(hr)) goto err;
    shader_code->Release();

	return dev;

err:
    printf("Init error\n");
    if (blit_decl) blit_decl->Release();
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
    IDirect3DSurface9 *backbuffer, *last_rt;
    DWORD start;
    unsigned long frames = ~0UL;
    unsigned int i, j;
    IDirect3DTexture9 *last_tex;

    device->GetSwapChain(0, &swapchain);
    swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    device->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    device->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    device->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

    device->SetSamplerState(2, D3DSAMP_MAXANISOTROPY, 4);

    last_rt = data[numdraws - 1].target;
    last_rt->GetContainer(IID_IDirect3DTexture9, (void **)&last_tex);

    start = GetTickCount();
    while(!time_limit || (start + time_limit > GetTickCount()))
    {
        while(PeekMessageW(&msg, window, 0, 0, PM_REMOVE) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        device->BeginScene();
        for (i = 0; i < numdraws; i++)
        {
            hr = device->SetRenderTarget(0, data[i].target);
            if (FAILED(hr))
                printf("SetRenderTarget failed\n");
            hr = device->SetVertexDeclaration(data[i].decl);
            if (FAILED(hr))
                printf("SetVertexDeclaration failed\n");
            hr = device->SetVertexShader(data[i].vs);
            if (FAILED(hr))
                printf("SetVertexShader failed\n");
            hr = device->SetPixelShader(data[i].ps);
            if (FAILED(hr))
                printf("SetPixelShader failed\n");
            for (j = 0; j < sizeof(data[i].textures) / sizeof(*data[i].textures); j++)
            {
                device->SetTexture(j, data[i].textures[j].texture);
                device->SetSamplerState(j, D3DSAMP_ADDRESSU, data[i].textures[j].address);
                device->SetSamplerState(j, D3DSAMP_ADDRESSV, data[i].textures[j].address);
                device->SetSamplerState(j, D3DSAMP_ADDRESSW, data[i].textures[j].address);
            }
            for (j = 0; j < sizeof(data[i].streams) / sizeof(*data[i].streams); j++)
            {
                hr = device->SetStreamSource(j, data[i].streams[j].buf, data[i].streams[j].offset,
                        data[i].streams[j].stride);
            }
            if (data[i].vs_const)
                device->SetVertexShaderConstantF(0, data[i].vs_const, data[i].vs_const_len);
            if (data[i].ps_const)
                device->SetPixelShaderConstantF(0, data[i].ps_const, data[i].ps_const_len);

            hr = device->DrawPrimitive(D3DPT_TRIANGLESTRIP, data[i].starvtx, data[i].primitives);
            if (FAILED(hr))
                printf("Draw %u failed: %08x\n", i, hr);
        }

        device->SetRenderTarget(0, backbuffer);
        device->SetTexture(0, last_tex);
        device->SetVertexDeclaration(blit_decl);
        device->SetVertexShader(blit_vs);
        device->SetPixelShader(blit_ps);
        hr = device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blit_vtx, sizeof(float) * 5);
        if (FAILED(hr))
            printf("Stretchrect failed: %08x\n", hr);

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
    last_tex->Release();
    backbuffer->Release();
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
