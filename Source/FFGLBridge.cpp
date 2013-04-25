#include <FFGL.h>
#include <FFGLLib.h>
#include "dxConnector.h"
#include "FFGLBridge.h"

#define FFPARAM_SharingName		(0)
#define FFPARAM_Reload			(1)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
	FFGLBridge::CreateInstance,	// Create method
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

FFGLBridge::FFGLBridge()
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

DWORD FFGLBridge::InitGL(const FFGLViewportStruct *vp)
{
	HWND hWnd = GetForegroundWindow(); // we don't have the Window Handle until now :(

	//initialize gl extensions and
	//make sure required features are supported
	m_extensions.Initialize();
  
	m_dxConnector.init(hWnd);
		// DirectX device initialization needs the window to be redrawn (creates black areas)
	SendMessage( hWnd, WM_PAINT, NULL, NULL );

	return FF_SUCCESS;
}

DWORD FFGLBridge::DeInitGL()
{
	/* this causes death. either it must be used another way or there's no need to unregister
	wglDXUnregisterObjectNV(pDevice, m_TextureFromDX9Handle);*/
	m_dxConnector.cleanup();

	return FF_SUCCESS;
}

DWORD FFGLBridge::ProcessOpenGL(ProcessOpenGLStruct *pGL)
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

DWORD FFGLBridge::GetParameter(DWORD dwIndex)
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

DWORD FFGLBridge::SetParameter(const SetParameterStruct* pParam)
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
