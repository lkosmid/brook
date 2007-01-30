#pragma once
#include "ctmruntime.hpp"

namespace brook
{
    class CTMStream
    {
    public:
        enum ComponentType
        {
            kComponentType_Float = 0,
            kComponentType_Fixed = 1,
            kComponentType_ShortFixed = 2
        };
        
        static CTMStream* create(GPUContextCTM* inContext,
                                 int inWidth,
                                 int inHeight,
                                 int inComponents,
                                 ComponentType inComponentType = kComponentType_Float,
                                 bool read_only=false,
                                 AMuint32 GPUAddress = 0);
        ~CTMStream();
        
        int getWidth() { return width; }
        int getHeight() { return height; }
        int getOrigWidth() { return userWidth; }
        int getOrigHeight() { return userHeight; }
        int getInternalComponents() { return internalComponents; }
        int getComponents() { return components; }
        int getComponentSize() { return componentSize; }
        AMUcbufFldFormat getInternalFormat() { return internalFormat; }
        AMUcbufFldTiling getInternalTiling() { return internalTiling; }
        unsigned int getAllocatedSize();

        AMuint32 GPUAddress;
    private:
        CTMStream( GPUContextCTM* inContext,
                   int inWidth, int inHeight,
                   int origWidth,
                   int origHeight,
                   int inComponents,
                   ComponentType inComponentType = kComponentType_Float ,
                   bool read_only=true,
                   AMuint32 GPUAddress = 0 );
        
        GPUContextCTM* _context;
        int width;
        int height;
        int userWidth;
        int userHeight;
        int components;
        int internalComponents;
        ComponentType componentType;
        AMUcbufFldFormat internalFormat;
        AMUcbufFldTiling internalTiling;
        int componentSize;
        bool read_only;
    };
}
