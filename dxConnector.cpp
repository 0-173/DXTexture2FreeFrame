#include "dxconnector.h"
#include <string>

PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = NULL;
PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = NULL;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV = NULL;
PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = NULL;
PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = NULL;
PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = NULL;
PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = NULL;

// this function initializes and prepares Direct3D for use
void initD3D(IDirect3D9Ex** ppD3D, IDirect3DDevice9Ex** ppDevice, HANDLE* pInteropHandle, GLuint* pTextureName, HWND hWnd, int width, int height)
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
		// prepare gl texture
	glGenTextures(1, pTextureName);
}

// this is the function that cleans up Direct3D and COM
void cleanD3D(IDirect3D9Ex* pD3D, IDirect3DDevice9Ex* pDevice, HANDLE* pInteropHandle)
{
	wglDXCloseDeviceNV(*pInteropHandle);

    pDevice->Release();    // close and release the 3D device
    pD3D->Release();    // close and release Direct3D
}


BOOL load_texture(IDirect3DDevice9Ex* pDevice, HANDLE interopHandle, HANDLE* pTextureHandle, GLuint TextureName) {
	LPDIRECT3DTEXTURE9 texture;
	DX9SharedTextureInfo textureInfo;
	getSharedTextureInfo(&textureInfo);
	HANDLE textureShareHandle = (HANDLE) textureInfo.shareHandle;
	HRESULT res = pDevice->CreateTexture(textureInfo.width,textureInfo.height,1,D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, &textureShareHandle );
		// USAGE may also be D3DUSAGE_DYNAMIC and pay attention to format and resolution!!!
	if ( res != D3D_OK ) {
		return FALSE;
	}


	HRESULT ok = D3D_OK;
	HRESULT invalidcall = D3DERR_INVALIDCALL;

		// prepare shared resource
	if (!wglDXSetResourceShareHandleNV(texture, textureShareHandle) ) {
		MessageBox(NULL, "wglDXSetResourceShareHandleNV() failed.", "Error", 0);
		return FALSE;
	}

		// register for interop and associate with dx texture
	*pTextureHandle = wglDXRegisterObjectNV(interopHandle, texture,
		TextureName,
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

BOOL getSharedTextureInfo(DX9SharedTextureInfo* info) {
	TCHAR szName[]=TEXT("vvvvToResolume/MainRenderer");
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = OpenFileMapping(
					FILE_MAP_READ,   // read/write access
					FALSE,                 // do not inherit the name
					szName);               // name of mapping object

	if (hMapFile == NULL) {
		// no texture to share
		return FALSE;
	}

	pBuf = (LPTSTR) MapViewOfFile(hMapFile, // handle to map object
				FILE_MAP_READ,  // read/write permission
				0,
				0,
				sizeof(DX9SharedTextureInfo) );

	if (pBuf == NULL)
	{
		MessageBox(NULL, "Could not map view of file.", "Error", 0), 
		CloseHandle(hMapFile);
		return FALSE;
	}

	memcpy( info, pBuf, sizeof(DX9SharedTextureInfo) );
	
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	return TRUE;
}