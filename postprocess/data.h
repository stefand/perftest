#include "d3d9.h"

struct streamdata
{
    IDirect3DVertexBuffer9 *buf;
    unsigned int offset, stride;
};

struct texdata
{
    const char *filename;
    IDirect3DBaseTexture9 *texture;
    D3DTEXTUREADDRESS address;
};

struct drawdata
{
    const D3DVERTEXELEMENT9 *decl_data;
    IDirect3DVertexDeclaration9 *decl;
    const char *vs_txt;
    IDirect3DVertexShader9 *vs;
    const char *ps_txt;
    IDirect3DPixelShader9 *ps;
    struct streamdata streams[16];
    struct texdata textures[16];
    IDirect3DSurface9 *target;
    const float *vs_const;
    unsigned int vs_const_len;
    const float *ps_const;
    unsigned int ps_const_len;
    unsigned int starvtx, primitives;
};

HRESULT init_data(IDirect3DDevice9 *device);
extern struct drawdata data[];
extern const unsigned int numdraws;