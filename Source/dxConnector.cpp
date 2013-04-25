#include "dxconnector.h"
#include <string>

PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = NULL;
PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = NULL;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV = NULL;
PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = NULL;
PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = NULL;
PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = NULL;
PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = NULL;

DXGLConnector::DXGLConnector() {
	IDirect3D9Ex * m_pD3D = NULL;
	IDirect3DDevice9Ex * m_pDevice = NULL;
	LPDIRECT3DTEXTURE9 m_dxTexture = NULL;
	m_glTextureHandle = NULL;
	m_glTextureName = 0;
	m_hWnd = NULL;
	m_bInitialized = FALSE;
	strcpy( m_shardMemoryName, "" );
	getNvExt();
}

DXGLConnector::~DXGLConnector() {
	m_bInitialized = FALSE;
}

// this function initializes and prepares Direct3D
void DXGLConnector::init(HWND hWnd)
{
    Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
		// this seems to be quite a dummy thing because we use directx only for accessing the handle
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		// some dummy resolution - we don't render anything
    d3dpp.BackBufferWidth = 10;
    d3dpp.BackBufferHeight = 10;
	
	// attention: changed this from device9ex to device9 (perhaps it needs to be changed back, but i want to have same code as in dx_interop sample code)

    // create a device class using this information and the info from the d3dpp stuct
    m_pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED,
                      &d3dpp,
					  NULL,
                      &m_pDevice);
		
	m_InteropHandle = wglDXOpenDeviceNV(m_pDevice);
		// prepare gl texture
	glGenTextures(1, &m_glTextureName);

		// directly connect to texture if possible
	connectToTexture();
	m_bInitialized = TRUE;
}

// this is the function that cleans up Direct3D and COM
void DXGLConnector::cleanup()
{
	if (m_glTextureName) {
		glDeleteTextures(1, &m_glTextureName);
		m_glTextureName = 0;
	}
	if ( m_glTextureHandle != NULL ) { // already a texture connected => unregister interop
		wglDXUnregisterObjectNV(m_InteropHandle, m_glTextureHandle);
		m_glTextureHandle = NULL;
	}

	wglDXCloseDeviceNV(m_InteropHandle);

    m_pDevice->Release();    // close and release the 3D device
    m_pD3D->Release();    // close and release Direct3D
	m_bInitialized = FALSE;
}

void DXGLConnector::setSharedMemoryName(char* sharedMemoryName) {
	if ( strcmp(sharedMemoryName, m_shardMemoryName) == 0 ) {
		return;
	}
	strcpy( m_shardMemoryName, sharedMemoryName );
	if ( m_bInitialized ) { // directly connect to texture (if already initialized)
		connectToTexture();
	}
}

BOOL DXGLConnector::Reload() {
	if ( m_bInitialized ) { // directly connect to texture (if already initialized)
		return connectToTexture();
	}
	return FALSE;
}

BOOL DXGLConnector::connectToTexture() {
	if ( m_glTextureHandle != NULL ) { // already a texture connected => unregister interop
		wglDXUnregisterObjectNV(m_InteropHandle, m_glTextureHandle);
		m_glTextureHandle = NULL;
	}

	if ( !getSharedTextureInfo(m_shardMemoryName) ) {  // error accessing shared memory texture info
		return FALSE;
	}

	HANDLE textureShareHandle = (HANDLE) m_TextureInfo.shareHandle;
	HRESULT res = m_pDevice->CreateTexture(m_TextureInfo.width,m_TextureInfo.height,1,D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_dxTexture, &textureShareHandle );
		// USAGE may also be D3DUSAGE_DYNAMIC and pay attention to format and resolution!!!
	if ( res != D3D_OK ) {
		return FALSE;
	}

		// prepare shared resource
	if (!wglDXSetResourceShareHandleNV(m_dxTexture, textureShareHandle) ) {
			// this is not only a non-accessible share-handle, something worse
		MessageBox(NULL, "wglDXSetResourceShareHandleNV() failed.", "Error", 0);
		return FALSE;
	}

		// register for interop and associate with dx texture
	m_glTextureHandle = wglDXRegisterObjectNV(m_InteropHandle, m_dxTexture,
		m_glTextureName,
		GL_TEXTURE_2D,
		WGL_ACCESS_READ_ONLY_NV);

	return TRUE;
}

/**
* Load the Nvidia-Extensions dynamically
*/
BOOL DXGLConnector::getNvExt() 
{
	wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)wglGetProcAddress("wglDXOpenDeviceNV");
	if(wglDXOpenDeviceNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXOpenDeviceNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	if(wglDXRegisterObjectNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXRegisterObjectNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");
	if(wglDXUnregisterObjectNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXRegisterObjectNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXSetResourceShareHandleNV = (PFNWGLDXSETRESOURCESHAREHANDLENVPROC)wglGetProcAddress("wglDXSetResourceShareHandleNV");
	if(wglDXSetResourceShareHandleNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXSetResourceShareHandleNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
	if(wglDXLockObjectsNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXLockObjectsNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");
	if(wglDXUnlockObjectsNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXUnlockObjectsNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}
	wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)wglGetProcAddress("wglDXCloseDeviceNV");
	if(wglDXUnlockObjectsNV == NULL)
	{
		MessageBox(m_hWnd, "wglDXCloseDeviceNV ext is not supported by your GPU.", "Error", 0);
		return FALSE;
	}

	return TRUE;
}

BOOL DXGLConnector::getSharedTextureInfo(char* sharedMemoryName) {
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = OpenFileMapping(
					FILE_MAP_READ,   // read/write access
					FALSE,                 // do not inherit the name
					sharedMemoryName);               // name of mapping object

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

	memcpy( &m_TextureInfo, pBuf, sizeof(DX9SharedTextureInfo) );
	
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	return TRUE;
}