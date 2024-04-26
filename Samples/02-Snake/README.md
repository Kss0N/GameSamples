# Snake

This game introduces concepts:

	- Different Coordinate Systems
	- Instance Buffers
	- Constant Buffers
	- Textures and Sprite Sheets (Shader Resources)
	
BONUS:

	- System Time
	- Keyboard Input

## Coordinate System
Four different coordinate systems are used: Screen Space, Viewport Space, World Space and Texture Space.

Screen space is the coordinate system for the window's client area. 
It's origin (the point that has coordinate (0,0)) is in the top-left corner of the window's client area.
The positive X axis goes to the right and the positive Y axis goes down. 
The bottom-right corner's coordinate is (*screen width*, *screen height*).
The board is only located in the middle of the screen; it is not drawn over the entire window.

The Viewport space is where the pipeline will draw. 
It's origin is in the middle of the viewport area. 
Top-Right is (1,1) and bottom-left is (0,0).
In reality it has a 3rd dimension to it, but it's ignored here.
It is much more easy to do 2D operations on a 3D platform than the other way around; simply restrict yourself to the plane where Z = 0.

The World space (here called snake-space) is the coordinate system that the game is operating in. 
This can therefore be arbitrarily defined, as long as appropriate transformations are performed during drawing.
The snake-space system only operates on integers.
It's origin is the bottom left tile. 
The top-right tile has coordinates (c_Rows, c_Cols)

The final space is the Texture Space. Texture coordinates are used for sampling images. 
In DirectX, it's origin is the top-left of the image. 
The bottom right of the image has coordinates (1,1).

## Instance Buffers

The only geometric shape being drawn in this game are squares. 
The game includes more than that - checkboard colors, a snake & an apple.
It is possible to supply special data specific to one instance of a quad. 
In this case what is being supplied is the position of the apple / snake body parts on the board as well as data pertaining to what texture it's going to have. (More on that later)
This data is stored sequencially in a block of data called a `buffer`.
In this case the buffer contains structs of SnakeParts.
Each instance of a quad being drawn using the snake-pipeline


## Const Buffers

Const Buffers allow for data not defined at Shader-compile time to be supplied to the Pipeline.
A Const Buffer is a buffer, but it typically is only 256 bytes large and holds auxillary data that doesn't need to be instance-individual.
For a pipeline to use a const buffer, it must be allowed to do so via the Root Signature.
Const buffer typically has as Const Buffer View (`CBV`) that is ued to connect the resource to the pipeline.

## Texture2D and Sprite Sheet

A Texture2D is an image resource. 
This image resource in particular is a Sprite Sheet (also known as an Atlas) - which means many images are packed onto the texture, occupying just one area.
This is also one of the reasons the texcoord system was emphasized.
Texture2D resources are bound to the Pipeline via ShaderResourceViews.
