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
#include "RR-ExportWyphon.h"

#define FFPARAM_WyphonApplicationName	(0)
#define FFPARAM_WyphonTextureDescription		(1)

using namespace Wyphon;
using namespace WyphonUtils;

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
	RRExportWyphon::CreateInstance,	// Create method
	"WYEX",								// Plugin unique ID											
	"AAAWyphonEx (RR)",					// Plugin name											
	1,						   			// API major version number 													
	000,								  // API minor version number	
	1,										// Plugin major version number
	000,									// Plugin minor version number
	FF_EFFECT,						// Plugin type
	"Wyphon Texture Bridge",	// Plugin description
	"by Elio / www.r-revue.de" // About
);


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

RRExportWyphon::RRExportWyphon()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_maxCoordsLocation(-1)
{
	// Input properties
	SetMinInputs(1);
	SetMaxInputs(1);

	// Parameters

	strcpy_s( m_WyphonApplicationName, "myApplication" );
	strcpy_s( m_WyphonTextureDescription, "myTexture" );
		// also take care of the wchar_t versions
	mbstowcs( m_WyphonApplicationNameT, m_WyphonApplicationName, WYPHON_MAX_DESCRIPTION_LENGTH+1 );
	mbstowcs( m_WyphonTextureDescriptionT, m_WyphonTextureDescription, WYPHON_MAX_DESCRIPTION_LENGTH+1 );

	SetParamInfo(FFPARAM_WyphonApplicationName, "Application Name", FF_TYPE_TEXT, m_WyphonApplicationName);
	SetParamInfo(FFPARAM_WyphonTextureDescription, "Sharing Name", FF_TYPE_TEXT, m_WyphonTextureDescription);

	m_hWyphonPartner = NULL;
	m_glTextureHandle = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD RRExportWyphon::InitGL(const FFGLViewportStruct *vp)
{
//	HWND hWnd = GetForegroundWindow(); // we don't have the Window Handle until now :(

	//initialize gl extensions and
	//make sure required features are supported
	m_extensions.Initialize();

	// generate framebuffer object for copying textures
	m_extensions.glGenFramebuffersEXT(1, &m_fbo);
	m_width = vp->width;
	m_height = vp->height;

	m_shareHandle = NULL;

	m_hWyphonPartner = CreateWyphonPartner(
		m_WyphonApplicationNameT,
		this, // link to instance of this object (for callbacks)
		NULL,NULL,NULL,NULL /* no callbacks */
	);
	
	m_hWyphonDevice = WyphonUtils::InitDevice();
	// DirectX device initialization needs the window to be redrawn (creates black areas)
//	SendMessage( hWnd, WM_PAINT, NULL, NULL );

	if ( GenerateTexture() ) {
		ShareD3DTexture(m_hWyphonPartner, m_shareHandle, m_width, m_height, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET, m_WyphonTextureDescriptionT);
	}


	return FF_SUCCESS;
}

DWORD RRExportWyphon::DeInitGL()
{
		// tell everybody that we're off line
	DestroyWyphonPartner(m_hWyphonPartner);

	if ( m_glTextureHandle ) { // release any active texture
		WyphonUtils::ReleaseLinkedGLTexture(m_glTextureName, m_glTextureHandle);
	}

		// close dx device and gl/dx interop device
	WyphonUtils::ReleaseDevice(m_hWyphonDevice);

	return FF_SUCCESS;
}

DWORD RRExportWyphon::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	if (pGL->numInputTextures<1) return FF_FAIL;

	if (pGL->inputTextures[0]==NULL) return FF_FAIL;
  
	FFGLTextureStruct &InputTexture = *(pGL->inputTextures[0]);

	// bind the original texture (for drawing)
	glBindTexture(GL_TEXTURE_2D, InputTexture.Handle);

	//enable texturemapping
	glEnable(GL_TEXTURE_2D);

	//get the max s,t that correspond to the 
	//width,height of the used portion of the allocated texture space
	FFGLTexCoords maxCoords = GetMaxGLTexCoords(InputTexture);

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
  
	//disable texturemapping
	glDisable(GL_TEXTURE_2D);
 
	if ( !m_glTextureHandle ) {
			// unbind the drawn texture
		glBindTexture(GL_TEXTURE_2D, 0);
	} else {
		/* Assume "fbo" is a name of a FBO created using glGenFramebuffersEXT(1, &fbo),
		 * and width/height are the dimensions of the texture, respectively.
		 * "tex_src" is the name of the source texture, and
		 * "tex_dst" is the name of the destination texture, which should have been
		 * already created */ 

		/*** COPY THE INPUT TEXTURE TO SHARED TEXTURE ****/
		/// bind the FBO
		m_extensions.glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		/// attach the source texture to the fbo
		m_extensions.glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_2D, InputTexture.Handle, 0);

		// lock and bind destination texture
		glBindTexture(GL_TEXTURE_2D, m_glTextureName);
		WyphonUtils::LockGLTexture(m_glTextureHandle);

		/// copy from framebuffer (here, the FBO!) to the bound texture
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,m_width, m_height);
		glBindTexture(GL_TEXTURE_2D, 0);
		// unlock destination texture (but do not unbind it, we need it for drawing)
		WyphonUtils::UnlockGLTexture(m_glTextureHandle);
		/// unbind the FBO
		m_extensions.glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
 
	return FF_SUCCESS;
}

DWORD RRExportWyphon::GetParameter(DWORD dwIndex)
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

DWORD RRExportWyphon::SetParameter(const SetParameterStruct* pParam)
{
	if (pParam != NULL) {
		
	BOOL bChanged;

	switch (pParam->ParameterNumber) {

		case FFPARAM_WyphonApplicationName:
			bChanged = strcmp( m_WyphonApplicationName, (char*) pParam->NewParameterValue ) != 0;
			if ( bChanged ) {
				strcpy_s( m_WyphonApplicationName, (char*) pParam->NewParameterValue);
				mbstowcs( m_WyphonApplicationNameT, m_WyphonApplicationName, WYPHON_MAX_DESCRIPTION_LENGTH+1 ); // also keep wchar_t version up to date
				
				if ( strlen(m_WyphonTextureDescription) && strlen(m_WyphonApplicationName) ) {
					DestroyWyphonPartner(m_hWyphonPartner); // destroys this wyphon partner and unshares textures
					m_hWyphonPartner = CreateWyphonPartner( // create with new name
						m_WyphonApplicationNameT,
						this, // link to instance of this object (for callbacks)
						NULL,NULL,NULL,NULL /* no callbacks */
					);
					if ( m_shareHandle ) { // re-share the old texture
						ShareD3DTexture(m_hWyphonPartner, m_shareHandle, m_width, m_height, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET, m_WyphonTextureDescriptionT);
					}
				}
			}
			break;
		case FFPARAM_WyphonTextureDescription:
			bChanged = strcmp( m_WyphonTextureDescription, (char*) pParam->NewParameterValue ) != 0;
			if ( bChanged ) {
				strcpy_s( m_WyphonTextureDescription, (char*) pParam->NewParameterValue);
				mbstowcs( m_WyphonTextureDescriptionT, m_WyphonTextureDescription, WYPHON_MAX_DESCRIPTION_LENGTH+1 ); // also keep wchar_t version up to date
				
				if ( strlen(m_WyphonTextureDescription) && strlen(m_WyphonApplicationName) ) {
					if ( m_shareHandle) {
						UnshareD3DTexture(m_hWyphonPartner, m_shareHandle);
					}
					if ( GenerateTexture() ) {
						ShareD3DTexture(m_hWyphonPartner, m_shareHandle, m_width, m_height, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET, m_WyphonTextureDescriptionT);
					}
				}
			}
			break;

		default:
			return FF_FAIL;
		}

		return FF_SUCCESS;
	
	}

	return FF_FAIL;
}

BOOL RRExportWyphon::GenerateTexture() {
	if ( m_glTextureHandle ) {
		ReleaseLinkedGLTexture(m_glTextureName, m_glTextureHandle);
	}
	m_shareHandle = NULL; // means: create new shared texture
	HRESULT res = CreateLinkedGLTexture(m_width, m_height, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, m_shareHandle, m_glTextureName, m_glTextureHandle);
	return (res == S_OK);
}
