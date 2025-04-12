// Dear ImGui: standalone example application for SDL3 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important to understand: SDL_Renderer is an _optional_ component of SDL3.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.

#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include "main_gui.h"


#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif
#include <imgui_internal.h>

int activeConId = 0;
std::thread runCal;
std::thread runUpdateCon;
std::thread runRemapAct;
std::thread runRemapClick;

bool virginCall = true;
bool first = true;
ImGuiID dockspace_id;
float sensitivity = 1;
int theListeningOne = -1;

bool NoCentering = false;

bool NoReqAcGyrocursor = false;
bool NoReqAcLeftClick = false;
bool NoGyroCursor = false;
bool NoLeftClick = false; 
bool invX = false;
bool invY = false;
float fontSize = 2.0f;
SDL_Event event;
// Main code
int mainRender(int, char**)
{
    UpdateConList();
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Create window with SDL_Renderer graphics context
    Uint32 window_flags = SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow("MotionCursor GUI", 1440, 810, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiWindowFlags_NoTitleBar;
    io.ConfigFlags |= ImGuiWindowFlags_NoCollapse;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    const int targetFPS = 15;
    const std::chrono::milliseconds frameDuration(1000 / targetFPS);
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        auto frameStart = std::chrono::high_resolution_clock::now();
        while (SDL_PollEvent(&event)) //Diesen code villeciht in den 200ms block reintun!!!!WAIT HALLO GUCK HIER DASD KÖNNTE ES`SEINEINFACH DAS GLIBAL aber nur efso 15 mal die selkunde9ßitjgjfkr49jgjq+000004ßwjr ölk
        {//warte das ist magic code. der lloopt einfach hier alleine ohne fick auf 15ms zu geben. hier mach ich alles joker lets go wieder glück gehabt
            //SDL_PumpEvents(); //sdl pollevent macht schon brauch ich nicht noch explicit. damals hatte ich ncith pollevent da musste ich rufen
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
            {
                done = true;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            {
                done = true;
                
            }

        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }




        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();


        if (virginCall)
        {
            dockspace_id = ImGui::GetID("MyDockSpace");
            if (ImGui::DockBuilderGetNode(dockspace_id) == NULL)
            {
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_id, 0); // Add empty node
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(0.45f, 0.55f));

                ImGuiID dock_main_id = dockspace_id;
                ImGui::DockBuilderDockWindow("Settings", dock_main_id);

                ImGui::DockBuilderFinish(dockspace_id);
            }
            //ImGui::DockSpace(dockspace_id);
            virginCall = false;
        }

        ImGui::DockSpaceOverViewport(dockspace_id);
        
        
        ImGui::GetIO().FontGlobalScale = fontSize;  // Adjust scale factor as needed TODO MAKE ADJUSTABLE

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        //ImGui::DockBuilderGetNode(your_id) == 0;
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoTitleBar;
            window_flags |= ImGuiWindowFlags_NoCollapse;
            window_flags |= ImGuiConfigFlags_ViewportsEnable;
            window_flags |= ImGuiWindowFlags_NoResize;
            window_flags |= ImGuiWindowFlags_NoMove;
            


            ImGui::Begin("Settings", NULL, window_flags);                          // Create a window called "Settings" and append into it.
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state


            ImGui::Text("font size");
            ImGui::SameLine();
            if (ImGui::Button("reset"))
                fontSize = 2.0f;
            ImGui::SameLine();
            ImGui::SliderFloat(" ", &fontSize, 1.0f, 5.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            
           
            
            if (ImGui::Button("Calibrate"))
            {
                if (!runningCal)
                {
                    if (runCal.joinable()) {
                        runCal.join();  // Make sure old thread is done
                    }
                    runCal = std::thread(Calibration);
                }
            }
            ImGui::SameLine();
            if (calibrated && !(calibratedConName.compare("Z") == 0))
            {
                std::string p = "calibrated for ";
                std::string c = (calibratedConName);
                p += c;
                ImGui::Text(p.c_str()); //TODO
            }
            else
            {
                ImGui::Text("not calibrated");
            }

            

            
            if (ImGui::Button("Refresh"))
            {
                if (first)
                {
                    first = false;
                    UpdateConList();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Controller"))
                ImGui::OpenPopup("Controller_Select");
            ImGui::SameLine();
            if (controllers.empty())
            {
                ImGui::TextUnformatted("<None>");
                activeConId = 0; //gehört hier nicht hin nur bei con update soll das einmal geändert werden nicht jeden frae wo es keinen con gibt
            }
            else
            {
                ImGui::TextUnformatted(controllers[activeConId]);
            }
            if (ImGui::BeginPopup("Controller_Select"))
            {
                ImGui::SeparatorText("Controllers");
                for (int i = 0; i < controllers.size(); i++)
                {
                    if ((controllers[i] && controllers[i][0] != '\0'))
                    {
                        std::string str = std::to_string(i);
                        str += " ";
                        str += controllers[i];
                        char const* pchar = str.c_str();
                        if (ImGui::Selectable(pchar))
                        {
                            if (activeConId != i) //ganz kurz
                            {
                                if (runningCal)
                                {
                                    runningCal = false; //muss noch auf thread warten machen ! und update =false damit updateloop nicht rumspackt 
                                }
                                if (runUpdateCon.joinable()) {
                                    runUpdateCon.join();  // Make sure old thread is done
                                }
                                runUpdateCon = std::thread(UpdateCon, conIds[i]);
                                calibrated = false;
                            }
                            activeConId = i;
                        }
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::Text("sensitivity multipliyer");
            ImGui::SameLine();
            if (ImGui::Button("default"))
                sensitivity = 1.0f;
            ImGui::SameLine();
            ImGui::SliderFloat("  ", &sensitivity, 0.001f, 5.0f);
           
            if (ImGui::Button("remap activation button") && !listening && !listeningClick)
            {
                if (runRemapAct.joinable()) {
                    runRemapAct.join();  // Make sure old thread is done
                }
                runRemapAct = std::thread(RemapActivator);
            }
            ImGui::SameLine();
            if (listening)
            {
                ImGui::TextUnformatted("<listening>");
            }
            else
            {
                if (triggerAct)
                {
                    ImGui::TextUnformatted((axisActivator == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "lefttrigger" : "righttrigger");
                }
                else
                {
                    const char* buttonName = SDL_GetGamepadStringForButton(buttonActivator);
                    ImGui::TextUnformatted(buttonName ? buttonName : "<none>");
                }
            }


            if (ImGui::Button("remap click button") && !listening && !listeningClick)
            {
                if (runRemapClick.joinable()) {
                    runRemapClick.join();  // Make sure old thread is done
                }
                runRemapClick = std::thread(RemapClick);
            }
            ImGui::SameLine();
            if (listeningClick)
            {
                ImGui::TextUnformatted("<listening>");
            }
            else
            {
                if (triggerClick)
                {
                    ImGui::TextUnformatted((axisClick == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "lefttrigger" : "righttrigger");
                }
                else
                {
                    const char* buttonName = SDL_GetGamepadStringForButton(buttonClick);
                    ImGui::TextUnformatted(buttonName ? buttonName : "Unknown");
                }
            }

            //uncoooment if you want this setting but cant reccommend it nur probleme weil dann mouse blockiert und wenn gyro nicht richtig klaoppt doof ImGui::Checkbox("Do not require activation for gyro cursor", &NoReqAcGyrocursor); //default off muss immernoch calibration static text fixxen aber ez einfach if svgx nicht 0 oder überhaupt initialisiert dann soll da stehen cablibrated
            ImGui::Checkbox("do not require activation for left clicking", &NoReqAcLeftClick); //default off 
            ImGui::Checkbox("turn hold activation to toggle activation", &NoReqAcGyrocursor); //toogle ist besser falls man abgefuckten controller oder abgefuckte calibration hat selbst mit mouse kann man villeicht nicht gegen ankämpfen
            ImGui::Checkbox("disable centering cursor on activation", &NoCentering); //save to file TODO
            ImGui::Checkbox("disable left clicking", &NoLeftClick); //default off
            ImGui::Checkbox("disable gyro cursor", &NoGyroCursor); //default off
            
            //resetbutton
                //lock
            ImGui::Checkbox("invert x axis gyro", &invX); //dont save as it shouldnt be needed most times
            ImGui::Checkbox("invert y axis gyro", &invY); //default off
            ImGui::TextUnformatted("\nfor calibration, place your controller on a flat surface and hit 'calibrate'");
            ImGui::TextUnformatted("calibration takes less than a second and is controller specific, only latest calibration is saved");
            ImGui::TextUnformatted("calibrating only affect this program, not the controller");
            //ImGui::TextUnformatted("in case of gyro weirdness : reconnect controllers, restart programs gucken ob das noch passiert");

            
            ImGui::End();
        }
        ////////////////////////////////////
        
        /////////////////////////////////////////////
        // Rendering
        ImGui::Render();
        //SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);


        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = frameEnd - frameStart;

        if (elapsed < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }


#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]

    std::ofstream file("MotionCursor.ini");

    file << fontSize << "\n";
    file << avgDriftX << "\n";
    file << avgDriftY << "\n";
    file << sensitivity << "\n";
    file << buttonActivator << "\n";
    file << buttonClick << "\n";
    file << axisActivator << "\n";
    file << axisClick << "\n";
    file << triggerAct << "\n";
    file << triggerClick << "\n";
    file << NoReqAcLeftClick << "\n";
    file << NoLeftClick << "\n";
    file << NoGyroCursor << "\n";
    file << NoReqAcGyrocursor << "\n";
    file << invX << "\n";
    file << invY << "\n";
    file << NoCentering << "\n";
    file << calibratedConName;

    file.close();

        

    if (runCal.joinable())
    {
        runCal.join();
    }
    if (runUpdateCon.joinable())
    {
        runUpdateCon.join();
    }
    if (runRemapAct.joinable())
    {
        runRemapAct.join();
    }
    if (runRemapClick.joinable())
    {
        runRemapClick.join();
    }
    if (runUpdateLoop.joinable())
    {
        update = false;
        runUpdateLoop.join();
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}
