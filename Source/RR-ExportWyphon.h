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

#ifndef RRExportWyphon_H
#define RRExportWyphon_H

#include <FFGLShader.h>
#include "PluginBase/FFGLPluginSDK.h"
#include "../../../Wyphon/WyphonUtils/WyphonUtils.h"

class RRExportWyphon :
public CFreeFrameGLPlugin
{
public:
	RRExportWyphon();
  virtual ~RRExportWyphon() {}

	///////////////////////////////////////////////////
	// FreeFrameGL plugin methods
	///////////////////////////////////////////////////
	
	DWORD	SetParameter(const SetParameterStruct* pParam);		
	DWORD	GetParameter(DWORD dwIndex);					
	DWORD	ProcessOpenGL(ProcessOpenGLStruct* pGL);
	DWORD	InitGL(const FFGLViewportStruct *vp);
	DWORD	DeInitGL();
	BOOL	GenerateTexture();

	DWORD RRExportWyphon::UpdateTexture();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
  {
  	*ppOutInstance = new RRExportWyphon();
	  if (*ppOutInstance != NULL)
      return FF_SUCCESS;
	  return FF_FAIL;
  }

protected:	
	// Parameters

	/* both string containers are needed because FFGL works with char* and Wyphon works with wchar_t*
	   we'll keep them equal */
	char							m_WyphonApplicationName[WYPHON_MAX_DESCRIPTION_LENGTH+1];
	wchar_t							m_WyphonApplicationNameT[WYPHON_MAX_DESCRIPTION_LENGTH+1];
	char							m_WyphonTextureDescription[WYPHON_MAX_DESCRIPTION_LENGTH+1];
	wchar_t							m_WyphonTextureDescriptionT[WYPHON_MAX_DESCRIPTION_LENGTH+1];

	// 
	HANDLE							m_hWyphonPartner;
	HANDLE							m_hWyphonDevice;

	Wyphon::WyphonD3DTextureInfo	m_WyphonTextureInfo;
	
	// Wyphon's GL Texture
	GLuint							m_glTextureName;
	HANDLE							m_glTextureHandle;
	HANDLE							m_shareHandle;

	// The Framebuffer for copying the texture
	GLuint							m_fbo;
	int								m_width;
	int								m_height;

	int m_initResources;

	FFGLExtensions m_extensions;
	GLint m_maxCoordsLocation;
};


#endif
