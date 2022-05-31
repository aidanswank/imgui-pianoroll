#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

// #include <iostream>
#include "vprint.h"
// #include <stdio.h>
// #include <stdint.h>
// #include <assert.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "entt/entt.hpp"
#include "ImSequencer.h"

#include "MidiFileWrapper.h"

#define WinWidth 320 * 2
#define WinHeight 240 * 2

// #define SEQUENTITY_IMPLEMENTATION
// #include "Sequentity.h"
// entt::registry registry;

int sel = -1;
ImVec2 myvec = {-1,-1};

void SequencerWindow(bool* isOpen, std::vector<ImVec2>& points)
{
    ImGui::Begin("pianoroll",isOpen,ImGuiWindowFlags_NoTitleBar);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos(); 
    // print(p.x,p.y);
    ImGuiIO &io = ImGui::GetIO();

    for(int i = 0; i < points.size(); i++)
    {
        ImVec2 size = ImVec2( 32 , 32 );
        ImGui::SetCursorPos(ImVec2(points[i].x,points[i].y));
        std::string s = std::to_string(i);

        ImGui::Button(s.c_str(), size);
        if(ImGui::IsItemClicked())
        {
            // print("c",s.c_str());
            sel = i;
            print("isitemclicked", s.c_str(), " pos ", io.MouseClickedPos[0].x, " ", io.MouseClickedPos[0].y);
            myvec = points[i];
        }

    }

    if( ImGui::IsMouseDragging(ImGuiMouseButton_Left) )
    {
        print("sel", sel, " ", ImGui::GetMouseDragDelta().x, " ", ImGui::GetMouseDragDelta().y);
        ImVec2 new_pos = ImGui::GetMouseDragDelta();
        // points[i].x = pos.x;
        if(sel!=-1)
        {
            points[sel].x = myvec.x + new_pos.x;
            points[sel].y = round((myvec.y + new_pos.y)/32)*32;
        }
    }

    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        print("ImGuiMouseButton_Left released");
        sel = -1;
    }

    ImGui::End();
}

int main(void)
{
    // printf("hello world");
    SDL_Init( SDL_INIT_EVERYTHING );
    // SDL_Init(SDL_INIT_VIDEO);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // to make macos happy
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // uint32_t WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
    uint32_t WindowFlags = SDL_WINDOW_OPENGL | SDL_RENDERER_PRESENTVSYNC | SDL_WINDOW_RESIZABLE;
    SDL_Window *Window = SDL_CreateWindow("OpenGL Test", 0, 0, WinWidth, WinHeight, WindowFlags);
    // assert(Window);
    if (Window == NULL)
    {
        printf("failed to create window!");
    }

    SDL_GLContext Context = SDL_GL_CreateContext(Window);
    SDL_GL_MakeCurrent(Window, Context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    // ImGui::StyleColorsLight();
    ImGuiStyle* style = &ImGui::GetStyle();
	style->FrameBorderSize = 1.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(Window, Context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    ImFont* font = io.Fonts->AddFontFromFileTTF("./res/fonts/unifont-14.0.01.ttf", 16.0f);

    bool isRunning = true;
    bool isFullScreen = true;

    bool isOpenSequencerWindow = true;
    std::vector<ImVec2> points = { ImVec2(0,0), ImVec2(32,32), ImVec2(32,64), ImVec2(32,128) };
    
    MidiFileWrapper mid;
    int err = mid.init("./res/midis/background_guitar.mid");
    if(err==0)
    {
        std::cout << "error loading midi!!" << std::endl;
    }
    // mid.printData();
    std::vector<MidiTrack> tracks = mid.makeStructs();

    while(isRunning)
    {
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            ImGui_ImplSDL2_ProcessEvent(&Event);
            if (Event.type == SDL_KEYDOWN)
            {
                switch (Event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    isRunning = 0;
                    break;
                case 'f':
                    isFullScreen = !isFullScreen;
                    if (isFullScreen)
                    {
                        SDL_SetWindowFullscreen(Window, WindowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    else
                    {
                        SDL_SetWindowFullscreen(Window, WindowFlags);
                    }
                    break;
                default:
                    break;
                }
            }
                else if (Event.type == SDL_QUIT)
            {
                isRunning = 0;
            }
        }

        glClear( GL_COLOR_BUFFER_BIT );


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(Window);
        ImGui::NewFrame();

        {

            ImGui::PushFont(font);

            ImGui::Begin("debug");

            // ImGui::SliderFloat("rotation", &rot_amount, 0.0f, 360.0f);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();

            // ImGui::ShowDemoWindow();
            SequencerWindow(&isOpenSequencerWindow, points);

            ImGui::PopFont();
        }
        

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(Window);

    }

       // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
};