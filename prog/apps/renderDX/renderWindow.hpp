// renderWindow.hpp
#pragma once

#include "core.hpp"

class RenderWindow : public fibble::Window
{
public:
  RenderWindow();
  ~RenderWindow();

  void handleIdle();

protected:
  virtual void handleMouseDown( int x, int y );
  virtual void handleMouseUp( int x, int y );
  virtual void handleMouseMove( int x, int y );

private:
  fibble::Context* context;

  bool mouseDown;
  float mouseX;
  float mouseY;

  brook::stream fluidStream0;
  brook::stream fluidStream1;

  brook::stream normalStream;
  IDirect3DTexture9* normalTexture;

  fibble::VertexShader* vertexShader;
  fibble::PixelShader* pixelShader;
  IDirect3DVertexBuffer9* vertexBuffer;
  IDirect3DVertexDeclaration9* vertexDecl;

  IDirect3DSurface9* defaultRenderTarget;

  IDirect3DDevice9* device;
};