// dx9texture.hpp
#pragma once

#include "dx9base.hpp"

namespace brook
{

    class DX9Texture
    {
    public:
        static DX9Texture* create( GPUContextDX9* inContext, int inWidth, int inHeight, int inComponents );
        ~DX9Texture();

        int getWidth() { return width; }
        int getHeight() { return height; }

        void setData( const float* inData, unsigned int inStride, unsigned int inCount );
        void getData( float* outData, unsigned int inStride, unsigned int inCount );

        void markCachedDataChanged();
        void markShadowDataChanged();
        void validateCachedData();

        void getPixelAt( int x, int y, float4& outResult );

        LPDIRECT3DTEXTURE9 getTextureHandle() {
        return textureHandle;
        }

        LPDIRECT3DSURFACE9 getSurfaceHandle() {
        return surfaceHandle;
        }

    private:
        DX9Texture( GPUContextDX9* inContext, int inWidth, int inHeight, int inComponents );
        bool initialize();

        void flushCachedToShadow();
        void flushShadowToCached();
        void getShadowData( void* outData, unsigned int inStride, unsigned int inCount  );
        void setShadowData( const void* inData, unsigned int inStride, unsigned int inCount  );

        GPUContextDX9* _context;
        LPDIRECT3DDEVICE9 device;

        int width;
        int height;
        int components;
        int internalComponents;
        LPDIRECT3DTEXTURE9 textureHandle;
        LPDIRECT3DSURFACE9 surfaceHandle;

        LPDIRECT3DSURFACE9 shadowSurface;

        enum DirtyFlag {
            kShadowDataDirty = 0x01,
            kCachedDataDirty = 0x02
        };
        int dirtyFlags;
    };
}