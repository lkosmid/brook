// ctmruntime.cpp
#include "ctmruntime.hpp"
#include "ctmstream.hpp"

#define ALIGN 2048

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
    
    struct CTMVertex
    {
        float4 position;
        float4 texcoords[7];
    };
    
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
    
    GPUContextCTM* GPUContextCTM::create( void* inContextValue )
    {
        GPUContextCTM* result = new GPUContextCTM();
        if( result->initialize( inContextValue ) )
            return result;
        delete result;
        return NULL;
    }
    
    GPUContextCTM::GPUContextCTM()
    {
    }
    
    GPUContextCTM::~GPUContextCTM()
    {
    }
    

    bool GPUContextCTM::initialize( void* inContextValue )
    {
        memset(&params, 0, sizeof(params));
        
        if( inContextValue == NULL )
        {
            // open board and create resources
            char pcie[ 32 ];
            
            sprintf( pcie, "/dev/pcie%d", 0 );
            AMmanagedDeviceInfo _info = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            info = _info;
            vm = amOpenManagedConnection( pcie, &info );
            if(vm == NULL)
            {
                fprintf(stderr, "Fatal: can't open board\n");
                return false;
            }
            
            printf("GPU Memory     available: %dMB\n",
                   info.arenaSizeGPU/(1024*1024));
            printf("GPU Memory (B) available: %dMB\n",
                   info.arenaSizeGPUb/(1024*1024));
            printf("SYS            available: %dMB\n",
                   info.arenaSizeSYS/(1024*1024));
            printf("SYSc           available: %dMB\n",
                   info.arenaSizeSYSc/(1024*1024));
            printf("\n");

            fflush(stdout);
 
            // Use the larger of the cached and uncached arenas
            // Currently, on Linux, only cached arenas are valid with CTM
            bool cached = info.arenaSizeSYSc > info.arenaSizeSYS;
            arenaSizeSYS        = cached ? info.arenaSizeSYSc : info.arenaSizeSYS;
            baseAddressSYS      = cached ? info.baseAddressSYSc : info.baseAddressSYS;
            baseAddressCPU      = cached ? info.baseAddressCPUc : info.baseAddressCPU;            
            currentAddressGPU   = info.baseAddressGPU;
            
            cbufAddressSYS = baseAddressSYS + 0 * 1024 * 1024;
            cbufAddressCPU = getAddressCPU( cbufAddressSYS, cached, info );
            
            programAddressSYS = baseAddressSYS + 1 * 1024 * 1024;
            programAddressCPU = getAddressCPU( programAddressSYS, cached, info);

            
            fconstAddressSYS = baseAddressSYS + 2 * 1024 * 1024;
            iconstAddressSYS = baseAddressSYS + 3 * 1024 * 1024;
            interpAddressSYS = baseAddressSYS + 4 * 1024 * 1024;
            
            fconstAddressCPU = getAddressCPU( fconstAddressSYS, cached, info);
            iconstAddressCPU = getAddressCPU( iconstAddressSYS, cached, info);
            interpAddressCPU = getAddressCPU( interpAddressSYS, cached, info);

            // Allocate largest possible buffers
            bufferSYS  = alignCTMAddress( baseAddressSYS + 5 * 1024 * 1024, ALIGN );
            
            bufferCPU  = getAddressCPU( bufferSYS, cached, info );

            // Get compiler handle
            comp = amuCompOpenCompiler();
            
            copyProgram = new CTMProgram();
            
            AMUcompMacro defines[] =
                {
                    {"AMU_LANG_PS3", "1"},
                    {0, 0}
                };
            
            // Compile program
            if ( amuCompCompile( comp, copy, (unsigned int)strlen(copy),
                                 defines, NULL, "main", &copyProgram->size, &copyProgram->binary,
                                 LogPrintFunction ) == 0 )
            {
                fprintf(stderr, "Found error compiling, bailing out!\n");
                fflush(stderr);
                return false;
            }
            
            copyProgram->constants = amuABIExtractConstants( copyProgram->binary );
            
            // Disassemble what we just built and print it out, just for
            // fun
            amuAsmDisassemble(copyProgram->binary, LogPrintFunction);

            _passthroughPixelShader = createPixelShader(kPassthroughPixelShaderSource);

            cbp = NULL;
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
        return inExtent <= 4096; // MCH: arbitrary and
                                 // hardcoded... bad.  Need way to
                                 // view caps bits
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
            scaleBiasX = -0.5f*((float)domainMax[0]-(float)domainMin[0])/(float)outputWidth+kInterpolantBias;
            scaleBiasY = 0.0f;
        }
        else
        {
            minX = domainMin[1];
            minY = domainMin[0];
            maxX = domainMax[1];
            maxY = domainMax[0];
            scaleBiasX = -0.5f*((float)domainMax[1]-(float)domainMin[1])/(float)outputWidth+kInterpolantBias;
            scaleBiasY = -0.5f*((float)domainMax[0]-(float)domainMin[0])/(float)outputHeight+kInterpolantBias;
        }

        
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
        
        switch( inFormat )
        {
        case kTextureFormat_Float1:
        case kTextureFormat_ShortFixed1:
        case kTextureFormat_Fixed1:
            components = 1;
            break;
        case kTextureFormat_Float2:
        case kTextureFormat_ShortFixed2:
        case kTextureFormat_Fixed2:
            components = 2;
            break;
        case kTextureFormat_Float3:
        case kTextureFormat_ShortFixed3:
        case kTextureFormat_Fixed3:
            components = 3;
            break;
        case kTextureFormat_Float4:
        case kTextureFormat_ShortFixed4:
        case kTextureFormat_Fixed4:
            components = 4;
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

        // Assign a pointer for this and update GPU pointer
        // FIXME: Need to check for space and switch to GPUB as needed
        CTMStream* result = CTMStream::create( this, inWidth, inHeight, components,
                                               componentType, read_only, currentAddressGPU);
        currentAddressGPU = alignCTMAddress( currentAddressGPU+result->getAllocatedSize()+8192, ALIGN);
        
        return result;
    }
    
    
    void GPUContextCTM::releaseTexture( TextureHandle inTexture )
    {
        CTMStream* texture = (CTMStream*)inTexture;
        delete texture;
    }
    
    void GPUContextCTM::copyData( void* toBuffer, size_t toRowStride,  size_t toElementStride,
                                  const void* fromBuffer, size_t fromRowStride, size_t fromElementStride,
                                  size_t columnCount, size_t rowCount, 
                                  size_t numElements,size_t elementSize )
    {
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
        
        unsigned char *input = NULL;
        input  = (unsigned char*)bufferCPU;
        
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
        
        memset(input, 0, texture->getWidth()*texture->getHeight()*texture->getInternalComponents()*texture->getComponentSize());
        
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
        
        assert(cbp == NULL);
        cbp = new AMUcbufPack( ( unsigned int* ) cbufAddressCPU, 1024 * 256 );
        
        // Setup program literal constants
        amuABISetLiteralConstants( copyProgram->constants,
                                   fconstAddressCPU,
                                   iconstAddressCPU,
                                   bconstAddressCPU );
        
        // Flush Output cache
        cbp->appendFlushOutCache();
        
        // Flush conditional output cache
        cbp->appendFlushCondOutCache();
        
        
        // Notice we are switching from a tiled format on the GPU to a linear in system memory
        cbp->appendSetInpFmt( 0, bufferSYS,
                              texture->getInternalFormat(), AMU_CBUF_FLD_TILING_LINEAR,
                              texture->getWidth(), texture->getHeight() );
        
        // Setup output pointer and format (float4 with linear format)
        cbp->appendSetOutFmt( 0, texture->GPUAddress,
                              texture->getInternalFormat(), texture->getInternalTiling(),
                              texture->getWidth(), texture->getHeight() );
        
        // Invalidate input cache
        cbp->appendInvInpCache();
        
        // Invalidate conditional output cache
        cbp->appendInvCondOutCache();
        
        // Set instruction format
        // copy program in place
        memcpy( programAddressCPU, copyProgram->binary, copyProgram->size );
        cbp->appendSetInstFmt( programAddressSYS, 0, 0, 0 );
        cbp->appendInvInstCache();
        
        // Setup constants and invalidate constant caches
        
        // Floats
        cbp->appendSetConstfFmt( fconstAddressSYS, 0, 0, 0 );
        cbp->appendInvConstfCache();
        // Integers
        cbp->appendSetConstiFmt( iconstAddressSYS, 0, 0, 0 );
        cbp->appendInvConstiCache();

        // Set domain of output
        int startx, endx, starty, endy;
        if(inRank == 1)
        {
            startx = inDomainMin[0];
            endx   = inDomainMax[0]-1;
            starty = 0;
            endy   = 0;
            
        }
        else
        {
            startx = inDomainMin[1];
            endx   = inDomainMax[1]-1;
            starty = inDomainMin[0];
            endy   = inDomainMax[0]-1;
            
        }
        cbp->appendSetDomain( startx, starty, endx, endy );
        
        params[0] = 0.0f;
        params[1] = 0.0f;
        params[2] = 0.0f;
        params[3] = 1.0f;
        
        params[32] = 3.0f+texture->getWidth();
        params[33] = 0.0f;
        params[34] = 0.0f;
        params[35] = 1.0f;
        
        params[64] = 0.0f;
        params[65] = 3.0f*texture->getHeight();
        params[66] = 0.0f;
        params[67] = 1.0f;
        
        // setup interpolants
        for( int i=0; i<32*3; i++)
            ((float*) interpAddressCPU)[i] = params[i];
        
        cbp->appendSetParams( interpAddressSYS );
        
        // After setting everything up, start program
        cbp->appendStartProgram();
        
        // Flush Output cache
        cbp->appendFlushOutCache();
        
        // Flush conditional output cache
        cbp->appendFlushCondOutCache();
        
        // Get a copy of command buffer size
        AMuint32 cbufsize = cbp->getCommandBufferSize();
        assert(cbufsize <=1024*256);
        delete cbp;
        cbp = NULL;
        
        // Submit buffer to hardware
        unsigned int bufid = amSubmitCommandBuffer( vm, cbufAddressSYS, cbufsize );
        if(!bufid){
            fprintf(stderr, "Command buffer returned an error!\n");
        }
        
        // Query if buffer has been sent
        while ( amCommandBufferConsumed( vm, bufid ) == 0 )
        {
            //Do other stuff here in until GPU is done
            //NOTE: currently this is a blocking command
        }
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

        unsigned char *output = NULL;
        output  = (unsigned char*)bufferCPU;

        memset(output, 0, texture->getWidth()*texture->getHeight()*texture->getInternalComponents()*texture->getComponentSize());

        assert(cbp == NULL);
        
        cbp = new AMUcbufPack( ( unsigned int* ) cbufAddressCPU, 1024 * 256 );
        
        // Setup program literal constants
        amuABISetLiteralConstants( copyProgram->constants,
                                   fconstAddressCPU,
                                   iconstAddressCPU,
                                   bconstAddressCPU );
        
        // Flush Output cache
        cbp->appendFlushOutCache();
        
        // Flush conditional output cache
        cbp->appendFlushCondOutCache();
        
        // Notice we are switching from a tiled format on the GPU to a linear in system memory
        cbp->appendSetInpFmt( 0, texture->GPUAddress,
                              texture->getInternalFormat(), texture->getInternalTiling(),
                              texture->getWidth(), texture->getHeight() );
        
        // Setup output pointer and format (float4 with linear format)
        cbp->appendSetOutFmt( 0, bufferSYS,
                              texture->getInternalFormat(), AMU_CBUF_FLD_TILING_LINEAR,
                              texture->getWidth(), texture->getHeight() );
        
        // Invalidate input cache
        cbp->appendInvInpCache();
        
        // Invalidate conditional output cache
        cbp->appendInvCondOutCache();
        
        // Set instruction format
        // copy program in place
        memcpy( programAddressCPU, copyProgram->binary, copyProgram->size ); 
        cbp->appendSetInstFmt( programAddressSYS, 0, 0, 0 );
        cbp->appendInvInstCache();
        
        // Setup constants and invalidate constant caches
        
        // Floats
        cbp->appendSetConstfFmt( fconstAddressSYS, 0, 0, 0 );
        cbp->appendInvConstfCache();
        // Integers
        cbp->appendSetConstiFmt( iconstAddressSYS, 0, 0, 0 );
        cbp->appendInvConstiCache();

        // Set domain of output
        int startx, endx, starty, endy;
        if(inRank == 1)
        {
            startx = inDomainMin[0];
            endx   = inDomainMax[0]-1;
            starty = 0;
            endy   = 0;
            
        }
        else
        {
            startx = inDomainMin[1];
            endx   = inDomainMax[1]-1;
            starty = inDomainMin[0];
            endy   = inDomainMax[0]-1;
            
        }
        cbp->appendSetDomain( startx, starty, endx, endy );
        
        
        params[0] = 0.0f;
        params[1] = 0.0f;
        params[2] = 0.0f;
        params[3] = 1.0f;

        
        params[32] = 3.0f*texture->getWidth();
        params[33] = 0.0f;
        params[34] = 0.0f;
        params[35] = 1.0f;

        
        params[64] = 0.0f;
        params[65] = 3.0f*texture->getHeight();
        params[66] = 0.0f;
        params[67] = 1.0f;
        
        // setup interpolants
        for( int i=0; i<32*3; i++)
            ((float*) interpAddressCPU)[i] = params[i];
        
        cbp->appendSetParams( interpAddressSYS );
        
        // After setting everything up, start program
        cbp->appendStartProgram();
        
        // Flush Output cache
        cbp->appendFlushOutCache();
        
        // Flush conditional output cache
        cbp->appendFlushCondOutCache();
        
        // Get a copy of command buffer size
        AMuint32 cbufsize = cbp->getCommandBufferSize();
        assert(cbufsize <=1024*256);
        delete cbp;
        cbp = NULL;
        
        // Submit buffer to hardware
        unsigned int bufid = amSubmitCommandBuffer( vm, cbufAddressSYS, cbufsize );
        if(!bufid){
            fprintf(stderr, "Command buffer returned an error!\n");
        }
        
        // Query if buffer has been sent
        while ( amCommandBufferConsumed( vm, bufid ) == 0 )
        {
            //Do other stuff here in until GPU is done
            //NOTE: currently this is a blocking command
            printf(".");
        }
        
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
    
    GPUContextCTM::PixelShaderHandle GPUContextCTM::createPixelShader( const char* inSource )
    {
        // Tell compiler what we are about to hand it
        AMUcompMacro defines[] =
        {
            {"AMU_LANG_PS3", "1"},
            {0, 0}
        };
        
        CTMProgram* program = new CTMProgram();

        // Compile program
        if ( amuCompCompile( comp, inSource, (unsigned int)strlen(inSource),
                             defines, NULL, "main", &program->size, &program->binary,
                             LogPrintFunction ) == 0 )
        {
            fprintf(stderr, "Found error compiling, bailing out!\n");
            assert(0);
            return NULL;
        }
        
        program->constants = amuABIExtractConstants( program->binary );
        
        // Disassemble what we just built and print it out, just for
        // fun
        amuAsmDisassemble(program->binary, LogPrintFunction);

        return (PixelShaderHandle)program;
    }
    
    GPUContextCTM::VertexShaderHandle GPUContextCTM::getPassthroughVertexShader( const char* inShaderFormat ) {
        return _passthroughVertexShader;
    }
    
    GPUContextCTM::PixelShaderHandle GPUContextCTM::getPassthroughPixelShader( const char* inShaderFormat ) {
        return _passthroughPixelShader;
    }
    
    void GPUContextCTM::beginScene()
    {
        assert(cbp == NULL);
        cbp = new AMUcbufPack( ( unsigned int* ) cbufAddressCPU, 1024 * 256 );
    }
    
    void GPUContextCTM::endScene()
    {
        assert(cbp);
        delete cbp;
        cbp = NULL;
    }
    
    
    void GPUContextCTM::bindConstant( PixelShaderHandle /* unused */, 
                                      size_t inIndex,
                                      const float4& inValue )
    {
        memcpy(((float*)fconstAddressCPU)+4*inIndex, &inValue, sizeof(float)*4);
    }

    void GPUContextCTM::bindTexture( size_t inIndex,
                                     TextureHandle inTexture )
    {
        CTMStream* texture = (CTMStream*)inTexture;
        assert(cbp);
        cbp->appendSetInpFmt( inIndex, texture->GPUAddress,
                              texture->getInternalFormat(), texture->getInternalTiling(),
                              texture->getWidth(), texture->getHeight() );
        // Invalidate input cache
        cbp->appendInvInpCache();
    }
    
    void GPUContextCTM::bindOutput( size_t inIndex, TextureHandle inTexture )
    {
        CTMStream* texture = (CTMStream*)inTexture;
        assert(cbp);
        cbp->appendSetOutFmt( inIndex, texture->GPUAddress,
                              texture->getInternalFormat(), texture->getInternalTiling(),
                              texture->getWidth(), texture->getHeight() );
    }
    
    void GPUContextCTM::bindPixelShader( PixelShaderHandle inPixelShader )
    {
        // Setup program literal constants
        CTMProgram* program = (CTMProgram*) inPixelShader;
        amuABISetLiteralConstants( program->constants,
                                   fconstAddressCPU,
                                   iconstAddressCPU,
                                   bconstAddressCPU );
        // copy program in place
        memcpy( programAddressCPU, program->binary, program->size );
        assert(cbp);
        cbp->appendSetInstFmt( programAddressSYS, 0, 0, 0 );
        cbp->appendInvInstCache();
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

        unsigned int minX = inOutputRegion.viewport.minX;
        unsigned int minY = inOutputRegion.viewport.minY;
        unsigned int maxX = inOutputRegion.viewport.maxX;
        unsigned int maxY = inOutputRegion.viewport.maxY;
        
        // The guts of drawing should be here
        CTMAssert( inInterpolantCount <= 7,
                   "Can't have more than 7 texture coordinate interpolators" );
        
        // Invalidate conditional output cache
        assert(cbp);
        cbp->appendInvCondOutCache();
        
        // Setup constants and invalidate constant caches
        // Floats
        cbp->appendSetConstfFmt( fconstAddressSYS, 0, 0, 0 );
        cbp->appendInvConstfCache();
        
        // Integers
        cbp->appendSetConstiFmt( iconstAddressSYS, 0, 0, 0 );
        cbp->appendInvConstiCache();

        // Need to setup interpolants still
        
        // Set domain of output
        cbp->appendSetDomain( minX, minY, maxX, maxY );

        float* paramsptr = params;
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

            paramsptr+= (32-4-4*inInterpolantCount);
        }

        // setup interpolants
        for( int i=0; i<32*3; i++){
#ifdef DEBUG
            if(i%4==0)
                fprintf(stderr, "\n");
            if(i%32==0)
                fprintf(stderr, "\n");
            fprintf(stderr, "%3.1f, ", params[i]);
#endif
            ((float*) interpAddressCPU)[i] = params[i];
        }
#ifdef DEBUG
        fprintf(stderr, "\n");
        fflush(stderr);
#endif`
        
        cbp->appendSetParams( interpAddressSYS );
        
        // After setting everything up, start program
        cbp->appendStartProgram();
        
        // Flush Output cache
        cbp->appendFlushOutCache();
        
        // Get a copy of command buffer size
        AMuint32 cbufsize = cbp->getCommandBufferSize();
        assert(cbufsize <=1024*256);
        
        assert(cbp);
        delete cbp;
        cbp = NULL;
        
        // Submit buffer to hardware
        unsigned int bufid = amSubmitCommandBuffer( vm, cbufAddressSYS, cbufsize );
        if(!bufid){
            fprintf(stderr, "Command buffer returned an error!\n");
        }
        
        // Query if buffer has been sent
        while ( amCommandBufferConsumed( vm, bufid ) == 0 )
        {
            //Do other stuff here in until GPU is done
            //NOTE: currently this is a blocking command
        }
        assert(cbp == NULL);
        cbp = new AMUcbufPack( ( unsigned int* ) cbufAddressCPU, 1024 * 256 );
     }
}
