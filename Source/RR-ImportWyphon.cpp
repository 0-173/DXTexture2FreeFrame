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
#include <d3d9.h>
#include "../../../Wyphon/Wyphon/Wyphon.h"
#include "RR-ImportWyphon.h"

#define FFPARAM_WyphonApplicationName	(0)
#define FFPARAM_WyphonTextureDescription		(1)
#define FFPARAM_Reload					(2)

using namespace Wyphon;

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
	RRImportWyphon::CreateInstance,	// Create method
	"WYIM",								// Plugin unique ID											
	"Wyphon Import (RR)",					// Plugin name											
	1,						   			// API major version number 													
	000,								  // API minor version number	
	1,										// Plugin major version number
	000,									// Plugin minor version number
	FF_SOURCE,						// Plugin type
	"Texture Bridge",	// Plugin description
	"by Elio / www.r-revue.de" // About
);


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

RRImportWyphon::RRImportWyphon()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_maxCoordsLocation(-1)
{
	// Input properties
	SetMinInputs(3);
	SetMaxInputs(3);

	// Parameters
	SetParamInfo(FFPARAM_WyphonApplicationName, TEXT("Application Name"), FF_TYPE_TEXT, TEXT("myApplication"));
	SetParamInfo(FFPARAM_WyphonTextureDescription, TEXT("Sharing Name"), FF_TYPE_TEXT, TEXT("myTexture"));
	strcpy_s( m_WyphonApplicationName, TEXT("myApplication") );
	strcpy_s( m_WyphonTextureDescription, TEXT("myTexture") );
	SetParamInfo(FFPARAM_Reload, TEXT("Update"), FF_TYPE_EVENT, false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD RRImportWyphon::InitGL(const FFGLViewportStruct *vp)
{
	HWND hWnd = GetForegroundWindow(); // we don't have the Window Handle until now :(

	//initialize gl extensions and
	//make sure required features are supported
	m_extensions.Initialize();

	HANDLE g_hWyphonPartner = CreateWyphonPartner(
		TEXT("FFGL Wyphon Import (RR)"),
		this, // link to instance of this object (for callbacks)
		/*WyphonPartnerJoinedCALLBACK*/NULL,
		/*WyphonPartnerLeftCALLBACK*/NULL,
		TextureSharingStartedCALLBACK,
		TextureSharingStoppedCALLBACK
	);
	UnsetTextureData();

	if ( WyphonUtils::InitDX9Ex() != S_OK) {
		return FF_FAIL;
	}
	if ( WyphonUtils::InitGLDXInterop() != S_OK ) {
		return FF_FAIL;
	}
		// DirectX device initialization needs the window to be redrawn (creates black areas). This helps in some cases
	SendMessage( hWnd, WM_PAINT, NULL, NULL );

	return FF_SUCCESS;
}

DWORD RRImportWyphon::DeInitGL()
{
	/* this causes death. either it must be used another way or there's no need to unregister
	wglDXUnregisterObjectNV(pDevice, m_TextureFromDX9Handle);*/
	
	WyphonUtils::ReleaseDX9Ex();
	WyphonUtils::ReleaseGLDXInterop();

	return FF_SUCCESS;
}

DWORD RRImportWyphon::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	UpdateTexture();
	// lock and bind the gl-dx linked texture
	WyphonUtils::LockGLTexture(m_glTextureHandle);
	glBindTexture(GL_TEXTURE_2D, m_glTextureName);
	//enable texturemapping
	glEnable(GL_TEXTURE_2D);

	//modulate texture colors with white (just show
	//the texture colors as they are)
	glColor4f(1.f, 1.f, 1.f, 1.f);

	glBegin(GL_QUADS);
		//lower left
		glTexCoord2d(0.0, 1.0);
		glVertex2f(-1,-1);

		//upper left
		glTexCoord2d(0.0, 0.0);
		glVertex2f(-1,1);

		//upper right
		glTexCoord2d(1.0, 0.0);
		glVertex2f(1,1);

		//lower right
		glTexCoord2d(1.0, 1.0);
		glVertex2f(1,-1);
	glEnd();

	// unbind the drawn texture
	glBindTexture(GL_TEXTURE_2D, 0);
	// unlock dx object
	WyphonUtils::UnlockGLTexture(m_glTextureHandle);
  
	//disable texturemapping
	glDisable(GL_TEXTURE_2D);
  
	return FF_SUCCESS;
}

DWORD RRImportWyphon::GetParameter(DWORD dwIndex)
{
	DWORD dwRet;

	switch (dwIndex) {

	case FFPARAM_WyphonApplicationName:
	    dwRet = (DWORD) m_WyphonApplicationName;
		return dwRet;
	case FFPARAM_WyphonTextureDescription:
	    dwRet = (DWORD) m_WyphonTextureDescription;
		return dwRet;
	default:
		return FF_FAIL;
	}
}

DWORD RRImportWyphon::SetParameter(const SetParameterStruct* pParam)
{
	if (pParam != NULL) {
		
		switch (pParam->ParameterNumber) {

		case FFPARAM_WyphonApplicationName:
			strcpy_s( m_WyphonApplicationName, (char*) pParam->NewParameterValue);
//			m_WyphonConnector.setSharedMemoryName(m_WyphonApplicationName);
			break;
		case FFPARAM_WyphonTextureDescription:
			strcpy_s( m_WyphonTextureDescription, (char*) pParam->NewParameterValue);
//			m_WyphonConnector.setSharedMemoryName(m_WyphonTextureDescription);
			break;
		case FFPARAM_Reload:
			if ( pParam->NewParameterValue ) {
//				m_WyphonConnector.Reload();
			}
			break;

		default:
			return FF_FAIL;
		}

		return FF_SUCCESS;
	
	}

	return FF_FAIL;
}

DWORD RRImportWyphon::UpdateTexture() {
	if ( m_bTextureNeedsUpdate ) {
		WyphonUtils::CreateLinkedGLTexture(m_WyphonTextureInfo.width, m_WyphonTextureInfo.height, m_WyphonTextureInfo.usage, m_WyphonTextureInfo.format, (HANDLE) m_WyphonTextureInfo.hSharedTexture, m_glTextureName, m_glTextureHandle);
		m_bTextureNeedsUpdate = FALSE;
	}
	return FF_SUCCESS;
}

/**
 * Just store the information about the texture
 * The call to WyphonUtils must be made by this thread and not by the callback function.
 */
DWORD RRImportWyphon::SetTextureData(WyphonD3DTextureInfo WyphonTextureInfo) {
	m_WyphonTextureInfo = WyphonTextureInfo;
	m_bTextureNeedsUpdate = TRUE;

	return FF_SUCCESS;
}

DWORD RRImportWyphon::UnsetTextureData() {
	ZeroMemory(&m_WyphonTextureInfo, sizeof(m_WyphonTextureInfo));
	m_bTextureNeedsUpdate = FALSE;
	return FF_SUCCESS;
}


void TextureSharingStartedCALLBACK( HANDLE wyphonPartnerHandle, unsigned __int32 sendingPartnerId, HANDLE sharedTextureHandle, unsigned __int32 width, unsigned __int32 height, DWORD format, DWORD usage, LPTSTR description, void * customData ) {
	RRImportWyphon* pObj = (RRImportWyphon*) customData;
	
	WyphonD3DTextureInfo WyphonTextureInfo;
	WyphonTextureInfo.format = format;
	WyphonTextureInfo.width = width;
	WyphonTextureInfo.height = height;
	WyphonTextureInfo.hSharedTexture = (unsigned __int32) sharedTextureHandle;
	WyphonTextureInfo.partnerId = sendingPartnerId;
	WyphonTextureInfo.usage = usage;
	//WyphonTextureInfo.description = description;
	
	pObj->SetTextureData(WyphonTextureInfo);
}

void TextureSharingStoppedCALLBACK( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD format, DWORD usage, LPTSTR description, void * customData ) {
	// close GlTextureName
	// close GlTextureHandle
	RRImportWyphon* pObj = (RRImportWyphon*) customData;
}

void WyphonPartnerJoinedCALLBACK( HANDLE wyphonPartnerHandle, unsigned __int32 sendingWyphonPartnerId, LPCTSTR description, void * customData ) {
}

void WyphonPartnerLeftCALLBACK( HANDLE wyphonPartnerHandle, unsigned __int32 sendingWyphonPartnerId, void * customData ) {
}

