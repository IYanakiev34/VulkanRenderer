# Engine notes
In this document I will outline basic notes on the engine development process.
This is a learning experience for me and hopefully for the users of this repository.
The idea of the engine is to be implemented in vulkan. I am using various tutorials
and sources on google to troubleshoot any errors. I am writing in in C for various reasons.

1. I am very familiar with the language and also with C++. I am developing a simpler engine version
in parallel in C++. For me the engine desing process and graphics rendering is not that much of an issue,
however, time and time again I feel the complexity of C++ gets to me. Maybe it is because I am not that
proficient, maybe it's because I feel the need to use complex feature, but I wanted to make an engine
with a simpler language. For me C is more intuitive.
2. Premake as build system. I am using premake since it abstracts away the complications of build systems like
make, cmake, etc. For me the build system should be simple. I do not want it to get in the way of development. I think
the build process and pipeline should be simple to setup and use. This is why I chose premake5.

# Basic Design Idea
The engine itself should be a shared library. This design choice stems from
the fact that the engine might grow, it might have multiple plugins, systems,
might link to many libraries and I do not want to client to need to link
to all of these systems. This makes the design more modular and simple to manage.
On top of the engine we will build a few applications. The first one is the Editor
itself. Think about the editor as th unreal editor or unity editor. This is what this
application should do. It should provide interface for easy interaction with the engine.
The second applciation should be the Testbed. This is meant as an internal testing application
responsible for testing engine state and new features. Finally it is the game application
this is the actual game that the user can build and run. This application should be an exe,
but when developing within the engine we should make sure we have dll portion for hot reloading code.

![High-Level Engine Design](Diagrams/engine_arc.png)

![Engine Layers](Diagrams/engine_layers.png)

// TODO: Insert graph

## Engine Part 1 (Of god knows how many)
1. Setup build system with premake. Make sure engine is dll and create the appropriate postbuildcommands.
2. Setup different environments: DEBUG,RELEASE,DIST
3. Link to Vulkan Libraries (Note those should be downloaded by the user, I will setup some scripts in the future to do that)
4. Enable exporting and importing of symbols from engine
I created a macro which allows for us to easily export and import based on if we are building the dll or not.
5. Create a logger
The current logger has 6 operational levels. Only ERROR and FATAL cannot be switched off, the rest can be switched
with preprocessor directives.
6. Create assertion system
The assertion system allows for various different types of asserts. Most importantly it provides a way
to place a breakpoint at the point of the assert and also output useful information such as file and line number.
Assert are also enabled and disabled via preprocessor directive. The idea is that we can opt to not have them,
or lets say only have DEBUG type of asserts.
7. The testbed
The testbed links to the engine and can use it's functionality via its exported symbols.
8. Typedefs
For me writing unsigned char, unsigned long long, is a waist of time. I created a defines.h which basically
shortness the type definitions. Basically we only care about the size of the type and what the type is. Is it
integral, floating, boolean type.

Some interesting stuff I learned while doing this:
```C
	va_list arg_ptr;
	va_start(arg_ptr,msg);
```
I knew in C there we variadic arguments to macros, but I did not know how to work
with variadic arguments to functions. So yeah that was knew + the function `vsnprintf(buffer,size,msg,arg_ptr(va_list))`.
This function formats arguments into a buffer which is basically a string. The msg parameter is the string format we need
the buffer is the place where we will store them.

I will try to include some diagrams of the current logger design as well as assertion design.

## Engine Part 2
We now need to start building the platform layer of the system. This layer should handle platform
specific operations, such as windowing (if applicable to the system), allocation, threads, etc.
Since each platform has different representation of internal state we can encapsulate the state
via a platform state structure which holds a void pointer to intern state. In each of the
platform source files we will define internal state struct only to be used internally.
```C
 typedef struct platform_state{
	void *internal_state;
 } platform_state;
```
In each platform file we will provide the implementation between an #ifdef and #endif block. The idea
behiind this is to use conditional compilation, since the logic needed to run on windows will not be
needed if we run on playstation for example. The most important functions in the platform layer will
be: b8 platform_startup, and platform_shutdown. These function should instantiate the window and anything
needed by the platform. The shutdown will clean up all the resources. In the windows platform case,
after we create a window we will need to handle events that come into the window. This will be done 
using the windows native callback mechanism. I will outline some important properties and processes
to follow when creating and managin a window.

1. Handles - Windows and many othe roperating system operate using handles, we need to obtain the handle
of the window so that we can reference it and perform operations on it
2. Window size - Each window has a total size, however if we want 1024 by 768 we want this to be the client
size of the window, not the total window including the title bar and everything else. Thus we need to calculate
this by using the AdjustWindowEx function of the windows api.
3. Window styles - using the windows api we can apply style to the window. Some important ones are

```C
	WS_CAPTION,
	WS_OVERLAPPED,
	WS_SYSMENY,
	WS_THICKFRAME,
	WS_THINFRAME,
	WS_MAXIMIZE,
	WS_MINIMIZE,
```
4. Handle window messages - This is done using the LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);
function. This is a window provided function that we can redefine to implement our own callbacks. Some important thing about
callbacks. On some events we want to say to the OS that we will handle the event thus we need to return a specific code to indicate that.
Those are events like closing, or redrawing the window. For the rest we simply query that parameters and call our own functions to handle
the events. For some events that are not of our interest we can simply call DefWindowProcA pass in the parameters and windows will handle
the event for us.
5. Clock - We want to be able to query the absolute time. In order to do that we need 2 things mainly. The lcok frequency, and the clocks that have passed.
Then we can obtain the 64 bit number and perform basic math to obtain the time.
6. Writing to STD_OUT and STD_ERROR. We can color our log input levels in windows. This could be done via windows api.
We need to get the handle to the STD_OUT or ERROR. Using the GetStdHandle(STD_OUT_HANDLE); and then set
the color of the text based on some value like: FOREGROUND_BLUE, using SetConsoleTextAttribute(handle,color);

Again I will try to include some preliminary design graphs of the platform layers. I will focus on the data structures
and the control flow if any exists currently.


## Engine Part 3
The goal for this part is to abstract the entry point. The user should only supply configuration
of the application along with his game and that's it. The engine should be responsible for 
running the game. So how will this be achieved. We need to define a header file in the engine
called entry.h. There we will tell the compiler that we have b8 extern create_game function. This
function will be defined and created by the user. In the entry.h file we will have a main function
which will call the create game function, to create a game. Then it will create an application,
and finally run the application with the game. But what is a game? What is application? What interfaces
does a game provide.

1. Application - We need to configuration for now: width, height, start_x, start_y, name, platform_state
2. Game - game_state (void *), function pointers for, creation, rendering, updating, window resizing



I will again place design graphs. These will be a little higher level control flow graphs that expose the
control flow from the start to the end of the application. This will show which part is user code, and which
is delegated to the engine. I will also try to provide basic graphs of the design of the data structures and
how they flow throughout the code.