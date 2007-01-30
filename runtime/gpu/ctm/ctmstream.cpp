#include "ctmstream.hpp"

using namespace brook;

static const int kComponentSizes[] =
{
  sizeof(float),
  sizeof(unsigned char),
  sizeof(unsigned short)
};

CTMStream::CTMStream( GPUContextCTM* inContext,
                      int inWidth,
                      int inHeight,
                      int origWidth,
                      int origHeight,
                      int inComponents,
                      ComponentType inComponentType,
                      bool readonly,
                      AMuint32 GPUAddress)
	: width(inWidth),
      height(inHeight),
      userWidth(origWidth),
      userHeight(origHeight),      
      components(inComponents),
      componentType(inComponentType),
      componentSize(0),
      read_only(readonly),
      GPUAddress(GPUAddress)
{
    componentSize = kComponentSizes[ componentType ];
    
    if(inComponents == 3)
        internalComponents = 4;
    else
        internalComponents = inComponents;
    
    switch(componentType)
    {
    case kComponentType_Float:
        switch(components)
        {
        case 1:
            internalFormat = AMU_CBUF_FLD_FORMAT_FLOAT32_1;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        case 2:
            internalFormat = AMU_CBUF_FLD_FORMAT_FLOAT32_2;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        case 3:
        case 4:
            internalFormat = AMU_CBUF_FLD_FORMAT_FLOAT32_4;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        }
        break;
    case kComponentType_Fixed:
        switch(components)
        {
        case 1:
            internalFormat = AMU_CBUF_FLD_FORMAT_UINT8_1;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        case 2:
            internalFormat = AMU_CBUF_FLD_FORMAT_UINT8_2;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        case 3:
        case 4:
            internalFormat = AMU_CBUF_FLD_FORMAT_UINT8_4;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        }
        break;
    case kComponentType_ShortFixed:
        switch(components)
        {
        case 1:
            internalFormat = AMU_CBUF_FLD_FORMAT_UINT16_1;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        case 2:
            internalFormat = AMU_CBUF_FLD_FORMAT_UINT16_2;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        case 3:
        case 4:
            internalFormat = AMU_CBUF_FLD_FORMAT_UINT16_4;
            internalTiling = AMU_CBUF_FLD_TILING_TILED0;
            break;
        }
        break;
    }
    
    _context = inContext;
}

unsigned int CTMStream::getAllocatedSize()
{
    return componentSize*internalComponents*width*height;
}

CTMStream::~CTMStream()
{
    CTMLOG(2) << "~CTMStream";
    // We should attempt to release space from context/runtime here
}

CTMStream* CTMStream::create( GPUContextCTM* inContext,
                              int inWidth,
                              int inHeight,
                              int inComponents,
                              ComponentType inType,
                              bool read_only, AMuint32 GPUAddress )
{
    CTMPROFILE("CTMStream::create");
    
    // pad width out to 128 bytes and 16(!) elements each side (tiling
    // requirement)
    
    int finalComponents;
    if(inComponents == 3)
        finalComponents = 4;
    else
        finalComponents = inComponents;
    
    int test = 128/(kComponentSizes[ inType ]*finalComponents);
    if(test < 16)
        test = 16;
    int padWidth  = ((inWidth + test-1)/test) * test;
    int padHeight = ((inHeight + 15)/16) * 16;
        
    CTMStream* result = new CTMStream( inContext,
                                       padWidth, padHeight,
                                       inWidth, inHeight,
                                       inComponents,
                                       inType,
                                       read_only,
                                       GPUAddress );
    return result;
}

