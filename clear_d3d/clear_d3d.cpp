#include <windows.h>
#include <d3d9.h>
#include <cstdio>

unsigned long time_limit = 0;

static DWORD prev_time;

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

HWND window;

static IDirect3DDevice9 *create_device()
{
    HRESULT hr;
    IDirect3D9 *d3d9;
    IDirect3DDevice9 *dev = NULL;
    D3DPRESENT_PARAMETERS presparm;

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

    return dev;

err:
    printf("Init error\n");
    if (dev)
    {
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
    MSG msg;
    HRESULT hr;
    IDirect3DSwapChain9 *swapchain;
    DWORD start;
    unsigned long frames = ~0UL;
    D3DPRESENT_PARAMETERS pp;
    const unsigned int step = 6;

    device->GetSwapChain(0, &swapchain);
    swapchain->GetPresentParameters(&pp);
    start = GetTickCount();
    while(!time_limit || (start + time_limit > GetTickCount()))
    {
        unsigned int x, y;
        while(PeekMessageW(&msg, window, 0, 0, PM_REMOVE) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        for(y = 0; y < pp.BackBufferHeight; y+=step)
        {
            float g = ((float) y) / (pp.BackBufferHeight - 1);

            for(x = 0; x < pp.BackBufferWidth; x+=step)
            {
                float r = ((float) x) / (pp.BackBufferWidth - 1);
                D3DRECT rect;
                DWORD color = 0;

                rect.x1 = x;
                rect.y1 = y;
                rect.x2 = min(pp.BackBufferWidth, x + step);
                rect.y2 = min(pp.BackBufferHeight, y + step);

                color |= ((DWORD) (r * 255)) << 16;
                color |= ((DWORD) (g * 255)) <<  8;

                hr = device->Clear(1, &rect, D3DCLEAR_TARGET, color, 1.0, 0);
                if(FAILED(hr)) printf("Clear failed\n");
            }
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

