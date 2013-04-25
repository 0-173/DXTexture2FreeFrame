#ifndef DxConnector_H
#define DxConnector_H

#include <windowsx.h>
#include <d3d9.h>
#include <gl/gl.h>
#include <wingdi.h>

#pragma comment (lib, "d3d9.lib")


//-----------------------------------------------------------------------------
// GL consts that are needed and aren't present in GL.h
//-----------------------------------------------------------------------------
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define WGL_ACCESS_READ_ONLY_NV 0x0000
#define WGL_ACCESS_READ_WRITE_NV 0x0001
#define WGL_ACCESS_WRITE_DISCARD_NV 0x0002

//-----------------------------------------------------------------------------
// GL ext that are needed to demo the issue
//-----------------------------------------------------------------------------
typedef HANDLE (WINAPI * PFNWGLDXOPENDEVICENVPROC) (void* dxDevice);
extern PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV;
typedef BOOL (WINAPI * PFNWGLDXCLOSEDEVICENVPROC) (HANDLE hDevice);
extern PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV;
typedef HANDLE (WINAPI * PFNWGLDXREGISTEROBJECTNVPROC) (HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access);
extern PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV;
typedef BOOL (WINAPI * PFNWGLDXUNREGISTEROBJECTNVPROC) (HANDLE hDevice, HANDLE hObject);
extern PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV;
typedef BOOL (WINAPI * PFNWGLDXSETRESOURCESHAREHANDLENVPROC) (void *dxResource, HANDLE shareHandle);
extern PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV;
typedef BOOL (WINAPI * PFNWGLDXLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE *hObjects);
extern PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV;
typedef BOOL (WINAPI * PFNWGLDXUNLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE *hObjects);
extern PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV;

struct DX9SharedTextureInfo {
	UINT16 width;
	UINT16 height;
	HANDLE shareHandle;
};

void initD3D(IDirect3D9Ex** ppD3D, IDirect3DDevice9Ex** ppDevice, HANDLE* pInteropHandle, GLuint* pTextureName, HWND hWnd, int width, int height);    // sets up and initializes Direct3D
void cleanD3D(IDirect3D9Ex* pD3D, IDirect3DDevice9Ex* pDevice, HANDLE* pInteropHandle);
BOOL getSharedTextureInfo(DX9SharedTextureInfo* info);
BOOL load_texture(IDirect3DDevice9Ex* pDevice, HANDLE interopHandle, HANDLE* pTextureHandle, GLuint TextureName);
BOOL getNvExt( HWND hWnd );

#endif