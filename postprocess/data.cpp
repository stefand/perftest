/* Stupid mingw g++ */
#include <algorithm>
using namespace std;

#include <cstdio>
#include <d3dx9.h>

#include "data.h"

static const float ps_const1[] =
{
   -8.65951657e-01f,-3.12918157e-04f, 5.00127733e-01f, 1.82914925e+01f,
   -2.11608633e-01f, 9.06307757e-01f,-3.65824997e-01f, 2.79653721e+01f,
   -4.53155160e-01f,-4.22618061e-01f,-7.84884810e-01f, 7.13246078e+01f,
    1.02400000e+03f, 7.68000000e+02f, 5.12000000e+02f, 3.84000000e+02f,
    5.99999987e-02f, 0.00000000e+00f, 0.00000000e+00f, 0.00000000e+00f,
    6.99999988e-01f, 0.00000000e+00f, 0.00000000e+00f, 0.00000000e+00f,
    1.00000000e+00f, 0.00000000e+00f, 0.00000000e+00f, 0.00000000e+00f,
    1.66666679e+01f, 0.00000000e+00f, 0.00000000e+00f, 0.00000000e+00f
};

static const char *vs1 =
    "vs_3_0\n"
    "dcl_position v0\n"
    "dcl_texcoord v1\n"
    "dcl_position o0\n"
    "dcl_texcoord o1.xy\n"
    "mov o0, v0\n"
    "mov o1.xy, v1\n";

static const char *ps1 =
    "ps_3_0\n"
    "def c8, 0.99609375, 0.00389099121, 1.51991844e-005, 1\n"
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
    "dcl_texcoord v0.xy\n"
    "dcl_2d s0\n"
    "dcl_2d s1\n"
    "dcl_2d s2\n"
    // native / wined3d = ratio
    // 1,0105782792665726375176304654443
    "texld_pp r0, v0, s1\n" // 1300 / 1198 = 1,0851419031719532554257095158598
    "mad_pp r0.zw, r0.xyxy, c17.x, c17.y\n"
    "mad_pp r0.z, r0.z, c17.z, c17.w\n"
    "frc_pp r0.z, r0.z\n"
    "mad_pp r0.z, r0.z, c18.x, c18.y\n" // 1243 / 1198 = 1,0375626043405676126878130217028
    "sincos_pp r1.xy, r0.z\n" // 987 / 953 = 1,0356768100734522560335781741868
    "mad_pp r0.z, r0.w, -r0.w, c8.w\n"
    "rsq_pp r0.z, r0.z\n"
    "rcp_pp r0.z, r0.z\n"
    "mul_pp r0.xy, r0.z, r1\n"
    "dp3 r1.x, r0.xyww, c0\n"
    "dp3 r1.y, r0.xyww, c1\n"
    "dp3 r1.z, r0.xyww, c2\n"
    "mul r0.xyz, r1, c18.zwzw\n"
    "nrm r1.xyz, r0\n"
    "mul r0.xy, c3, v0\n"
    "mul r0.zw, r0.xyxy, c9.x\n" // 501 / 493 = 1,0162271805273833671399594320487
    "frc r0.zw, r0_abs\n" // 458 / 450 = 1,0177777777777777777777777777778

    "cmp r0.xy, r0, r0.zwzw, -r0.zwzw\n"
    "texld r0, r0, s2\n"
    "mad r0.xyz, r0, c17.x, c17.y\n"
    "nrm r2.xyz, r0\n"
    "dp3 r0.x, c9.yzww, r2\n"
    "add r0.x, r0.x, r0.x\n"
    "mad r0.xyz, r2, -r0.x, c9.yzww\n"
    "mul r0.xyz, r0, c4.x\n"
    "dp3 r0.w, r0, r1\n"
    "cmp r0.xyz, r0.w, r0.zxyw, -r0.zxyw\n"
    "add r3.xy, r0.yzzw, v0\n"
    "texld r3, r3, s0\n"
    "dp3 r1.w, c10, r2\n"
    "add r1.w, r1.w, r1.w\n"
    "mad r4.xyz, r2, -r1.w, c10\n"
    "mul r4.xyz, r4, c4.x\n"
    "dp3 r1.w, r4, r1\n"
    "cmp r4.xyz, r1.w, r4, -r4\n"
    "mov r0.y, r4.z\n"
    "add r4.xy, r4, v0\n"
    "texld r4, r4, s0\n"
    "dp3 r1.w, c11, r2\n"
    "add r1.w, r1.w, r1.w\n"
    "mad r5.xyz, r2, -r1.w, c11\n"
    "mul r5.xyz, r5, c4.x\n"
    "dp3 r1.w, r5, r1\n"
    "cmp r5.xyz, r1.w, r5, -r5\n"
    "mov r0.z, r5.z\n"
    "add r5.xy, r5, v0\n"
    "texld r5, r5, s0\n"
    "dp3 r1.w, c12, r2\n"
    "add r1.w, r1.w, r1.w\n"
    "mad r6.xyz, r2, -r1.w, c12\n"
    "mul r6.xyz, r6, c4.x\n"
    "dp3 r1.w, r6, r1\n"
    "cmp r6.xyz, r1.w, r6, -r6\n"
    "mov r0.w, r6.z\n"
    "add r6.xy, r6, v0\n"
    "texld r6, r6, s0\n"
    "add r0, r0, c8.w\n"
    "texld r7, v0, s0\n"
    "dp3 r1.w, r7, c8\n"
    "rcp r1.w, r1.w\n"
    "dp3 r7.x, r3, c8\n"
    "dp3 r7.y, r4, c8\n"
    "dp3 r7.z, r5, c8\n"
    "dp3 r7.w, r6, c8\n"
    "mad r0, r7, -r1.w, r0\n"
    "mul r0, r0, c7.x\n"
    "rcp r7.x, r0.x\n"
    "rcp r7.y, r0.y\n"
    "rcp r7.z, r0.z\n"
    "rcp r7.w, r0.w\n"
    "mul_sat r7, r7, c6.x\n"
    "mul_pp r2.w, r3.w, c10.w\n"
    "frc r3.x, r2.w\n"
    "add r2.w, r2.w, -r3.x\n"
    "cmp r3.x, -r3.x, c20.x, c20.y\n"
    "cmp r3.x, r3.w, c11.w, r3.x\n"
    "add r2.w, r2.w, r3.x\n"
    "add r3, r2.w, c19\n"
    "add r2.w, r2.w, c12.w\n"
    "cmp r3.x, -r3_abs.x, c20.y, c20.x\n"
    "cmp r3.x, -r3_abs.y, c11.w, r3.x\n"
    "cmp r3.x, -r3_abs.z, c8.w, r3.x\n"
    "cmp r3.x, -r3_abs.w, c8.w, r3.x\n"
    "cmp r3.x, -r2_abs.w, c8.w, r3.x\n"
    "mul_pp r2.w, r4.w, c10.w\n"
    "frc r4.x, r2.w\n"
    "add r2.w, r2.w, -r4.x\n"
    "cmp r4.x, -r4.x, c20.x, c20.y\n"
    "cmp r4.x, r4.w, c11.w, r4.x\n"
    "add r2.w, r2.w, r4.x\n"
    "add r4, r2.w, c19\n"
    "add r2.w, r2.w, c12.w\n"
    "cmp r4.x, -r4_abs.x, c20.y, c20.x\n"
    "cmp r4.x, -r4_abs.y, c11.w, r4.x\n"
    "cmp r4.x, -r4_abs.z, c8.w, r4.x\n"
    "cmp r4.x, -r4_abs.w, c8.w, r4.x\n"
    "cmp r3.y, -r2_abs.w, c8.w, r4.x\n"
    "mul_pp r2.w, r5.w, c10.w\n"
    "frc r4.x, r2.w\n"
    "add r2.w, r2.w, -r4.x\n"
    "cmp r4.x, -r4.x, c20.x, c20.y\n"
    "cmp r4.x, r5.w, c11.w, r4.x\n"
    "add r2.w, r2.w, r4.x\n"
    "add r4, r2.w, c19\n"
    "add r2.w, r2.w, c12.w\n"
    "cmp r4.x, -r4_abs.x, c20.y, c20.x\n"
    "cmp r4.x, -r4_abs.y, c11.w, r4.x\n"
    "cmp r4.x, -r4_abs.z, c8.w, r4.x\n"
    "cmp r4.x, -r4_abs.w, c8.w, r4.x\n"
    "cmp r3.z, -r2_abs.w, c8.w, r4.x\n"
    "mul_pp r2.w, r6.w, c10.w\n"
    "frc r4.x, r2.w\n"
    "add r2.w, r2.w, -r4.x\n"
    "cmp r4.x, -r4.x, c20.x, c20.y\n"
    "cmp r4.x, r6.w, c11.w, r4.x\n"
    "add r2.w, r2.w, r4.x\n"
    "add r4, r2.w, c19\n"
    "add r2.w, r2.w, c12.w\n"
    "cmp r4.x, -r4_abs.x, c20.y, c20.x\n"
    "cmp r4.x, -r4_abs.y, c11.w, r4.x\n"
    "cmp r4.x, -r4_abs.z, c8.w, r4.x\n"
    "cmp r4.x, -r4_abs.w, c8.w, r4.x\n"
    "cmp r3.w, -r2_abs.w, c8.w, r4.x\n"
    "mul r3, r3, r7\n"
    "cmp r0, -r0, c11.w, r3\n"
    "dp3 r2.w, c13, r2\n"
    "add r2.w, r2.w, r2.w\n"
    "mad r3.xyz, r2, -r2.w, c13\n"
    "mul r3.xyz, r3, c4.x\n"
    "dp3 r2.w, r3, r1\n"
    "cmp r3.xyz, r2.w, r3.zxyw, -r3.zxyw\n"
    "add r4.xy, r3.yzzw, v0\n"
    "texld r4, r4, s0\n"
    "dp3 r2.w, c14, r2\n"
    "add r2.w, r2.w, r2.w\n"
    "mad r5.xyz, r2, -r2.w, c14\n"
    "mul r5.xyz, r5, c4.x\n"
    "dp3 r2.w, r5, r1\n"
    "cmp r5.xyz, r2.w, r5, -r5\n"
    "mov r3.y, r5.z\n"
    "add r5.xy, r5, v0\n"
    "texld r5, r5, s0\n"
    "dp3 r2.w, c15, r2\n"
    "add r2.w, r2.w, r2.w\n"
    "mad r6.xyz, r2, -r2.w, c15\n"
    "mul r6.xyz, r6, c4.x\n"
    "dp3 r2.w, r6, r1\n"
    "cmp r6.xyz, r2.w, r6, -r6\n"
    "mov r3.z, r6.z\n"
    "add r6.xy, r6, v0\n"
    "texld r6, r6, s0\n"
    "dp3 r2.w, c16, r2\n"
    "add r2.w, r2.w, r2.w\n"
    "mad r2.xyz, r2, -r2.w, c16\n"
    "mul r2.xyz, r2, c4.x\n"
    "dp3 r1.x, r2, r1\n"
    "cmp r1.xyz, r1.x, r2, -r2\n"
    "mov r3.w, r1.z\n"
    "add r1.xy, r1, v0\n"
    "texld r2, r1, s0\n"
    "add r3, r3, c8.w\n"
    "dp3 r7.x, r4, c8\n"
    "dp3 r7.y, r5, c8\n"
    "dp3 r7.z, r6, c8\n"
    "dp3 r7.w, r2, c8\n"
    "mad r1, r7, -r1.w, r3\n"
    "mul r1, r1, c7.x\n"
    "rcp r3.x, r1.x\n"
    "rcp r3.y, r1.y\n"
    "rcp r3.z, r1.z\n"
    "rcp r3.w, r1.w\n"
    "mul_sat r3, r3, c6.x\n"
    "mul_pp r2.x, r4.w, c10.w\n"
    "frc r2.y, r2.x\n"
    "add r2.x, r2.x, -r2.y\n"
    "cmp r2.y, -r2.y, c20.x, c20.y\n"
    "cmp r2.y, r4.w, c11.w, r2.y\n"
    "add r2.x, r2.y, r2.x\n"
    "add r2.y, r2.x, c12.w\n"
    "add r4, r2.x, c19\n"
    "cmp r2.x, -r4_abs.x, c20.y, c20.x\n"
    "cmp r2.x, -r4_abs.y, c11.w, r2.x\n"
    "cmp r2.x, -r4_abs.z, c8.w, r2.x\n"
    "cmp r2.x, -r4_abs.w, c8.w, r2.x\n"
    "cmp r4.x, -r2_abs.y, c8.w, r2.x\n"
    "mul_pp r2.x, r5.w, c10.w\n"
    "frc r2.y, r2.x\n"
    "add r2.x, r2.x, -r2.y\n"
    "cmp r2.y, -r2.y, c20.x, c20.y\n"
    "cmp r2.y, r5.w, c11.w, r2.y\n"
    "add r2.x, r2.y, r2.x\n"
    "add r2.y, r2.x, c12.w\n"
    "add r5, r2.x, c19\n"
    "cmp r2.x, -r5_abs.x, c20.y, c20.x\n"
    "cmp r2.x, -r5_abs.y, c11.w, r2.x\n"
    "cmp r2.x, -r5_abs.z, c8.w, r2.x\n"
    "cmp r2.x, -r5_abs.w, c8.w, r2.x\n"
    "cmp r4.y, -r2_abs.y, c8.w, r2.x\n"
    "mul_pp r2.x, r6.w, c10.w\n"
    "frc r2.y, r2.x\n"
    "add r2.x, r2.x, -r2.y\n"
    "cmp r2.y, -r2.y, c20.x, c20.y\n"
    "cmp r2.y, r6.w, c11.w, r2.y\n"
    "add r2.x, r2.y, r2.x\n"
    "add r2.y, r2.x, c12.w\n"
    "add r5, r2.x, c19\n"
    "cmp r2.x, -r5_abs.x, c20.y, c20.x\n"
    "cmp r2.x, -r5_abs.y, c11.w, r2.x\n"
    "cmp r2.x, -r5_abs.z, c8.w, r2.x\n"
    "cmp r2.x, -r5_abs.w, c8.w, r2.x\n"
    "cmp r4.z, -r2_abs.y, c8.w, r2.x\n"
    "mul_pp r2.x, r2.w, c10.w\n"
    "frc r2.y, r2.x\n"
    "add r2.x, r2.x, -r2.y\n"
    "cmp r2.y, -r2.y, c20.x, c20.y\n"
    "cmp r2.y, r2.w, c11.w, r2.y\n"
    "add r2.x, r2.y, r2.x\n"
    "add r2.y, r2.x, c12.w\n"
    "add r5, r2.x, c19\n"
    "cmp r2.x, -r5_abs.x, c20.y, c20.x\n"
    "cmp r2.x, -r5_abs.y, c11.w, r2.x\n"
    "cmp r2.x, -r5_abs.z, c8.w, r2.x\n"
    "cmp r2.x, -r5_abs.w, c8.w, r2.x\n"
    "cmp r4.w, -r2_abs.y, c8.w, r2.x\n"
    "mul r2, r3, r4\n"
    "cmp r1, -r1, c11.w, r2\n"
    "add r0, r0, r1\n"
    "dp4 r0.x, r0, c8.w\n"
    "mul r0.x, r0.x, c20.z\n"
    "pow r1.x, r0.x, c5.x\n"
    "add r0.x, -r1.x, c8.w\n"
    "max oC0, r0.x, c11.w\n";

static const D3DVERTEXELEMENT9 decl1[] =
{
    { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, 
    { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};

static const float vtxdata1[] =
{
    -1.001f,    1.001f,     0.000f,     0.000f,     0.000f,
    -1.001f,   -0.999f,     0.000f,     0.000f,     1.000f,
     0.999f,    1.001f,     0.000f,     1.000f,     0.000f,
     0.999f,   -0.999f,     0.000f,     1.000f,     1.000f,
};

const char *ps2 =
    "ps_3_0\n"
    "def c3, 0.25, 0, 0, 0\n"
    "dcl_texcoord v0.xy\n"
    "dcl_2d s0\n"
    "add r0.xy, c0, v0\n"
    "texld r0, r0, s0\n"
    "texld r1, v0, s0\n"
    "add r0.x, r0.x, r1.x\n"
    "add r0.yz, c1.xxyw, v0.xxyw\n"
    "texld r1, r0.yzzw, s0\n"
    "add r0.x, r0.x, r1.x\n"
    "add r0.yz, c2.xxyw, v0.xxyw\n"
    "texld r1, r0.yzzw, s0\n"
    "add r0.x, r0.x, r1.x\n"
    "mul oC0, r0.x, c3.x\n";

static const float ps_const2[] =
{
    0.000f, -0.001f, 0.000f, 0.000f,
    0.000f,  0.001f, 0.000f, 0.000f,
    0.000f,  0.003f, 0.000f, 0.000f 
};

struct drawdata data[] =
{
    {
        decl1, NULL, vs1, NULL, ps1, NULL,
        {{NULL /* buffer1 */, 0, 20}},
        {{"tex0.dds", NULL, D3DTADDRESS_CLAMP}, {"tex1.dds", NULL, D3DTADDRESS_CLAMP}, {"tex2.dds", NULL, D3DTADDRESS_WRAP}},
        NULL /*dst surface, initialized later */,
        NULL, 0, /* vs const */
        ps_const1, 8,
        0, 2
    },
    {
        decl1, NULL, vs1, NULL, ps2, NULL,
        {{NULL /* buffer1 */, 0, 20}},
        {{NULL, NULL, D3DTADDRESS_WRAP}},
        NULL /*dst surface, initialized later */,
        NULL, 0, /* vs const */
        ps_const2, 3,
        0, 2
    }
};
const unsigned int numdraws = sizeof(data) / sizeof(*data);

HRESULT init_data(IDirect3DDevice9 *dev)
{
    HRESULT hr;
    void *buf_ptr;
    unsigned int i, j;
    ID3DXBuffer *code, *msg;
    IDirect3DSurface9 *surface = NULL;
    IDirect3DTexture9 *tex = NULL;

    hr = dev->CreateVertexBuffer(sizeof(vtxdata1), 0, 0, D3DPOOL_DEFAULT, &data[0].streams[0].buf, NULL);
    if (FAILED(hr))
        goto err;
    data[0].streams[0].buf->Lock(0, 0, &buf_ptr, 0);
    memcpy(buf_ptr, vtxdata1, sizeof(vtxdata1));
    data[0].streams[0].buf->Unlock();

    hr = dev->CreateTexture(1024, 768, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &tex, NULL);
    if (FAILED(hr))
        goto err;
    tex->GetSurfaceLevel(0, &surface);
    data[1].textures[0].texture = tex;
    data[0].target = surface;
    tex = NULL;
    surface = NULL;

    data[1].streams[0].buf = data[0].streams[0].buf;
    data[1].streams[0].buf->AddRef();
    hr = dev->CreateTexture(1024, 768, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &tex, NULL);
    if (FAILED(hr))
        goto err;
    tex->GetSurfaceLevel(0, &surface);
    //data[2].textures[0].texture = tex;
    data[1].target = surface;
    tex = NULL;
    surface = NULL;

    for (i = 0; i < 3; i++)
    {
        hr = D3DXCreateTextureFromFileEx(dev, data[0].textures[i].filename, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
                D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL,
                (IDirect3DTexture9 **)&data[0].textures[i].texture);
        if (FAILED(hr))
            goto err;
    }

    for (i = 0; i < sizeof(data) / sizeof(*data); i++)
    {
        /*** Vertex declaration ***/
        for (j = 0; j < i; j++)
        {
            if (data[i].decl_data == data[j].decl_data)
            {
                data[i].decl = data[j].decl;
                data[i].decl->AddRef();
            }
        }
        if (!data[i].decl)
        {
            hr = dev->CreateVertexDeclaration(data[i].decl_data, &data[i].decl);
            if (FAILED(hr))
                goto err;
        }

        /*** Vertex Shader ***/
        for (j = 0; j < i; j++)
        {
            if (data[i].vs_txt == data[j].vs_txt)
            {
                data[i].vs = data[j].vs;
                data[i].vs->AddRef();
            }
        }
        if (!data[i].vs)
        {
	        hr = D3DXAssembleShader(data[i].vs_txt, strlen(data[i].vs_txt), NULL, NULL, 0, &code, &msg);
	        if (FAILED(hr))
	        {
		        printf("vs msgs: %s\n", msg->GetBufferPointer());
                msg->Release();
		        goto err;
	        }

	        hr = dev->CreateVertexShader((DWORD *)code->GetBufferPointer(), &data[i].vs);
            code->Release();
            if (msg) msg->Release();
            if (FAILED(hr))
                goto err;
        }

        /*** Pixel Shader ***/
        for (j = 0; j < i; j++)
        {
            if (data[i].ps_txt == data[j].ps_txt)
            {
                data[i].ps = data[j].ps;
                data[i].ps->AddRef();
            }
        }
        if (!data[i].ps)
        {
	        hr = D3DXAssembleShader(data[i].ps_txt, strlen(data[i].ps_txt), NULL, NULL, 0, &code, &msg);
	        if (FAILED(hr))
	        {
		        printf("ps msgs: %s\n", msg->GetBufferPointer());
                msg->Release();
		        goto err;
	        }

	        hr = dev->CreatePixelShader((DWORD *)code->GetBufferPointer(), &data[i].ps);
            code->Release();
            if (FAILED(hr))
                goto err;
        }
    }

    return D3D_OK;

err:
    for (i = 0; i < sizeof(data) / sizeof(*data); i++)
    {
        if (data[i].decl)
            data[i].decl->Release();
        if (data[i].vs)
            data[i].vs->Release();
        if (data[i].ps)
            data[i].ps->Release();
        if (data[i].target)
            data[i].target->Release();
        for (j = 0; j < sizeof(data[i].streams) / sizeof(*data[i].streams); j++)
        {
            if (data[i].streams[j].buf)
                data[i].streams[j].buf->Release();
        }
    }
    if (tex) tex->Release();
    if (surface) surface->Release();
    return hr;
}
