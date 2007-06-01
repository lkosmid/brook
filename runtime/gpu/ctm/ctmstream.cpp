#include "ctmstream.hpp"

using namespace brook;

extern bool verbose;

static const int kComponentSizes[] =
{
  sizeof(float),
  sizeof(unsigned char),
  sizeof(unsigned short)
};

//! Utility routine to print the contents of the given buffer
void
printBuffer(void *data, const int width, const int height)
{
    float *dest = (float *)data;
    fprintf(stderr, "After execute\nDestination\n");
    for(int i = 0; i < height; i++ )
    {
        for(int j = 0; j < width; j++ )
            printf( "%3.0f ", dest[i * width + j] );
        printf("\n");
    }
}

//! Constructor for CTMStream class 
CTMStream::CTMStream( GPUContextCTM* inContext,
                      int inWidth,
                      int inHeight,
                      int origWidth,
                      int origHeight,
                      int inComponents,
                      ComponentType inComponentType,
                      bool readOnly)
    : _userWidth(origWidth),
      _userHeight(origHeight),
      _components(inComponents),
      _componentType(inComponentType),
      _componentSize(0),
      _readOnly(readOnly)
{
    _componentSize = kComponentSizes[ _componentType ];
    
    // CTM does not support 3-component buffers so used 4 components instead
    if(inComponents == 3)
        _internalComponents = 4;
    else
        _internalComponents = inComponents;

    // Select the internal format and tiling mode for the CTM buffer
    switch(_componentType)
    {
    case kComponentType_Float:
        switch(_components)
        {
        case 1:
            _internalFormat = CTM::FLOAT32_1;
            break;
        case 2:
            _internalFormat = CTM::FLOAT32_2;
            break;
        case 3:
        case 4:
            _internalFormat = CTM::FLOAT32_4;
            break;
        }
        break;
        
    case kComponentType_Fixed:
        switch(_components)
        {
        case 1:
            _internalFormat = AMU_CBUF_FLD_FORMAT_UINT8_1;
            break;
        case 2:
            _internalFormat = AMU_CBUF_FLD_FORMAT_UINT8_2;
            break;
        case 3:
        case 4:
            _internalFormat = AMU_CBUF_FLD_FORMAT_UINT8_4;
            break;
        }
        break;
        
    case kComponentType_ShortFixed:
        switch(_components)
        {
        case 1:
            _internalFormat = AMU_CBUF_FLD_FORMAT_UINT16_1;
            break;
        case 2:
            _internalFormat = AMU_CBUF_FLD_FORMAT_UINT16_2;
            break;
        case 3:
        case 4:
            _internalFormat = AMU_CBUF_FLD_FORMAT_UINT16_4;
            break;
        }
        break;
    }

    // xxx Using TILED0 by default
    _internalTiling = CTM::LINEAR; 
    _context = inContext;

    // Create the CTM::Buffer in GPU memory
    _buffer = _context->getDevice()->alloc(CTM::MEM_GPU, 
                                           inWidth, inHeight, 
                                           _internalFormat, _internalTiling);
    if(verbose)
        fprintf(stderr, "Allocated GPU buffer %p of size %d, %d\n", _buffer, inWidth, inHeight);
    
}

//! Size of buffer in bytes 
unsigned int CTMStream::getAllocatedSize()
{
    return _componentSize * _internalComponents * _buffer->getWidth() * _buffer->getHeight();
}

//! Destructor 
CTMStream::~CTMStream()
{
    CTMLOG(2) << "~CTMStream";
    // We should attempt to release space from context/runtime here
    // xxx Does not free the internal CTM::Buffer... will soon run out of GPU memory
}

//! CTMStream factory
CTMStream* CTMStream::create( GPUContextCTM* inContext,
                              int inWidth,
                              int inHeight,
                              int inComponents,
                              ComponentType inType,
                              bool read_only)
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
                                       read_only);
    return result;
}
