/**
	Copyright 2013 Elio <elio@r-revue.de>
	
	
	This file is part of RR-DXBridge

    RR-DXBridge is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RR-DXBridge is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with RR-DXBridge.  If not, see <http://www.gnu.org/licenses/>.

    Diese Datei ist Teil von RR-DXBridge.

    RR-DXBridge ist Freie Software: Sie können es unter den Bedingungen
    der GNU Lesser General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
    veröffentlichten Version, weiterverbreiten und/oder modifizieren.

    RR-DXBridge wird in der Hoffnung, dass es nützlich sein wird, aber
    OHNE JEDE GEWÄHELEISTUNG, bereitgestellt; sogar ohne die implizite
    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
    Siehe die GNU Lesser General Public License für weitere Details.

    Sie sollten eine Kopie der GNU Lesser General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
 */

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

class DXGLConnector {

	public:

		// parent window
		HWND m_hWnd;
		// texture info struct from sending application
		DX9SharedTextureInfo m_TextureInfo;

		// DIRECTX Connections
		IDirect3D9Ex * m_pD3D;
		IDirect3DDevice9Ex * m_pDevice;
		LPDIRECT3DTEXTURE9 m_dxTexture;

		// GL Connections
		GLuint m_glTextureName;
		HANDLE m_glTextureHandle;

		// the DX/GL interop device's handle
		HANDLE m_InteropHandle;

		DXGLConnector();
		~DXGLConnector();

		    // sets up and initializes Direct3D
		void init(HWND hWnd);
			// deinit Direct3D
		void cleanup();

		BOOL connectToTexture();
		BOOL Reload();
		void setSharedMemoryName(char* sharedMemoryName);
	protected:
		BOOL m_bInitialized;
		char m_shardMemoryName[256];

		BOOL getSharedTextureInfo(char* sharedMemoryName);
		BOOL getNvExt();
};

#endif