#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_sensor.h>
#include <iostream>
#include <cmath>
#include <Windows.h>
#include <numeric>
#include "main_gui.h"
#include <thread>
#define SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS" = 1

//TODO setting: Enable clicking even when motion activation is off
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

int main()
{
    std::thread guirenderer(mainRender, NULL, nullptr);
    if (SDL_Init(SDL_INIT_SENSOR | SDL_INIT_GAMEPAD) < 0) //Initializes controller and checks for available sensors, also checks for error
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "hi\n";

    int on;
    SDL_JoystickID* gem = SDL_GetGamepads(&on);

    if (on == 0)
    {
        std::cout << "no Sdl controllers connected.";
        return 3;
    }

    const char* name = SDL_GetGamepadNameForID(*gem);
    std::cout << *gem << "\n " << name;

    SDL_Gamepad* moruk = SDL_OpenGamepad(2);
    bool tf = SDL_GamepadConnected(moruk);

    std::cout << tf << " " << moruk;
    
    SDL_SensorType type = SDL_SENSOR_GYRO;
    bool cem = SDL_GamepadHasSensor(moruk, type); //add gyro joycon support
    std::cout << "\n\n\n" << cem;

    bool isenab = SDL_GamepadSensorEnabled(moruk, type);
    std::cout << "\n isenab " << isenab;
    bool sen = SDL_SetGamepadSensorEnabled(moruk, type, true);
    std::cout << "\n isenab now " << sen;

    float rete = SDL_GetGamepadSensorDataRate(moruk, type);
    std::cout << "\n" << rete << "\n ";

    float data[2] = { 0.0f, 0.0f }; //Reset delta yaw and pitch for cursor
    float DYNdata[2] = { 0.0f, 0.0f }; //Reset read delta yaw and pitch

    if (!SDL_GamepadSensorEnabled(moruk, SDL_SENSOR_GYRO)) {
        std::cerr << "Gyroscope failed to enable!" << std::endl;
        return 1;
    }


    float drift[2] = { 0.0f, 0.0f };
    float xDrift[10];
    float yDrift[10];

    for (int i = 0; i < 10; i++) //calibaration, takes 10 samples of idle gyro for avg and counters later TODO: needs feedack for end of calibration
    {
        SDL_PumpEvents();

        SDL_GetGamepadSensorData(moruk, type, drift, 2);
        xDrift[i] = drift[1];
        yDrift[i] = drift[0];

        std::cout << "\nx: " << xDrift[i] << "y: " << yDrift[i] << "\n";
        SDL_Delay(500); // auch beim letzten aber jucki
    }
    float avgx = std::accumulate(xDrift + 1, xDrift + 10, 0.0f) / 9.0f; //skips first value because it is 0 for ome reason. then divides by 9 as ve avergae 9 values
    float avgy = std::accumulate(yDrift + 1, yDrift + 10, 0.0f) / 9.0f;

    POINT cursorPos;
    INPUT inputs[1] = {};

    // input type is mouse
    inputs[0].type = INPUT_MOUSE;
    bool wasDown = false;
    while (true)
    {
        SDL_PumpEvents();
        if (SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_MISC1)) //checks for activation button be pressed
        {
            cursorPos.x =  screenWidth / 2;
            cursorPos.y = screenHeight / 2;
            SetCursorPos(screenWidth / 2, screenHeight / 2);
            float data[2] = { 0.0f, 0.0f };
            while (SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_MISC1))
            {
                SDL_PumpEvents();

                SDL_GetGamepadSensorData(moruk, type, DYNdata, 2);

                //subtracts drift
                DYNdata[1] -= avgx; 
                DYNdata[0] -= avgy;

                const float threashold = 0.02f; //0.01f min maybe, maybe controller specific ()customizeable. automatic threshold. während calibration test values also gucken ob im bereich 000.0003 oder 0.0007 und je nachdem plus x prozent davon als threshold

                if (DYNdata[0] > threashold || DYNdata[0] < -threashold) //discard if movement is so minor it is probably drift
                {
                    data[0] += DYNdata[0];
                }
                if (DYNdata[1] > threashold || DYNdata[1] < -threashold)
                {
                    data[1] += DYNdata[1];
                }

                int nill = 1;
                if (data[1] < 0) { nill = -1; }

                int nuhull = 1;
                if (data[0] < 0) { nuhull = -1; }

                float MaxGyroForComforty = 125.0f;
                float MaxGyroForComfortx = 125.0f;

                if (data[1] >= MaxGyroForComforty) { data[1] = MaxGyroForComforty; }
                if (data[1] < -MaxGyroForComforty) { data[1] = -MaxGyroForComforty; }

                if (data[0] >= MaxGyroForComfortx) { data[0] = MaxGyroForComfortx; }
                if (data[0] < -MaxGyroForComfortx) { data[0] = -MaxGyroForComfortx; }

                float newposx = 0.0f;
                float newposy = 0.0f;

                if (!data[1] == 0)
                {
                    newposx = (((abs(data[1]) / MaxGyroForComforty) * (float)(screenWidth / 2)) * nill); 
                }
                if (!data[0] == 0)
                {
                    newposy = (((abs(data[0]) / MaxGyroForComfortx) * (float)(screenHeight / 2)) * nuhull); 
                }

                std::cout << "Gyro: X=" << data[0] << " Y=" << data[1] << std::endl;
                std::cout << "Gyro Delta: X=" << DYNdata[0] << " Delta Y=" << DYNdata[1] << std::endl;

                GetCursorPos(&cursorPos);

                cursorPos.y = -newposy + screenHeight / 2;
                cursorPos.x = -newposx + screenWidth / 2;

                // Move the mouse
                SetCursorPos(cursorPos.x, cursorPos.y);

                if (SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_EAST) && !wasDown)
                {
                    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                    SendInput(1, inputs, sizeof(INPUT));
                    wasDown = true;
                }
                else if (wasDown && !SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_EAST))
                {
                    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    SendInput(1, inputs, sizeof(INPUT));
                    wasDown = false;
                }
            }
        }
    }
}
