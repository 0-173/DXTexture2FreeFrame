#include "dxconnector.h"

PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = NULL;
PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = NULL;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV = NULL;
PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = NULL;
PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = NULL;
PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = NULL;
PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = NULL;

// this function initializes and prepares Direct3D for use
void initD3D(IDirect3D9Ex** ppD3D, IDirect3DDevice9Ex** ppDevice, HANDLE* pInteropHandle, HWND hWnd, int width, int height)
{
    Direct3DCreate9Ex(D3D_SDK_VERSION, ppD3D);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
//	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth = width;
    d3dpp.BackBufferHeight = height;
	
	// attention: changed this from device9ex to device9 (perhaps it needs to be changed back, but i want to have same code as in dx_interop sample code)

    // create a device class using this information and the info from the d3dpp stuct
    (*ppD3D)->CreateDeviceEx(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED,
                      &d3dpp,
					  NULL,
                      ppDevice);
	// register the Direct3D device with GL
	*pInteropHandle = wglDXOpenDeviceNV(*ppDevice);
}

// this is the function that cleans up Direct3D and COM
void cleanD3D(IDirect3D9Ex* pD3D, IDirect3DDevice9Ex* pDevice, HANDLE* pInteropHandle)
{
	wglDXCloseDeviceNV(*pInteropHandle);

    pDevice->Release();    // close and release the 3D device
    pD3D->Release();    // close and release Direct3D
}


BOOL load_texture(IDirect3DDevice9Ex* pDevice, HANDLE interopHandle, HANDLE* pTextureHandle, GLuint* pTextureName) {
	HANDLE textureShareHandle = (HANDLE) 3221232642;
	LPDIRECT3DTEXTURE9 texture;
	HRESULT res = pDevice->CreateTexture(256,256,1,D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, &textureShareHandle );
	// USAGE may also be D3DUSAGE_DYNAMIC and pay attention to format and resolution!!!
//	HRESULT res2 = d3ddev->CreateRenderTarget(1920,1080,D3DFMT_X8R8G8B8,D3DMULTISAMPLE_NONE,0,false, &textureShareHandle );


	HRESULT ok = D3D_OK;
	HRESULT invalidcall = D3DERR_INVALIDCALL;

		// prepare shared resource
	if (!wglDXSetResourceShareHandleNV(texture, textureShareHandle) ) {
		MessageBox(NULL, "wglDXSetResourceShareHandleNV() failed.", "Error", 0);
		return FALSE;
	}

		// prepare gl texture
	glGenTextures(1, pTextureName);
		// register for interop and associate with dx texture
	*pTextureHandle = wglDXRegisterObjectNV(interopHandle, texture,
		*pTextureName,
		GL_TEXTURE_2D,
		WGL_ACCESS_READ_ONLY_NV);

	return TRUE;
}

/**
* Load the Nvidia-Extensions dynamically
*/
BOOL getNvExt( HWND hWnd ) 
{
	wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)wglGetProcAddress("wglDXOpenDeviceNV");
	if(wglDXOpenDeviceNV == NULL)
	{
		MessageBox(hWnd, "wglDXOpenDeviceNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	if(wglDXRegisterObjectNV == NULL)
	{
		MessageBox(hWnd, "wglDXRegisterObjectNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");
	if(wglDXUnregisterObjectNV == NULL)
	{
		MessageBox(hWnd, "wglDXRegisterObjectNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXSetResourceShareHandleNV = (PFNWGLDXSETRESOURCESHAREHANDLENVPROC)wglGetProcAddress("wglDXSetResourceShareHandleNV");
	if(wglDXSetResourceShareHandleNV == NULL)
	{
		MessageBox(hWnd, "wglDXSetResourceShareHandleNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
	if(wglDXLockObjectsNV == NULL)
	{
		MessageBox(hWnd, "wglDXLockObjectsNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");
	if(wglDXUnlockObjectsNV == NULL)
	{
		MessageBox(hWnd, "wglDXUnlockObjectsNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)wglGetProcAddress("wglDXCloseDeviceNV");
	if(wglDXUnlockObjectsNV == NULL)
	{
		MessageBox(hWnd, "wglDXCloseDeviceNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}

	return TRUE;
}
