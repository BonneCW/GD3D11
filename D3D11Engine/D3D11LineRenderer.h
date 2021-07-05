#pragma once
#include "baselinerenderer.h"

class D3D11VertexBuffer;
class D3D11LineRenderer :
    public BaseLineRenderer {
public:
    D3D11LineRenderer();
    ~D3D11LineRenderer();

    /** Adds a line to the list */
    virtual XRESULT AddLine( const LineVertex& v1, const LineVertex& v2 );
    virtual XRESULT AddLineDeferred( const ScreenSpaceLine& v1, const ScreenSpaceLine& v2 ) {
        if ( DeferredLineCache.size() >= 0xFFFFFFFF ) {
            return XR_FAILED;
        }

        DeferredLineCache.push_back( v1 );
        DeferredLineCache.push_back( v2 );
        return XR_SUCCESS;
    }

    /** Flushes the cached lines */
    virtual XRESULT Flush();
    virtual XRESULT FlushDeferredLines();

    /** Clears the line cache */
    virtual XRESULT ClearCache();


private:
    /** Line cache */
    std::vector<LineVertex> LineCache;
    std::vector<ScreenSpaceLine> DeferredLineCache;

    /** Buffer to hold the lines on the GPU */
    D3D11VertexBuffer* LineBuffer;
    unsigned int LineBufferSize; // Size in elements the line buffer can hold
};

