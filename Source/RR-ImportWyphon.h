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

#ifndef RRImportWyphon_H
#define RRImportWyphon_H

#include <FFGLShader.h>
#include "PluginBase/FFGLPluginSDK.h"
//#include "wyphonConnector.h"
#include "../../../Wyphon/WyphonUtils/WyphonUtils.h"


class RRImportWyphon :
public CFreeFrameGLPlugin
{
public:
	RRImportWyphon();
	virtual ~RRImportWyphon() {}

	///////////////////////////////////////////////////
	// FreeFrameGL plugin methods
	///////////////////////////////////////////////////
	
	DWORD	SetParameter(const SetParameterStruct* pParam);		
	DWORD	GetParameter(DWORD dwIndex);					
	DWORD	ProcessOpenGL(ProcessOpenGLStruct* pGL);
	DWORD	InitGL(const FFGLViewportStruct *vp);
	DWORD	DeInitGL();

	DWORD	UpdateTexture();
	DWORD	NotifySharingStarted(HANDLE wyphonPartnerHandle, Wyphon::WyphonD3DTextureInfo WyphonTextureInfo);
	DWORD	NotifySharingStopped(HANDLE wyphonPartnerHandle, Wyphon::WyphonD3DTextureInfo WyphonTextureInfo);
	BOOL	MatchWyphonString(LPCTSTR objectStringW, LPCSTR filterStringMB);
	DWORD	UnsetTextureData();
	HANDLE	GetTextureData(Wyphon::WyphonD3DTextureInfo &WyphonTextureInfo);
	
	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
	{
  		*ppOutInstance = new RRImportWyphon();
		if (*ppOutInstance != NULL) {
			return FF_SUCCESS;
		}
		return FF_FAIL;
	}

protected:	
	// Parameters
	char m_WyphonApplicationName[WYPHON_MAX_DESCRIPTION_LENGTH+1];
	char m_WyphonTextureDescription[WYPHON_MAX_DESCRIPTION_LENGTH+1];
	HANDLE m_hWyphonPartner;

	Wyphon::WyphonD3DTextureInfo m_WyphonTextureInfo;
	bool m_bTextureNeedsUpdate; // indicates that texture can be connected 
	
	int m_initResources;

	// Wyphon's GL Texture
	GLuint m_glTextureName;
	HANDLE m_glTextureHandle;

	FFGLExtensions m_extensions;
	GLint m_maxCoordsLocation;

		// the framebuffer (for copying textures)
	GLuint m_fbo;
};

void TextureSharingStartedCALLBACK( HANDLE wyphonPartnerHandle, unsigned __int32 sendingPartnerId, HANDLE sharedTextureHandle, unsigned __int32 width, unsigned __int32 height, DWORD format, DWORD usage, LPTSTR description, void * customData );
void TextureSharingStoppedCALLBACK( HANDLE wyphonPartnerHandle, unsigned int sendingPartnerId, HANDLE sharedTextureHandle, unsigned int width, unsigned int height, DWORD format, DWORD usage, LPTSTR description, void * customData );

#endif
