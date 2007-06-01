#ifndef _CTM_STREAM_H_
#define _CTM_STREAM_H_

#include "ctmruntime.hpp"

namespace brook
{
    //! CTMStream class - acts as a wrapper around a Brook stream
    //! The current implementation stores a CTM::Buffer object internally
    //! for CTM specific data storage. CTMStreams are assumed to be in GPU
    //! memory arenas
    class CTMStream
    {
    public:
    
        //! Component types - float, fixed and short
        enum ComponentType
        {
            kComponentType_Float = 0,
            kComponentType_Fixed = 1,
            kComponentType_ShortFixed = 2
        };
        
        //! Factory for CTMStream - accepts a CTM GPUContext object along with 
        //! other parameters and internally allocates a CTM::Buffer
        static CTMStream* create(GPUContextCTM* inContext,
                                 int inWidth,
                                 int inHeight,
                                 int inComponents,
                                 ComponentType inComponentType = kComponentType_Float,
                                 bool read_only=false);
                                 
        //! Destructor
        ~CTMStream();
        
        //! Get the CTM::Buffer
        CTM::Buffer *getBuffer() { return _buffer; }
        
        //! Other get methods for the CTMStream
        int getWidth() { return _buffer->getWidth(); }
        int getHeight() { return _buffer->getHeight(); }
        int getOrigWidth() { return _userWidth; }
        int getOrigHeight() { return _userHeight; }
        int getInternalComponents() { return _internalComponents; }
        int getComponents() { return _components; }
        int getComponentSize() { return _componentSize; }
        int getInternalFormat() { return _internalFormat; }
        int getInternalTiling() { return _internalTiling; }
        unsigned int getAllocatedSize();

    private:

        //! Private constructor    
        CTMStream( GPUContextCTM* inContext,
                   int inWidth, 
                   int inHeight,
                   int origWidth,
                   int origHeight,
                   int inComponents,
                   ComponentType inComponentType = kComponentType_Float,
                   bool read_only=true);
        
        //! CTM device context info
        GPUContextCTM* _context;
        
        //! Other buffer parameters - most likely we can remove these
        //! since the information is now stored in CTM::Buffer object
        int _userWidth;
        int _userHeight;
        int _components;
        int _internalComponents;
        ComponentType _componentType;
        int _internalFormat;
        int _internalTiling;
        int _componentSize;
        bool _readOnly;
        
        //! Internal CTM::Buffer object 
        CTM::Buffer *_buffer;
    };
}

// utility routine to print the contents of a buffer for debugging
// purposes. Assumes single channel float data right now
void printBuffer(void *data, const int width, const int height);

#endif
