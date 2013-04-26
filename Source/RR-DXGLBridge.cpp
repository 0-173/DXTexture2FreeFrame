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

#include <FFGL.h>
#include <FFGLLib.h>
#include "dxConnector.h"
#include "RR-DXGLBridge.h"

#define FFPARAM_SharingName		(0)
#define FFPARAM_Reload			(1)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
	RRDXGLBridge::CreateInstance,	// Create method
	"DXBR",								// Plugin unique ID											
	"DX Texture (RR)",					// Plugin name											
	1,						   			// API major version number 													
	000,								  // API minor version number	
	1,										// Plugin major version number
	000,									// Plugin minor version number
	FF_SOURCE,						// Plugin type
	"DirectX-Freeframe Texture Bridge",	// Plugin description
	"by Elio / www.r-revue.de" // About
);


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

RRDXGLBridge::RRDXGLBridge()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_maxCoordsLocation(-1)
{
	// Input properties
	SetMinInputs(1);
	SetMaxInputs(1);

	// Parameters
	SetParamInfo(FFPARAM_SharingName, "Sharing Name", FF_TYPE_TEXT, "myApplication/mySource");
	strcpy( m_SharedMemoryName, "myApplication/mySource" );
	SetParamInfo(FFPARAM_Reload, "Update", FF_TYPE_EVENT, false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD RRDXGLBridge::InitGL(const FFGLViewportStruct *vp)
{
	HWND hWnd = GetForegroundWindow(); // we don't have the Window Handle until now :(

	//initialize gl extensions and
	//make sure required features are supported
	m_extensions.Initialize();
  
	if ( !m_dxConnector.init(hWnd) ) {
		return FF_FAIL;
	}
		// DirectX device initialization needs the window to be redrawn (creates black areas)
	SendMessage( hWnd, WM_PAINT, NULL, NULL );

	return FF_SUCCESS;
}

DWORD RRDXGLBridge::DeInitGL()
{
	/* this causes death. either it must be used another way or there's no need to unregister
	wglDXUnregisterObjectNV(pDevice, m_TextureFromDX9Handle);*/
	m_dxConnector.cleanup();

	return FF_SUCCESS;
}

DWORD RRDXGLBridge::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	if (pGL->numInputTextures<1) return FF_FAIL;

	if (pGL->inputTextures[0]==NULL) return FF_FAIL;
  
	FFGLTextureStruct &Texture = *(pGL->inputTextures[0]);

	// lock and bind dx texture
	BOOL ret = wglDXLockObjectsNV(m_dxConnector.m_InteropHandle, 1, &m_dxConnector.m_glTextureHandle);
	glBindTexture(GL_TEXTURE_2D, m_dxConnector.m_glTextureName);
	//enable texturemapping
	glEnable(GL_TEXTURE_2D);

	//get the max s,t that correspond to the 
	//width,height of the used portion of the allocated texture space
	FFGLTexCoords maxCoords = GetMaxGLTexCoords(Texture);

	//modulate texture colors with white (just show
	//the texture colors as they are)
	glColor4f(1.f, 1.f, 1.f, 1.f);

	glBegin(GL_QUADS);
		//lower left
		glTexCoord2d(0.0, maxCoords.t);
		glVertex2f(-1,-1);

		//upper left
		glTexCoord2d(0.0, 0.0);
		glVertex2f(-1,1);

		//upper right
		glTexCoord2d(maxCoords.s, 0.0);
		glVertex2f(1,1);

		//lower right
		glTexCoord2d(maxCoords.s, maxCoords.t);
		glVertex2f(1,-1);
	glEnd();

	// unbind the drawn texture
	glBindTexture(GL_TEXTURE_2D, 0);
	// unlock dx object
	ret = wglDXUnlockObjectsNV(m_dxConnector.m_InteropHandle, 1, &m_dxConnector.m_glTextureHandle);
  
	//disable texturemapping
	glDisable(GL_TEXTURE_2D);
  
	return FF_SUCCESS;
}

DWORD RRDXGLBridge::GetParameter(DWORD dwIndex)
{
	DWORD dwRet;

	switch (dwIndex) {

	case FFPARAM_SharingName:
	    dwRet = (DWORD) m_SharedMemoryName;
		return dwRet;
	default:
		return FF_FAIL;
	}
}

DWORD RRDXGLBridge::SetParameter(const SetParameterStruct* pParam)
{
	if (pParam != NULL) {
		
		switch (pParam->ParameterNumber) {

		case FFPARAM_SharingName:
			strcpy( m_SharedMemoryName, (char*) pParam->NewParameterValue);
			m_dxConnector.setSharedMemoryName(m_SharedMemoryName);
			break;
		case FFPARAM_Reload:
			if ( pParam->NewParameterValue ) {
				m_dxConnector.Reload();
			}
			break;

		default:
			return FF_FAIL;
		}

		return FF_SUCCESS;
	
	}

	return FF_FAIL;
}
