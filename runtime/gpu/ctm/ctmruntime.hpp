#pragma once

#include <assert.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// DEFINE path somehow
#include "amDeviceManaged.h"
#include "amuComp.h"
#include "amuABI.h"
#include "amuCbufPack.h"
#include "amuCbufTypes.h"
#include "amuAsm.h"

#include "../gpubase.hpp"
#include "../gpucontext.hpp"
#include "../gpuruntime.hpp"

const static char copy[] =
"    ps_3_0            \n"
"    dcl vPos.xy       \n"
"    dcl_2d s0         \n"
"    texld r0, vPos, s0\n"
"    mov oC0.xyzw, r0  \n"
"    end               \n"
;

#define CTM_RUNTIME_STRING "ctm"

namespace brook
{

    class CTMProgram
    {
    public:
        AMuint32 size;
        void* binary;
        AMUabiConstsInfo constants;

        CTMProgram()
        {
            size = 0;
            binary = NULL;
        }

        ~CTMProgram()
        {
            // should free binary to be nice
        }
    };
    
    class GPURuntimeCTM : public GPURuntime
    {
    public:
        static GPURuntimeCTM* create( void* inContextValue = 0 );
        
    private:
        GPURuntimeCTM() {}
    };
    
    class GPUContextCTM : public GPUContext
    {
    public:
        static GPUContextCTM* create( void* inContextValue );
        GPUContextCTM();
        ~GPUContextCTM();
        
        int getShaderFormatRank (const char *name) const;
        
        /* Test to see if we can create a texture with
        ** a dimension of this size
        */
        bool isTextureExtentValid( unsigned int inExtent ) const;   
        
        /* Returns the number of shader outputs supported by
        ** the hardware.
        */
        unsigned int getMaximumOutputCount() const { return 4; }
        
        float4 getStreamIndexofConstant( TextureHandle inTexture ) const;
        float4 getStreamGatherConstant( unsigned int inRank,
                                        const unsigned int* inDomainMin,
                                        const unsigned int* inDomainMax,
                                        const unsigned int* inExtents ) const;
        void get1DInterpolant( const float4 &start, 
                               const float4 &end,
                               const unsigned int outputWidth,
                               GPUInterpolant &interpolant) const;
        
        void get2DInterpolant( const float2 &start, 
                               const float2 &end,
                               const unsigned int outputWidth,
                               const unsigned int outputHeight, 
                               GPUInterpolant &interpolant) const;
        
        void getStreamInterpolant( const TextureHandle texture,
                                   unsigned int rank,
                                   const unsigned int* domainMin,
                                   const unsigned int* domainMax,
                                   const unsigned int outputWidth,
                                   const unsigned int outputHeight, 
                                   GPUInterpolant &interpolant) const;
        
        void getStreamOutputRegion( const TextureHandle texture,
                                    unsigned int rank,
                                    const unsigned int* domainMin,
                                    const unsigned int* domainMax,
                                    GPURegion &region) const; 
        
        // for reductions
        void getStreamReduceInterpolant( const TextureHandle texture,
                                         const unsigned int outputWidth,
                                         const unsigned int outputHeight, 
                                         const unsigned int minX,
                                         const unsigned int maxX, 
                                         const unsigned int minY,
                                         const unsigned int maxY,
                                         GPUInterpolant &interpolant) const;
        
        void getStreamReduceOutputRegion( const TextureHandle texture,
                                          const unsigned int minX,
                                          const unsigned int maxX, 
                                          const unsigned int minY,
                                          const unsigned int maxY,
                                          GPURegion &region) const;
        
        /* Create a texture */
        TextureHandle createTexture2D( unsigned int inWidth, 
                                       unsigned int inHeight, 
                                       TextureFormat inFormat, 
                                       bool read_only=false );
        
        /* Free a texture */
        void releaseTexture( TextureHandle inTexture );
        
        /* Load up the texture with data */
        void setTextureData( TextureHandle inTexture, 
                             const float* inData, 
                             unsigned int inStrideBytes, 
                             unsigned int inElemCount,
                             unsigned int inRank,
                             const unsigned int* inDomainMin,
                             const unsigned int* inDomainMax,
                             const unsigned int* inExtents,
                             bool inUsesAddressTranslation );
        
        /* Fetch the texture data */
        void getTextureData( TextureHandle inTexture, 
                             float* outData, 
                             unsigned int inStrideBytes, 
                             unsigned int inElemCount,
                             unsigned int inRank,
                             const unsigned int* inDomainMin,
                             const unsigned int* inDomainMax,
                             const unsigned int* inExtents,
                             bool inUsesAddressTranslation );
        
        /* Create a pixel shader */
        PixelShaderHandle createPixelShader( const char* inSource );
        
        /* Handles to the passthrough shaders */
        VertexShaderHandle
        getPassthroughVertexShader( const char* inShaderFormat = NULL );
        
        PixelShaderHandle
        getPassthroughPixelShader( const char* inShaderFormat = NULL );
        
        /* To use around rendering operations */
        void beginScene();
        void endScene();
        
        /* Binding commands */
        void bindConstant( PixelShaderHandle ps, 
                           unsigned int inIndex, 
                           const float4& inValue );
        void bindTexture( unsigned int inIndex, 
                          TextureHandle inTexture );
        void bindOutput( unsigned int inIndex, 
                         TextureHandle inSurface );
        void bindPixelShader( PixelShaderHandle inPixelShader );
        void bindVertexShader( VertexShaderHandle inVertexShader );
        
        /* Turn off a shader output */
        void disableOutput( unsigned int inIndex );
        
        void setOutputDomainMode( bool inUsingOutputDomain ){}
        void setAddressTranslationMode( bool inUsingAddressTranslation ){}
        
        /* Issue a shader */
        void drawRectangle( const GPURegion &outputRegion, 
                            const GPUInterpolant *interpolants, 
                            unsigned int numInterpolants );
        
        /* hacky functions for rendering - will be deprecated soon */
        void* getTextureRenderData( TextureHandle inTexture )
        {
            GPUError("getTextureRenderData unimplemented"); throw 1;
        }
        
        void synchronizeTextureRenderData( TextureHandle inTexture )
        {
            GPUError("synchronizeTextureRenderData unimplemented"); throw 1;
        }
        
        // new entry points needed by C++ brook API
        void finish(){ GPUError("finish unimplemented"); throw 1; }
        void unbind(){ GPUError("unbind unimplemented"); throw 1; }
        void bind()  { GPUError("bind unimplemented");   throw 1; }

    private:
        bool initialize( void* inContextValue );
        bool _shouldBiasInterpolants;
        VertexShaderHandle _passthroughVertexShader;
        PixelShaderHandle _passthroughPixelShader;

        // Device access
        AMmanagedDeviceInfo info;
        AMmanagedDevice vm;
        
        // Command buffer:
        AMuint32 cbufAddressSYS;
        void*    cbufAddressCPU;
        
        // Compiled Programs:
        AMuint32 programAddressSYS;
        void*    programAddressCPU;
        
        // Program constants
        AMUabiConstsInfo programConstants;
        
        // Constant storage:
        AMuint32 fconstAddressSYS;
        void*    fconstAddressCPU;
        AMuint32 iconstAddressSYS;
        void*    iconstAddressCPU;
        AMuint32 bconstAddressSYS;
        void*    bconstAddressCPU;
        AMuint32 interpAddressSYS;
        void*    interpAddressCPU;

        // Input/Output buffer to use for data copy
        AMuint32 bufferSYS;
        void*    bufferCPU;

        AMuint32 arenaSizeSYS;
        AMuint32 baseAddressSYS;
        void*    baseAddressCPU;
        AMuint32 baseAddressGPU;
        AMuint32 currentAddressGPU;

        AMUcompCompiler comp;

        CTMProgram* copyProgram;

        AMUcbufPack* cbp;
        
        float params[ 32*3 ];

        // Helper functions for converting SYS addresses in CTM to
        // their often more useful CPU addresses
        void * getAddressCPU(AMuint32 addrSYS, AMmanagedDeviceInfo info)
        {
            int offset = addrSYS - info.baseAddressSYS;
            if(offset >= 0)
            {   
                return (unsigned char*)info.baseAddressCPU + offset;
            }
            else
            {
                offset = addrSYS - info.baseAddressSYSc;
                if(offset >= 0)
                    return (unsigned char*)info.baseAddressCPUc + offset;
            }
            assert(!"Invalid System Address");
            return 0;
        }
        
        // Help get alignment correct
        AMuint32 alignCTMAddress( AMuint32 address, int alignment )
        {
            AMuint32 new_addr = ((address + (alignment-1)) & (0-alignment));
            assert(new_addr % alignment == 0);
            return new_addr;
        }
        
        // Sets user defined constants, i.e. the constants listed in
        // the header of the HLSL output
        static void
        CTMsetUserConstant( AMUabiConstsInfo compOutput, void* addressCPU,
                            const char* name, const void* value, AMUabiDataType type )
        {
            //
            //  Scan through compiler output and find the given user constant
            //
            for ( unsigned int u = 0; u < compOutput.userConstsCount; u++ )
            {
                if ( strcmp( compOutput.userConsts[ u ].name.str, name ) == 0 )
                {
                    if ( compOutput.userConsts[ u ].type == AMU_ABI_FLOAT32 && type == AMU_ABI_FLOAT32 )
                    {
                        //
                        //  Set the user constant's value
                        //
                        float * c = &( ( float* ) addressCPU ) [ 4 * compOutput.userConsts[ u ].addr ];
                        float* v = ( float* ) value;
                        c[ 0 ] = v[ 0 ];
                        c[ 1 ] = v[ 1 ];
                        c[ 2 ] = v[ 2 ];
                        c[ 3 ] = v[ 3 ];
                        return ;
                    }
                    else if ( compOutput.userConsts[ u ].type == AMU_ABI_INT32 && type == AMU_ABI_INT32 )
                    {
                        //
                        //  Set the user constant's value
                        //
                        unsigned char * c = &( ( unsigned char* ) addressCPU ) [ 4 * compOutput.userConsts[ u ].addr ];
                        unsigned char* v = ( unsigned char* ) value;
                        c[ 0 ] = v[ 0 ];
                        c[ 1 ] = v[ 1 ];
                        c[ 2 ] = v[ 2 ];
                        c[ 3 ] = v[ 3 ];
                        return ;
                    }
                    else if ( compOutput.userConsts[ u ].type == AMU_ABI_BOOL32 && type == AMU_ABI_BOOL32 )
                    {
                        //
                        //  Set the user constant's value
                        //
                        unsigned char * c = &( ( unsigned char* ) addressCPU ) [ 4 * compOutput.userConsts[ u ].addr ];
                        unsigned char* v = ( unsigned char* ) value;
                        c[ 0 ] = v[ 0 ];
                        c[ 1 ] = v[ 1 ];
                        c[ 2 ] = v[ 2 ];
                        c[ 3 ] = v[ 3 ];
                        return ;
                    }
                }
            }
        }
        
        // After compiling the PS3.0 shader to an ELF, the constants inlined
        // in the shader are separated into the ELF.  This function loads
        // those values into the constant registers for you.
        void
        CTMsetLiteralConstants( AMUabiConstsInfo compOutput, void* floatConstAddressCPU, void* intConstAddressCPU, void* boolConstAddressCPU )
        {
            //
            // submit all literal values
            //
            float * cf = NULL;
            float* vf = NULL;
            unsigned char* ci = NULL;
            unsigned char* cb = NULL;
            unsigned int *vi = NULL;
            unsigned int *vb = NULL;
            
            for ( unsigned int c = 0; c < compOutput.litConstsCount; c++ )
            {
                switch ( compOutput.litConsts[ c ].type )
                {
                case AMU_ABI_FLOAT32:
                {
                    cf = &( ( float* ) floatConstAddressCPU ) [ 4 * compOutput.litConsts[ c ].addr ];
                    vf = compOutput.litConsts[ c ].value.float32;
                    cf[ 0 ] = vf[ 0 ];
                    cf[ 1 ] = vf[ 1 ];
                    cf[ 2 ] = vf[ 2 ];
                    cf[ 3 ] = vf[ 3 ];
                    break;
                }
                case AMU_ABI_INT32:
                {
                    ci = &( ( unsigned char* ) intConstAddressCPU ) [ 4 * compOutput.litConsts[ c ].addr ];
                    vi = compOutput.litConsts[ c ].value.int32;
                    ci[ 0 ] = vi[ 0 ];
                    ci[ 1 ] = vi[ 1 ];
                    ci[ 2 ] = vi[ 2 ];
                    ci[ 3 ] = vi[ 3 ];
                    break;
                }
                case AMU_ABI_BOOL32:
                {
                    cb = &( ( unsigned char* ) boolConstAddressCPU ) [ 4 * compOutput.litConsts[ c ].addr ];
                    vb = compOutput.litConsts[ c ].value.bool32;
                    cb[ 0 ] = vb[ 0 ];
                    break;
                }
                default:
                    break;
                };
            }
        }
        
        // Log function to hand off to compiler.
        static void
        LogPrintFunction( const char* msg )
        {
#ifdef DEBUG
            printf( stderr, msg );
#endif
        }
        void copyData( void* toBuffer, size_t toRowStride,  size_t toElementStride,
                       const void* fromBuffer, size_t fromRowStride, size_t fromElementStride,
                       size_t columnCount, size_t rowCount, size_t numElements,size_t elementSize );
        
    };
    
#define CTMPROFILE( __name ) \
    GPUPROFILE( __name )
    
#define CTMLOG( __level ) \
    GPULOG( __level )
    
#define CTMLOGPRINT( __level ) \
    GPULOGPRINT( __level )
    
#define CTMWARN GPUWARN
    
#define CTMAssertResult( _result, _message ) \
    if(SUCCEEDED(_result)) {} else GPUError( _message )
    
#define CTMAssert( _condition, _message ) \
    GPUAssert( _condition, _message )
}
