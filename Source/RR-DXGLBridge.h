#ifndef RRDXGLBridge_H
#define RRDXGLBridge_H

#include <FFGLShader.h>
#include "PluginBase/FFGLPluginSDK.h"

class RRDXGLBridge :
public CFreeFrameGLPlugin
{
public:
	RRDXGLBridge();
  virtual ~RRDXGLBridge() {}

	///////////////////////////////////////////////////
	// FreeFrameGL plugin methods
	///////////////////////////////////////////////////
	
	DWORD	SetParameter(const SetParameterStruct* pParam);		
	DWORD	GetParameter(DWORD dwIndex);					
	DWORD	ProcessOpenGL(ProcessOpenGLStruct* pGL);
	DWORD InitGL(const FFGLViewportStruct *vp);
	DWORD DeInitGL();

	DWORD RRDXGLBridge::UpdateTexture();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
  {
  	*ppOutInstance = new RRDXGLBridge();
	  if (*ppOutInstance != NULL)
      return FF_SUCCESS;
	  return FF_FAIL;
  }

protected:	
	// Parameters
	char m_SharedMemoryName[256];
	
	int m_initResources;

	DXGLConnector m_dxConnector;

	FFGLExtensions m_extensions;
	GLint m_maxCoordsLocation;
};


#endif
