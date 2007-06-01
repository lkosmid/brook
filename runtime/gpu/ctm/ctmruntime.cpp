//! ctmruntime.cpp
#include "ctmruntime.hpp"
#include "ctmstream.hpp"

#define ALIGN 2048
// #define DEBUG

bool verbose = (getenv("VERBOSE_BROOK") != NULL);

const int ParamOffset = 36;

namespace brook
{
    static const float kInterpolantBias = 0.05f;
    
    static const float kDefaultDepth = 0.0f;
   
    static const char* kPassthroughPixelShaderSource =
    "    ps_3_0              \n"
    "    dcl vPos.xy         \n"
    "    dcl_2d s0           \n"
    "    texld r0, vPos, s0  \n"
    "    mov oC0.xyzw, r0    \n"
    "    end                 \n"
    ;
    
    //! Create the GPURuntime for CTM - internally creates a CTM GPUContext
    //! object which then stores the device specific data structures for the GPU
    //! xxx multi-GPU?    
    GPURuntimeCTM* GPURuntimeCTM::create( void* inContextValue )
    {
        GPUContextCTM* context = GPUContextCTM::create( inContextValue );
        if( !context )
            return NULL;
        
        GPURuntimeCTM* result = new GPURuntimeCTM();
        if( !result->initialize( context ) )
        {
            delete context;
            return NULL;
        }
        
        return result;
    }
    
    //! Create the CTM GPUContext
    GPUContextCTM* GPUContextCTM::create( void* inContextValue )
    {
        GPUContextCTM* result = new GPUContextCTM();
        
        // Initialize the devide internally
        if( result->initialize( inContextValue ) )
            return result;
        
        // Failure in initialize routine
        delete result;
        return NULL;
    }
    
    //! Constructor for CTM GPUContext
    GPUContextCTM::GPUContextCTM()
    {
        // Initialize various data members to NULL
        
        _device = NULL;
        
        _pass = NULL;
        _program = NULL;   
        
        _fconst = NULL;
        _iconst = NULL;
        _bconst = NULL;
        _interp = NULL;

        _canPopMemory = false;
        _pushedInBegin = false;

        // Set number of textures to zero
        _numTextures = 0;
    }
    
    //! Destructor for CTM GPUContext
    GPUContextCTM::~GPUContextCTM()
    {
        // Delete all internally allocated data
        delete _pass;
        delete _program;
        delete _fconst;
        delete _iconst;
        delete _bconst;
        delete _interp;
        
        // Destroy the connection to the device
        delete _device;
    }
    

    bool GPUContextCTM::initialize( void* inContextValue )
    {
        memset(&_params, 0, sizeof(_params));
        
        if( inContextValue == NULL )
        {
            // Open device and create resources
            _device = CTM::Device::open(0);
            if(!_device)
            {
                fprintf(stderr, "Failed to open CTM device id %d!\n", 0);
                return false;
            }
            
            const int bufSize = 256 * 1024; // 256 KB should be enough for now
            
            // Allocate memory for various buffers
            _fconst = _device->alloc(CTM::MEM_SYS, bufSize);
            _iconst = _device->alloc(CTM::MEM_SYS, bufSize);
            _interp = _device->alloc(CTM::MEM_SYS, bufSize);
            
            _passthroughPixelShader = createPixelShader(kPassthroughPixelShaderSource);
            
            if(verbose)
            {
                printf("Device: %p\n", _device);
                printf("Float Const: %p\n", _fconst);
                printf("Int Const: %p\n", _iconst);
                printf("Interp: %p\n", _interp);
            }
        }
        else
        {
            // not sure yet how to best deal with an already created
            // context here
            CTMWARN << "Not sure what to do with context already up...\n";
            fflush(stderr);
        }

        return true;
    }
    
    int GPUContextCTM::getShaderFormatRank( const char* inNameString ) const
    {
        if( strcmp( "ctm", inNameString ) == 0 )
            return 1;
        else
            return -1;
    }

    bool GPUContextCTM::isTextureExtentValid( unsigned int inExtent ) const {
        return inExtent <= _device->getDeviceCaps()->getMaxBufferWidth(); 
                                // praveen - Using max width 
    }
    
    float4 GPUContextCTM::getStreamIndexofConstant( TextureHandle inTexture ) const
    {
        CTMStream* texture = (CTMStream*)inTexture;
        int textureWidth = texture->getWidth();
        int textureHeight = texture->getHeight();
        
        return float4( (float)textureWidth, (float)textureHeight, 0, 0 );
    }
    
    float4 GPUContextCTM::getStreamGatherConstant( unsigned int inRank,
                                                   const unsigned int* inDomainMin,
                                                   const unsigned int* inDomainMax,
                                                   const unsigned int* inExtents ) const
    {
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        if( inRank == 1 )
        {
            unsigned int base = inDomainMin[0];
            //unsigned int width = inExtents[0];
            
            scaleX = 1.0f;
            offsetX = (float)base + 0.5f;
        }
        else
        {
            unsigned int baseX = inDomainMin[1];
            unsigned int baseY = inDomainMin[0];
            //unsigned int width = inExtents[1];
            //unsigned int height = inExtents[0];
            
            scaleX = 1.0f;
            scaleY = 1.0f;
            offsetX = (float)baseX + 0.5f;
            offsetY = (float)baseY + 0.5f;
        }
        
        return float4( scaleX, scaleY, offsetX, offsetY );
    }

    
    void GPUContextCTM::get1DInterpolant( const float4 &start, 
                                          const float4 &end,
                                          const unsigned int outputWidth,
                                          GPUInterpolant &interpolant) const
    {
        float scaleBiasX = -0.5f*((float)end.x-(float)start.x)/(float)outputWidth;
        float scaleBiasY = -0.5f*((float)end.y-(float)start.y)/(float)outputWidth;
        float scaleBiasZ = -0.5f*((float)end.z-(float)start.z)/(float)outputWidth;
        float scaleBiasW = -0.5f*((float)end.w-(float)start.w)/(float)outputWidth;
        
        float4 f1;
        f1.x = start.x+ scaleBiasX;
        f1.y = start.y+ scaleBiasY;
        f1.z = start.z+ scaleBiasZ;
        f1.w = start.w+ scaleBiasW;
        
        float4 f2;
        f2.x = 3*(end.x - start.x)+start.x+ scaleBiasX;
        f2.y = 3*(end.y - start.y)+start.y+ scaleBiasY;
        f2.z = 3*(end.z - start.z)+start.z+ scaleBiasZ;
        f2.w = 3*(end.w - start.w)+start.w+ scaleBiasW;
        
        interpolant.vertices[0] = f1;
        interpolant.vertices[1] = f2; 
        interpolant.vertices[2] = f1;
    }
    
    void GPUContextCTM::get2DInterpolant( const float2 &start, 
                                          const float2 &end,
                                          const unsigned int outputWidth,
                                          const unsigned int outputHeight, 
                                          GPUInterpolant &interpolant) const
    {
        float scaleBiasX = -0.5f*((float)end.x-(float)start.x)/(float)outputWidth;
        float scaleBiasY = -0.5f*((float)end.y-(float)start.y)/(float)outputHeight;
        float4 f1 = float4( start.x+scaleBiasX, start.y+scaleBiasY, 0, 1 );
        
        float4 f2;
        f2.x = 3*(end.x - start.x)+start.x+scaleBiasX;
        f2.y = start.y+scaleBiasY;
        f2.z = 0;
        f2.w = 1;
        
        float4 f3;
        f3.x = start.x+scaleBiasX;
        f3.y = 3*(end.y - start.y)+start.y+scaleBiasY;
        f3.z = 0;
        f3.w = 1;
        
        interpolant.vertices[0] = f1;
        interpolant.vertices[1] = f2; 
        interpolant.vertices[2] = f3;
    }
    
    void GPUContextCTM::getStreamInterpolant( const TextureHandle inTexture,
                                              unsigned int rank,
                                              const unsigned int* domainMin,
                                              const unsigned int* domainMax,
                                              const unsigned int outputWidth,
                                              const unsigned int outputHeight, 
                                              GPUInterpolant &interpolant) const
    {
        //CTMStream* texture = (CTMStream*)inTexture;
        unsigned int minX, minY, maxX, maxY;
        float scaleBiasX = 0.0f;
        float scaleBiasY = 0.0f;
        
        if( rank == 1 )
        {
            minX = domainMin[0];
            minY = 0;
            maxX = domainMax[0];
            maxY = 1;
            scaleBiasX = -0.5f * ((float)maxX -(float)minX)/(float)outputWidth + kInterpolantBias;
            scaleBiasY = 0.0f;
        }
        else
        {
            minX = domainMin[1];
            minY = domainMin[0];
            maxX = domainMax[1];
            maxY = domainMax[0];
            scaleBiasX = -0.5f * ((float)maxX - (float)minX)/(float)outputWidth + kInterpolantBias;
            scaleBiasY = -0.5f * ((float)maxY - (float)minY)/(float)outputHeight + kInterpolantBias;
        }

        GPULOGPRINT(4) << "Interpolant Rank = " << rank << ", Min = (" << minX << ", " << maxX <<") to (" << minY << ", " << maxY << ")" << std::endl;
        float width = (float)(maxX - minX);
        float height = (float)(maxY - minY);
        
        float xmin = (float)minX + scaleBiasX;
        float ymin = (float)minY + scaleBiasY;
        
        float xmax = (float)minX + (3*width)  +scaleBiasX;
        float ymax = (float)minY + (3*height) +scaleBiasY;
        
        interpolant.vertices[0] = float4(xmin,ymin,0.0,1);
        interpolant.vertices[1] = float4(xmax,ymin,0.0,1);
        interpolant.vertices[2] = float4(xmin,ymax,0.0,1); 
    }

    void GPUContextCTM::getStreamOutputRegion( const TextureHandle inTexture,
                                               unsigned int rank,
                                               const unsigned int* domainMin,
                                               const unsigned int* domainMax,
                                               GPURegion &region) const
    {
        //CTMStream* texture = (CTMStream*)inTexture;
        unsigned int minX, minY, maxX, maxY;
        if( rank == 1 )
        {
            minX = domainMin[0];
            minY = 0;
            maxX = domainMax[0];
            maxY = 1;
            region.viewport.minX = minX;
            region.viewport.minY = minY;
            region.viewport.maxX = maxX-1;
            region.viewport.maxY = maxY-1;
        }
        else
        {
            minX = domainMin[1];
            minY = domainMin[0];
            maxX = domainMax[1];
            maxY = domainMax[0];
            region.viewport.minX = minX;
            region.viewport.minY = minY;
            region.viewport.maxX = maxX-1;
            region.viewport.maxY = maxY-1;
        }
        
        float xmin = (float)minX;
        float ymin = (float)minY;
        float width = (float)(maxX - minX);
        float height = (float)(maxY - minY);
        
        float xmax = xmin + 3*width;
        float ymax = ymin + 3*height;
        
        region.vertices[0] = float4(xmin,ymin,0.0,1);
        region.vertices[1] = float4(xmax,ymin,0.0,1);
        region.vertices[2] = float4(xmin,ymax,0.0,1);       
        
    }
    
    void GPUContextCTM::getStreamReduceInterpolant( const TextureHandle inTexture,
                                                    const unsigned int outputWidth,
                                                    const unsigned int outputHeight, 
                                                    const unsigned int minX,
                                                    const unsigned int maxX, 
                                                    const unsigned int minY,
                                                    const unsigned int maxY,
                                                    GPUInterpolant &interpolant) const
    {
        CTMStream* texture = (CTMStream*)inTexture;
        float scaleBiasX = 0.0f;
        float scaleBiasY = 0.0f;

        scaleBiasX = -0.5f+kInterpolantBias;
        scaleBiasY = -0.5f+kInterpolantBias;
        
        float biasX = texture->getWidth() <= 1 ? 0.0f : scaleBiasX;
        float biasY = texture->getHeight() <= 1 ? 0.0f : scaleBiasY;
        
        float width = (float)outputWidth;
        float height = (float)outputHeight;
        
        float xmin = (float)minX + biasX;
        float ymin = (float)minY + biasY;
        
        float xmax = (float)minX + (3*width)  + biasX;
        float ymax = (float)minY + (3*height) + biasY;
        
        interpolant.vertices[0] = float4(xmin,ymin,0.0,1);
        interpolant.vertices[1] = float4(xmax,ymin,0.0,1);
        interpolant.vertices[2] = float4(xmin,ymax,0.0,1); 
    }
    
    void GPUContextCTM::getStreamReduceOutputRegion( const TextureHandle inTexture,
                                                     const unsigned int minX,
                                                     const unsigned int maxX, 
                                                     const unsigned int minY,
                                                     const unsigned int maxY,
                                                     GPURegion &region) const
    {
        float xmin = (float)minX;
        float ymin = (float)minY;
        float width = (float)(maxX - minX);
        float height = (float)(maxY - minY);
        
        float xmax = xmin + 3*width;
        float ymax = ymin + 3*height;
        
        region.vertices[0] = float4(xmin,ymin,0.0,1);
        region.vertices[1] = float4(xmax,ymin,0.0,1);
        region.vertices[2] = float4(xmin,ymax,0.0,1);    
        
        region.viewport.minX = minX;
        region.viewport.minY = minY;
        region.viewport.maxX = maxX-1;
        region.viewport.maxY = maxY-1;
    }
    
    GPUContextCTM::TextureHandle GPUContextCTM::createTexture2D( size_t inWidth,
                                                                 size_t inHeight,
                                                                 TextureFormat inFormat,
                                                                 bool read_only)
    {
        int components;
        CTMStream::ComponentType componentType;
        int format = 0;
        
        switch( inFormat )
        {
        case kTextureFormat_Float1:
        case kTextureFormat_ShortFixed1:
        case kTextureFormat_Fixed1:
            components = 1;
            format = CTM::FLOAT32_1;
            break;
        case kTextureFormat_Float2:
        case kTextureFormat_ShortFixed2:
        case kTextureFormat_Fixed2:
            components = 2;
            format = CTM::FLOAT32_2;
            break;
        case kTextureFormat_Float3:
        case kTextureFormat_ShortFixed3:
        case kTextureFormat_Fixed3:
            components = 3;
            // xxx 3 component formats not supported by CTM
            format = CTM::FLOAT32_4;
            break;
        case kTextureFormat_Float4:
        case kTextureFormat_ShortFixed4:
        case kTextureFormat_Fixed4:
            components = 4;
            format = CTM::FLOAT32_4;
            break;
        default:
            GPUError("Unknown format for CTM Stream");
            return 0;
            break;
        }
        
        switch( inFormat )
        {
        case kTextureFormat_Float1:
        case kTextureFormat_Float2:
        case kTextureFormat_Float3:
        case kTextureFormat_Float4:
            componentType = CTMStream::kComponentType_Float;
            break;
        case kTextureFormat_Fixed1:
        case kTextureFormat_Fixed2:
        case kTextureFormat_Fixed3:
        case kTextureFormat_Fixed4:
            componentType = CTMStream::kComponentType_Fixed;
            break;
        case kTextureFormat_ShortFixed1:
        case kTextureFormat_ShortFixed2:
        case kTextureFormat_ShortFixed3:
        case kTextureFormat_ShortFixed4:
            componentType = CTMStream::kComponentType_ShortFixed;
            break;
        default:
            GPUError("Unknown format for CTM Stream");
            return 0;
            break;
        }

        // Save this state when the total number of textures is zero
        if(_numTextures == 0)
        {
//            _device->pushMemory();
            GPULOGPRINT(2) << "First texture being allocated. Pushing Device memory state" << std::endl;
        }

        // Assign a pointer for this and update GPU pointer
        CTMStream* result = CTMStream::create( this, inWidth, inHeight, components,
                                               componentType, read_only);

        // Increment the number of textures
        _numTextures++;
      
        return result;
    }
    
    //! Free the given inpu texture
    void GPUContextCTM::releaseTexture( TextureHandle inTexture )
    {
        CTMStream* texture = (CTMStream*)inTexture;

        // Decrease the texture count
        _numTextures--;

        // If the texture count reaches zero, free all memory
        if(_numTextures == 0)
        {
//            _device->popMemory();
            GPULOGPRINT(2) << "Texture count has reached zero. Popping Device memory state" << std::endl;
        }

        // xxx Does nothing really right now... 
        delete texture;
    }
    
    void GPUContextCTM::copyData( void* toBuffer, size_t toRowStride,  size_t toElementStride,
                                  const void* fromBuffer, size_t fromRowStride, size_t fromElementStride,
                                  size_t columnCount, size_t rowCount, 
                                  size_t numElements,size_t elementSize )
    {
#if 0
        if(verbose)
        {
            printBuffer(fromBuffer, columnCount, rowCount);
        }
#endif

        char* outputLine = (char*)toBuffer;
        const char* inputLine = (const char*)fromBuffer;
        
        // Fast copy
        if( toRowStride == fromRowStride &&
            toElementStride == fromElementStride)
        {
            memcpy(outputLine,inputLine,columnCount*rowCount*numElements*elementSize);
        }
        
        else{
            for( size_t y = 0; y < rowCount; y++ )
            {
                char* outputPixel = outputLine;
                const char* inputPixel = inputLine;
                if (numElements==1&&elementSize==1&&toElementStride==fromElementStride) {
                    memcpy(outputPixel,inputPixel,columnCount);
                }
                else if( toElementStride == fromElementStride&&elementSize!=1)
                {
                    memcpy(outputPixel, inputPixel, numElements*elementSize*columnCount);
                }
                else
                {
                    for( size_t x = 0; x < columnCount; x++ )
                    {
                        // TIM: for now we assume floating-point components
                        char* output = outputPixel;
                        const char* input = inputPixel;
                        if (elementSize!=1)
                        {
                            memcpy(output, input, numElements*elementSize);
                        }
                        else {
                            if (fromElementStride!=toElementStride&&toElementStride==1)
                                input+=(fromElementStride==4?fromElementStride-2:fromElementStride-1);//offset the input if writing to it
                            for( size_t i =((output+=(toElementStride==1?1:toElementStride-1)),0); i <(numElements>3?3:numElements); i++ )//offset the output, but remember the alpha channel is separate
                                *--output = *input++;// I've always wanted to do that
                            if(numElements>3)
                                output[3]=*input++;//now to copy alpha
                        }
                        
                        inputPixel += fromElementStride;
                        outputPixel += toElementStride;
                    }
                }
                inputLine += fromRowStride;
                outputLine += toRowStride;
            }
        }
#if 0
        if(verbose)
        {
            printBuffer(toBuffer, columnCount, rowCount);
        }
#endif
    }
    
    void GPUContextCTM::setTextureData( TextureHandle inTexture,
                                        const float* inData,
                                        size_t inStrideBytes,
                                        size_t inComponentCount,
                                        unsigned int inRank,
                                        const unsigned int* inDomainMin,
                                        const unsigned int* inDomainMax,
                                        const unsigned int* inExtents,
                                        bool inUsesAddressTranslation )
    {

        CTMAssert(!inUsesAddressTranslation, "Address translation unsupported!\n");
        
        CTMStream* texture = (CTMStream*)inTexture;

        int domainWidth;
        int domainHeight;
        int offsetX;
        int offsetY;
        
        if(inRank == 1)
        {
            domainWidth  = inDomainMax[0]-inDomainMin[0];
            domainHeight = 1;
            offsetX = inDomainMin[0]*texture->getInternalComponents()*texture->getComponentSize();
            offsetY = 0;
        }
        else
        {
            domainWidth  = inDomainMax[1]-inDomainMin[1];
            domainHeight = inDomainMax[0]-inDomainMin[0]; 
            offsetX = inDomainMin[1]*texture->getInternalComponents()*texture->getComponentSize();
            offsetY = inDomainMin[0]*texture->getInternalComponents()*texture->getComponentSize();        
        }
        
        // Save the memory allocated on device... will pop at the end of this routine to free this
        // temporary buffer
        _device->pushMemory();

        CTM::Buffer *inputBuf = _device->alloc(CTM::MEM_SYS, 
                                               texture->getWidth(), 
                                               texture->getHeight(), 
                                               texture->getInternalFormat(), 
                                               texture->getInternalTiling());
#ifdef DEBUG
        fprintf(stderr, "setTextureData: Allocated SYS buffer %p of size %d, %d\n", inputBuf, texture->getWidth(), texture->getHeight());
#endif
        
        unsigned char *input = (unsigned char*)inputBuf->getCPUAddress();
        
        memset(input, 0, texture->getWidth()*texture->getHeight()*texture->getInternalComponents()*texture->getComponentSize());

        // copy the data from application memory to PCIe SYS memory arena first        
        copyData(input+(offsetY*texture->getWidth())+offsetX,
                 texture->getWidth()*texture->getInternalComponents()*texture->getComponentSize(),
                 texture->getInternalComponents()*texture->getComponentSize(),
                 inData,
                 domainWidth*inStrideBytes,
                 inStrideBytes,
                 domainWidth,
                 domainHeight,
                 texture->getComponents(),
                 texture->getComponentSize());

        // Set parameters - this is being ignored for the copy passes right now
        // but will be used in the drawRectangle code
        _params[0] = 0.0f;
        _params[1] = 0.0f;
        _params[2] = 0.0f;
        _params[3] = 1.0f;
        
        _params[ParamOffset + 0] = 3.0f * texture->getWidth();
        _params[ParamOffset + 1] = 0.0f;
        _params[ParamOffset + 2] = 0.0f;
        _params[ParamOffset + 3] = 1.0f;
        
        _params[2 * ParamOffset + 0] = 0.0f;
        _params[2 * ParamOffset + 1] = 3.0f * texture->getHeight();
        _params[2 * ParamOffset + 2] = 0.0f;
        _params[2 * ParamOffset + 3] = 1.0f;
        
        // setup interpolants
        float *interpAddressCPU = (float *)_interp->getCPUAddress();
        
        for( int i=0; i<ParamOffset*3; i++)
            ((float*) interpAddressCPU)[i] = _params[i];
        
        if(verbose)
            fprintf(stderr,"GPUContextCTM::setTextureData: Executing copy to GPU buffer %p\n", texture->getBuffer());
        
#if 0
        if(verbose)
        {
            printBuffer(texture->getBuffer()->getCPUAddress(), texture->getWidth(), texture->getHeight());
        }
#endif

         // Copy the data from the given input buffer to the output buffer on the GPU
         CTM::Buffer *outputBuf = texture->getBuffer();
        _device->copy(1, &inputBuf, 1, &outputBuf);
 
        // Restore memory allocated till the previous push
        _device->popMemory();
        
        //xxx Not freeing this right now due to a limitation of CTM::Pass' StateVector mechanism        
//        delete inputBuf;
    }
    
    void GPUContextCTM::getTextureData( TextureHandle inTexture,
                                        float* outData,
                                        size_t inStrideBytes,
                                        size_t inComponentCount,
                                        unsigned int inRank,
                                        const unsigned int* inDomainMin,
                                        const unsigned int* inDomainMax,
                                        const unsigned int* inExtents,
                                        bool inUsesAddressTranslation )
    {
        CTMStream* texture = (CTMStream*)inTexture;

        CTMAssert(!inUsesAddressTranslation, "Address translation unsupported!\n");

        _device->pushMemory();
        
        // allocate memory for output buffer in PCIe memory
        CTM::Buffer *outputBuf = _device->alloc(CTM::MEM_SYS, 
                                                texture->getWidth(), 
                                                texture->getHeight(), 
                                                texture->getInternalFormat(), 
                                                texture->getInternalTiling());
#ifdef DEBUG
        fprintf(stderr, "getTextureData: Allocated SYS buffer %p of size %d, %d\n", outputBuf, texture->getWidth(), texture->getHeight());
#endif
                                                
        unsigned char *output = (unsigned char*)outputBuf->getCPUAddress();

        memset(output, 0, texture->getWidth()*texture->getHeight()*texture->getInternalComponents()*texture->getComponentSize());

        // Set parameters - this is being ignored for the copy passes right now
        // but will be used in the drawRectangle code
        _params[0] = 0.0f;
        _params[1] = 0.0f;
        _params[2] = 0.0f;
        _params[3] = 1.0f;
        
        _params[ParamOffset + 0] = 3.0f * texture->getWidth();
        _params[ParamOffset + 1] = 0.0f;
        _params[ParamOffset + 2] = 0.0f;
        _params[ParamOffset + 3] = 1.0f;
       
        _params[2 * ParamOffset + 0] = 0.0f;
        _params[2 * ParamOffset + 1] = 3.0f * texture->getHeight();
        _params[2 * ParamOffset + 2] = 0.0f;
        _params[2 * ParamOffset + 3] = 1.0f;
        
        // setup interpolants
        float *interpAddressCPU = (float *)_interp->getCPUAddress();
        for( int i=0; i<ParamOffset * 3; i++)
            ((float*) interpAddressCPU)[i] = _params[i];
        
        if(verbose)
            fprintf(stderr,"GPUContextCTM::getTextureData: Executing buffer %p copy back to SYS\n", texture->getBuffer());
        
         // Copy the data from the given texture on the GPU to the SYS memory buffer
        CTM::Buffer *inputBuf = texture->getBuffer();       
        _device->copy(1, &inputBuf, 1, &outputBuf);

        // Pops the memory allocated in this routine
        _device->popMemory();

        //xxx Not freeing this right now due to a limitation of CTM::Pass' StateVector mechanism        
//        delete outputBuf;
        int domainWidth;
        int domainHeight;
        int offsetX;
        int offsetY;
        
        if(inRank == 1)
        {
            domainWidth  = inDomainMax[0]-inDomainMin[0];
            domainHeight = 1;
            offsetX = inDomainMin[0]*texture->getInternalComponents()*texture->getComponentSize();
            offsetY = 0;
        }
        else
        {
            domainWidth  = inDomainMax[1]-inDomainMin[1];
            domainHeight = inDomainMax[0]-inDomainMin[0]; 
            offsetX = inDomainMin[1]*texture->getInternalComponents()*texture->getComponentSize();
            offsetY = inDomainMin[0]*texture->getInternalComponents()*texture->getComponentSize();        
        }
        
        // copy the data from PCIe SYS memory arena to application memory
        copyData(outData,
                 domainWidth*inStrideBytes,
                 inStrideBytes,
                 output+offsetY*texture->getWidth()+offsetX,
                 texture->getWidth()*texture->getInternalComponents()*texture->getComponentSize(),
                 texture->getInternalComponents()*texture->getComponentSize(),
                 domainWidth,
                 domainHeight,
                 texture->getComponents(),
                 texture->getComponentSize());
    }
 
    //! Create pixel shader given the PS3 string   
    GPUContextCTM::PixelShaderHandle GPUContextCTM::createPixelShader( const char* inSource )
    {
        // Tell compiler what we are about to hand it
        // Compile program
        if(verbose)
            fprintf(stderr, "GPUContextCTM::createPixelShader(): Creating new pixel shader\n");

        // We are going to allocate new memory here for the new program. If so, disallow 
        // this pop otherwise the instructions will be lost. 
        if(_pushedInBegin)
            _canPopMemory = false;

        return (PixelShaderHandle)_device->compileProgram(inSource, CTM::PROG_PS3);
    }
    
    //! Get the vertex shader handle - this will not have any meaning in CTM
    GPUContextCTM::VertexShaderHandle GPUContextCTM::getPassthroughVertexShader( const char* inShaderFormat ) {
        return _passthroughVertexShader;
    }
    
    //! Get the pixel shader handle
    GPUContextCTM::PixelShaderHandle GPUContextCTM::getPassthroughPixelShader( const char* inShaderFormat ) {
        return _passthroughPixelShader;
    }
    
    void GPUContextCTM::beginScene()
    {
        if(verbose)
            fprintf(stderr, "GPUContextCTM::beginScene(): Creating new pass for device %p\n", _device);

        // Push device memory right now and mark that we are allowed to pop this memory
        _device->pushMemory();
        _pushedInBegin = true;
        _canPopMemory = true;

        if(!_pass)
            _pass = new CTM::Pass(_device);
    }
    
    void GPUContextCTM::endScene()
    {
        if(verbose)
            fprintf(stderr, "GPUContextCTM::endScene(): Deleting pass %p for device %p\n", _pass, _device);

        delete _pass;
        _pass = NULL;

        // See if we are allowed to pop the memory for Pass and do so if allowed
        // xxx Note that this is not perfect and ultimately we will run out of device memory
        if(_pushedInBegin && _canPopMemory)
        {
            if(verbose)
                fprintf(stderr, "GPUContextCTM::endScene(): Calling popMemory for device %p\n", _device);

            _device->popMemory();
            _pushedInBegin = false;
            _canPopMemory = false;
        }
    }
    
    
    void GPUContextCTM::bindConstant( PixelShaderHandle /* unused */, 
                                      size_t inIndex,
                                      const float4& inValue )
    {
        float *fconstAddressCPU = (float *)_fconst->getCPUAddress();
        memcpy(((float*)fconstAddressCPU)+4*inIndex, &inValue, sizeof(float)*4);
    }

    void GPUContextCTM::bindTexture( size_t inIndex,
                                     TextureHandle inTexture )
    {
        if(verbose)
            fprintf(stderr, "GPUContextCTM::bindTexture: Input %d is Buffer %p\n", inIndex, ((CTMStream*)inTexture)->getBuffer());

        if(!_pass)
            _pass = new CTM::Pass(_device);
        _pass->setInput( inIndex, ((CTMStream*)inTexture)->getBuffer());
    }
    
    void GPUContextCTM::bindOutput( size_t inIndex, TextureHandle inTexture )
    {
        if(verbose)
            fprintf(stderr, "GPUContextCTM::bindOutput: Output %d is Buffer %p\n", inIndex, ((CTMStream*)inTexture)->getBuffer());

        if(!_pass)
            _pass = new CTM::Pass(_device);
        _pass->setOutput( inIndex, ((CTMStream*)inTexture)->getBuffer());
    }
    
    void GPUContextCTM::bindPixelShader( PixelShaderHandle inPixelShader )
    {
        if(!_pass)
            _pass = new CTM::Pass(_device);
        _pass->setProgram((CTM::Buffer *)inPixelShader);
    }
    
    void GPUContextCTM::bindVertexShader( VertexShaderHandle inVertexShader )
    {
        //CTMWARN <<  "bindVertexShader doesn't mean anything in CTM\n";
    }
    
    void GPUContextCTM::disableOutput( size_t inIndex )
    {
        //CTMWARN << "disableoutput doesn't mean anything in CTM\n";       
    }

    void GPUContextCTM::drawRectangle(
        const GPURegion& inOutputRegion, 
        const GPUInterpolant* inInterpolants, 
        unsigned int inInterpolantCount )
    {
    
        if(!_pass)
            _pass = new CTM::Pass(_device);

        unsigned int minX = inOutputRegion.viewport.minX;
        unsigned int minY = inOutputRegion.viewport.minY;
        unsigned int maxX = inOutputRegion.viewport.maxX;
        unsigned int maxY = inOutputRegion.viewport.maxY;

#ifdef DEBUG_INTERP
        fprintf(stderr, "In drawRectangle - inInterpolantCount = %d\n", inInterpolantCount);
#endif

        // The guts of drawing should be here
        CTMAssert( inInterpolantCount <= 8,
                   "Can't have more than 8 texture coordinate interpolators" );

        _pass->setFloatConst(_fconst);
        _pass->setIntegerConst(_iconst);

        // Set domain of output
        _pass->setDomain( minX, minY, maxX, maxY );

        // Setup interpolants here
        float *interpAddressCPU = (float *)_interp->getCPUAddress();
        float* paramsptr = &_params[0];
        for( size_t i = 0; i < 3; i++ )
        {
            float4 position = inOutputRegion.vertices[i];

            *paramsptr++ = position.x;
            *paramsptr++ = position.y;
            *paramsptr++ = 0.0f;
            *paramsptr++ = 1.0f;
            
            GPULOGPRINT(4) << "v[" << i << "] pos=<" << position.x << ", " << position.y << ">";
            
            for( size_t t = 0; t < inInterpolantCount; t++ )
            {
                *paramsptr++ = inInterpolants[t].vertices[i].x;
                *paramsptr++ = inInterpolants[t].vertices[i].y;
                *paramsptr++ = inInterpolants[t].vertices[i].z;
                *paramsptr++ = inInterpolants[t].vertices[i].w;
            }
            
            GPULOGPRINT(4) << std::endl;

            paramsptr+= (ParamOffset - 4 - 4 * inInterpolantCount);
        }

        // setup interpolants
        for( int i=0; i<ParamOffset * 3; i++){
#ifdef DEBUG_INTERP
            if(i%4==0)
                fprintf(stderr, "\n");
            if(i%ParamOffset == 0)
                fprintf(stderr, "\n");
            fprintf(stderr, "%3.1f, ", _params[i]);
#endif
            ((float*) interpAddressCPU)[i] = _params[i];
        }
#ifdef DEBUG_INTERP
        fprintf(stderr, "\n");
        fflush(stderr);
#endif
 
        // Set interpolants using params in Pass
        _pass->setParams(_interp);

        if(verbose)
            fprintf(stderr,"GPUContextCTM::drawRectangle: Executing actual Brook kernel\n");
        
        if(!_pass->execute())
        {
            fprintf(stderr, "Error occured while executing program \n");
        }

        delete _pass;
        _pass = NULL;
     }
}
