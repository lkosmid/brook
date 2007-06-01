#ifndef _CTM_RUNTIME_H_
#define _CTM_RUNTIME_H_

//! C/C++ includes
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//! Include CTM util headers
#include "CTM/Device.h"
#include "CTM/Buffer.h"
#include "CTM/Pass.h"

//! Include Brook headers
#include "../gpubase.hpp"
#include "../gpucontext.hpp"
#include "../gpuruntime.hpp"

#define CTM_RUNTIME_STRING "ctm"

namespace brook
{

    //! CTM GPU runtime class
    class GPURuntimeCTM : public GPURuntime
    {
    public:
        static GPURuntimeCTM* create( void* inContextValue = 0 );
        
    private:
        GPURuntimeCTM() {}
    };
    
    //! CTM GPU context class
    class GPUContextCTM : public GPUContext
    {
    
    public:
    
        static GPUContextCTM* create( void* inContextValue );
        
        GPUContextCTM();
        ~GPUContextCTM();
        
        int getShaderFormatRank (const char *name) const;
        
        //! Test to see if we can create a texture with
        //! a dimension of this size
        bool isTextureExtentValid( unsigned int inExtent ) const;   
        
        //! Returns the number of shader outputs supported by
        //! the hardware.
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
        
        //! Create a texture - This will allocate a CTM buffer on the GPU memory
        TextureHandle createTexture2D( unsigned int inWidth, 
                                       unsigned int inHeight, 
                                       TextureFormat inFormat, 
                                       bool read_only=false );
        
        //! Free a texture
        void releaseTexture( TextureHandle inTexture );
        
        //! Load up the texture with data - this will mean copying the given 
        //! data buffer in _application memory_ to GPU memory
        void setTextureData( TextureHandle inTexture, 
                             const float* inData, 
                             unsigned int inStrideBytes, 
                             unsigned int inElemCount,
                             unsigned int inRank,
                             const unsigned int* inDomainMin,
                             const unsigned int* inDomainMax,
                             const unsigned int* inExtents,
                             bool inUsesAddressTranslation );
        
        //! Fetch the texture data - this will mean copying the data from
        //! the GPU for the given texture back to the application memory
        void getTextureData( TextureHandle inTexture, 
                             float* outData, 
                             unsigned int inStrideBytes, 
                             unsigned int inElemCount,
                             unsigned int inRank,
                             const unsigned int* inDomainMin,
                             const unsigned int* inDomainMax,
                             const unsigned int* inExtents,
                             bool inUsesAddressTranslation );
        
        //! Create a pixel shader given the string pointer
        PixelShaderHandle createPixelShader( const char* inSource );
        
        //! Handles to the passthrough shaders
        VertexShaderHandle
        getPassthroughVertexShader( const char* inShaderFormat = NULL );
        
        PixelShaderHandle
        getPassthroughPixelShader( const char* inShaderFormat = NULL );
        
        //! Start and end a rendering/computational pass
        void beginScene();
        void endScene();
        
        //! Binding commands
        //! Will be used internally to update/set the device state via Pass object
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
        
        //! Primary kernel invocation routine. Passes the domain on which
        //! to execute the kernel along with parameters like interpolants
        void drawRectangle( const GPURegion &outputRegion, 
                            const GPUInterpolant *interpolants, 
                            unsigned int numInterpolants );
        
        //! xxx hacky functions for rendering - will be deprecated soon
        void* getTextureRenderData( TextureHandle inTexture )
        {
            GPUError("getTextureRenderData unimplemented"); throw 1;
        }
        
        //! xxx what is this supposed to do? 
        void synchronizeTextureRenderData( TextureHandle inTexture )
        {
            GPUError("synchronizeTextureRenderData unimplemented"); throw 1;
        }
        
        //! new entry points needed by C++ brook API
        void finish(){ GPUError("finish unimplemented"); throw 1; }
        void unbind(){ GPUError("unbind unimplemented"); throw 1; }
        void bind()  { GPUError("bind unimplemented");   throw 1; }

        //! Device access routine
        CTM::Device *getDevice() { return _device; }

    private:
    
        //! Internal initialization routine. Creates the device connection and
        //! stores the context internally. 
        bool initialize( void* inContextValue );
        
        //! Vertex and pixel pass through shaders
        //! xxx The vertex shader is not really used and the pixel shader 
        //! is the same as the copy shader in CTM
        VertexShaderHandle _passthroughVertexShader;
        PixelShaderHandle _passthroughPixelShader;

        //! Device access
        CTM::Device *_device;
        
        //! Passes - each pass submits some set of commands to the GPU
        //! and executes a kernel on the GPU. More kernels can be packed
        //! within the same pass as well with CTM but we don't use that
        //! feature right now
        CTM::Pass *_pass;
        
        //! Actual GPU program that is to be executed for the kernel
        CTM::Buffer *_program;
        
        //! Track the number of textures used by Brook - used for 
        //! freeing all GPU memory when number is zero
        int _numTextures;

        //! Constants used by various compiled kernels
        CTM::Buffer *_fconst;
        CTM::Buffer *_iconst;
        CTM::Buffer *_bconst;
        CTM::Buffer *_interp;

        //! Params used for interpolators 
        float _params[ 32 * 3 ];
        
        bool _shouldBiasInterpolants;

        //! Hack to workaround the limitation of not being able to delete memory
        //! If any CTM program has been compiled, we cannot do a pop again. 
        bool _canPopMemory;

        bool _pushedInBegin;

        //! Utility routine for copying a given memory buffer in host memory to another
        //! memory buffer in host memory
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

#endif
