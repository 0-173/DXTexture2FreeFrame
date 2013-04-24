#ifndef FFGLBridge_H
#define FFGLBridge_H

#include <FFGLShader.h>
#include "../FFGLPluginSDK.h"

class FFGLBridge :
public CFreeFrameGLPlugin
{
public:
	FFGLBridge();
  virtual ~FFGLBridge() {}

	///////////////////////////////////////////////////
	// FreeFrameGL plugin methods
	///////////////////////////////////////////////////
	
	DWORD	SetParameter(const SetParameterStruct* pParam);		
	DWORD	GetParameter(DWORD dwIndex);					
	DWORD	ProcessOpenGL(ProcessOpenGLStruct* pGL);
	DWORD InitGL(const FFGLViewportStruct *vp);
	DWORD DeInitGL();

	DWORD FFGLBridge::UpdateTexture();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
  {
  	*ppOutInstance = new FFGLBridge();
	  if (*ppOutInstance != NULL)
      return FF_SUCCESS;
	  return FF_FAIL;
  }

protected:	
	// Parameters
	char m_SharedMemoryName[256];
	
	int m_initResources;

	GLuint m_TextureFromDX9Name;
	HANDLE m_TextureFromDX9Handle;
		// the DX/GL interop device's handle
	HANDLE m_InteropHandle;

	FFGLExtensions m_extensions;
	GLint m_maxCoordsLocation;
};


#endif
