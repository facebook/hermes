# cimgui [![Build Status](https://travis-ci.org/cimgui/cimgui.svg?branch=master)](https://travis-ci.org/cimgui/cimgui)


This is a thin c-api wrapper programmatically generated for the excellent C++ immediate mode gui [Dear ImGui](https://github.com/ocornut/imgui).
All imgui.h functions are programmatically wrapped.
Generated files are: `cimgui.cpp`, `cimgui.h` for C compilation. Also for helping in bindings creation, `definitions.lua` with function definition information and `structs_and_enums.lua`.
This library is intended as a intermediate layer to be able to use Dear ImGui from other languages that can interface with C (like D - see [D-binding](https://github.com/Extrawurst/DerelictImgui))

History:

Initially cimgui was developed by Stephan Dilly as hand-written code but lately turned into an auto-generated version by sonoro1234 in order to keep up with imgui more easily (letting the user select the desired branch and commit)

Notes:
* currently this wrapper is based on version [1.79 of Dear ImGui with internal api]
* only functions, structs and enums from imgui.h (an optionally imgui_internal.h) are wrapped.
* if you are interested in imgui backends you should look [LuaJIT-ImGui](https://github.com/sonoro1234/LuaJIT-ImGui) project.
* All naming is algorithmic except for those names that were coded in cimgui_overloads table (https://github.com/cimgui/cimgui/blob/master/generator/generator.lua#L60). In the official version this table is empty.
* Current overloaded function names can be found in (https://github.com/cimgui/cimgui/blob/master/generator/output/overloads.txt)

# compilation

* clone 
  * `git clone --recursive https://github.com/cimgui/cimgui.git`
  * `git submodule update`
* compile 
  * using makefile on linux/macOS/mingw (Or use CMake to generate project)
  * cmake options are IMGUI_STATIC (compiling as static library), IMGUI_FREETYPE (for using Freetype2) and FREETYPE_PATH (Freetype2 cmake install location)
  * or as in https://github.com/sonoro1234/LuaJIT-ImGui/tree/master/build
  
  For compiling with backends there is now an example with SDL2 and opengl3 in folder backend_test. It will generate a cimgui_sdl module and a test_sdl executable. You only need to provide SDL_PATH telling cmake where to look for SDL2 cmake installation.

# using generator


* this is only needed (before compilation) if you want an imgui version different from the one provided, otherwise generation is already done.
* you will need LuaJIT (https://github.com/LuaJIT/LuaJIT.git better 2.1 branch) or precompiled for linux/macOS/windows in https://luapower.com/luajit/download
* you need to use also a C++ compiler for doing preprocessing: gcc (In windows MinGW-W64-builds for example), clang or cl (MSVC). (this repo was done with gcc)
* update `imgui` folder to the version you desire.
* edit `generator/generator.bat` on windows, or `generator/generator.sh` on linux, to choose between gcc, clang, or cl and to choose desired backends and whether imgui_internal is generated or not and Freetype2 is used or not. 
* the defaults of generator are gcc as compiler, imgui_internal included and sdl, glfw, opengl2 and opengl3 as backends.
* edit config_generator.lua for adding includes needed by your chosen backends (vulkan needs that).
* Run generator.bat or generator.sh with gcc, clang or cl and LuaJIT on your PATH.
* as a result some files are generated: `cimgui.cpp`, `cimgui.h` and `cimgui_impl.h` for compiling and some lua/json files with information about the binding: `definitions.json` with function info, `structs_and_enums.json` with struct and enum info, `impl_definitions.json` with functions from the backends info. 
* You can pass compiler flags to generator.sh or generator.bat at the end of the call to further specify the compiler behavior. (e.g. -DIMGUI_USER_CONFIG or -DIMGUI_USE_WCHAR32)

# generate binding
* C interface is exposed by cimgui.h when you define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
* with your prefered language you can use the lua or json files generated as in:
  * https://github.com/sonoro1234/LuaJIT-ImGui/blob/master/lua/build.bat (with lua code generation in https://github.com/sonoro1234/LuaJIT-ImGui/blob/master/lua/class_gen.lua)
  * https://github.com/mellinoe/ImGui.NET/tree/autogen/src/CodeGenerator
### definitions description
* It is a collection in which key is the cimgui name that would result without overloadings and the value is an array of overloadings (may be only one overloading)
* Each overloading is a collection. Some relevant keys and values are:
  * stname : the name of the struct the function belongs to (will be "" if it is top level in ImGui namespace)
  * ov_cimguiname : the overloaded cimgui name (if absent it would be taken from cimguiname)
  * cimguiname : the name without overloading (this should be used if there is not ov_cimguiname)
  * ret : the return type
  * retref : is set if original return type is a reference. (will be a pointer in cimgui)
  * argsT : an array of collections (each one with type: argument type and name: the argument name, when the argument is a function pointer also ret: return type and signature: the function signature)
  * args : a string of argsT concatenated and separated by commas
  * call_args : a string with the argument names separated by commas for calling imgui function
  * defaults : a collection in which key is argument name and value is the default value.
  * manual : will be true if this function is hand-written (not generated)
  * skipped : will be true if this function is not generated (and not hand-written)
  * isvararg : is set if some argument is a vararg
  * constructor : is set if the function is a constructor for a class.
  * destructor : is set if the function is a destructor for a class but not just a default destructor.
  * realdestructor : is set if the function is a destructor for a class
  * templated : is set if the function belongs to a templated class (ImVector)
  * templatedgen: is set if the function belongs to a struct generated from template (ImVector_ImWchar)
  * nonUDT : if present the original function was returning a user defined type so that signature has been changed to accept a pointer to the UDT as first argument.
  * location : name of the header file and linenumber this function comes from. (imgui:000, internal:123, imgui_impl_xxx:123)
  * is_static_function : is setted when it is an struct static function.
### structs_and_enums description
* Is is a collection with three items:
  * under key enums we get the enums collection in which each key is the enum tagname and the value is an array of the ordered values represented as a collection with keys
    * name : the name of this enum value
    * value : the C string
    * calc_value : the numeric value corresponding to value
  * under key structs we get the structs collection in which the key is the struct name and the value is an array of the struct members. Each one given as a collection with keys
    * type : the type of the struct member
    * template_type : if type has a template argument (as ImVector) here will be
    * name : the name of the struct member
    * size : the number of array elements (when it is an array)
    * bitfield : the bitfield width (in case it is a bitfield)
  * under key locations we get the locations collection in which each key is the enum tagname or the struct name and the value is the name of the header file and line number this comes from.
# usage

* use whatever method is in ImGui c++ namespace in the original [imgui.h](https://github.com/ocornut/imgui/blob/master/imgui.h) by prepending `ig`
* methods have the same parameter list and return values (where possible)
* functions that belong to a struct have an extra first argument with a pointer to the struct.
* where a function returns UDT (user defined type) by value some compilers complain so the function is generated accepting a pointer to the UDT type as the first argument (or second if belongs to a struct).

# usage with backends

* look at backend_test folder for a cmake module building with SDL and opengl3.

# example bindings based on cimgui

* [LuaJIT-ImGui](https://github.com/sonoro1234/LuaJIT-ImGui)
* [ImGui.NET](https://github.com/mellinoe/ImGui.NET)
* [nimgl/imgui](https://github.com/nimgl/imgui)
* [kotlin-imgui](https://github.com/Dominaezzz/kotlin-imgui)
* [CImGui.jl](https://github.com/Gnimuc/CImGui.jl)
* [odin-imgui](https://github.com/ThisDrunkDane/odin-imgui)
* [DerelictImgui](https://github.com/Extrawurst/DerelictImgui)
* [BindBC-CimGui](https://github.com/MrcSnm/bindbc-cimgui)
* [imgui-rs](https://github.com/Gekkio/imgui-rs)
* [imgui-pas](https://github.com/dpethes/imgui-pas)
* [crystal-imgui](https://github.com/oprypin/crystal-imgui)

# C examples based on cimgui

* [sdl2_opengl3](https://github.com/cimgui/cimgui/tree/docking_inter/backend_test)
* [sdl2-cimgui-demo](https://github.com/haxpor/sdl2-cimgui-demo)
* [cimgui_c_sdl2_example](https://github.com/canoi12/cimgui_c_sdl2_example/)
* [cimgui-c-example](https://github.com/peko/cimgui-c-example) with GLFW
