# Hello Triangle

This sample introduces the following concepts:

  - D3D12 Device
  - DXGI Swap Chain
  - Commands
  - Pipeline

## D3D12 Device
The D3D12 Device is the the quasi-bridge between the Engine and the GPU. 
Usually an *adapter* is selected before device creation, passing `NULL` for the *Adapter* makes the system choose any installed on the system.
If a system has an integrated and a discrete GPU installed, there's a chance the integrated GPU will be selected.
Integrated GPUs draw less battery, why they are prefered for less intensive tasks.
In Video Games, performance is everything; one reason D3D12 is chosen, as opposed to D3D11 or OpenGL, in the first place is to squeeze out every single FPS available from the hardware.
This sample is simple enough that performance does not matter.

## DXGI Swap Chain
This sample is static, but that's not the norm for really any program writing anything to the screen, regardless of reason. The Swap Chain has (at least 2) images (called buffers):
  - *front buffer*, whose content is being displayed on the screen.
  - *back buffer*,  which is the canvas onto which the renderer is drawing.
  - sometimes there's a middle buffer in order to decrease a visual artifact called tiering. This is usually not a large problem, unless the app starts getting performance intensive, as such this sample does not care about tiering.

When renderer has finished drawing onto the back buffer, the back and front buffer flip positions: the back buffer becomes the front buffer and the front buffer becomes the back buffer.
This means that renderer can clear what was the front buffer (which now is the back buffer) and draw the next frame onto that buffer. 
FPS really is about how many times per second the renderer can flip between front and back buffer.

The buffers are resources, in fact they can be considered 2D Textures. 
Because the Buffers are the target onto which the renderer is drawing, they are called *Render Target*s.
All resources in DirectX (except *input buffers*, e.g Vertex & Index Buffers) must be handled via *Views*.
In D3D12, the views can not be handled directly, instead they need to be handled via descriptors, which are stored in *Descriptor Heaps*. 
This sounds convoluted and while this sample is rather simple, this can be made unfantomably complicated with plenty of opportunity to shoot oneself in the foot. 
Authors will try to not too drastically increase complexity.

## Commands
The main selling point for D3D12 is multi-threading. Microsoft has done away with the Immediate Context, instead all commands to execute on the GPU are recorded in *CommandList*s, these then get submitted onto *CommandQueues*. 
CommandLists need to be allocated onto Command Allocators. And before they can be reused, the CommandAllocator and CommandList have to be reset.
CommandList recording is considered an expensive procedure and as such it is recommended that this is done in multiple threads.
Syncronizing in D3D12 is left to the programmer because of the multi-threaded nature of D3D12 and as such, before the buffers can be flipped, the progam has to wait for the all commands to finish executing. 

## Pipeline
The Pipeline is the procedure on the GPU for drawing figures. parts of the pipeline is static, other parts are programmable.

The programable stages are called Shaders. In DirectX, the Shading Language is called `HLSL`.
This sample uses two stages: the Vertex Shader and the Pixel Shader.

A Vertex is a geometric point, but it also can have other data associated with it. 
The Vertex Shader will take a list of vertices, these are usually stored in vertex buffers, but this sample is simple enough to where the vertices are stored inline in the vertex shader.
This sample uses the *SV_VertexID*, which is the ID of the vertex, in order to retreive the Vertex data.
Vertices in this sample consists of a geometric value called `pos` and a color value called `color`. 
The Semantic *SV_Position* will tell the Pipeline where the position of the vertex will be. DirectX uses a normalized plane with origin in the middle.

The Rasterizer stage will draw lines between the vertices and translate these into the triangle.

The Pixel Shader then says what color each pixel should be. It also applies an nice interpolatated gradient.

Pipelines as evident by this quick rundown can be made drastically complex: Geometry Shader, Tessellation, Stream Output, Compute Shade, Mesh Pipeline and Ray Tracing pipeline to name a few expansions.