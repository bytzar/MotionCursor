#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_sensor.h>
#include <iostream>
#include <cmath>
#include <Windows.h>
#include <numeric>
#define SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS" = 1


int main()
{
    if (SDL_Init(SDL_INIT_SENSOR | SDL_INIT_GAMEPAD) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "hi\n";

    int on;
    SDL_JoystickID* gem = SDL_GetGamepads(&on);
    const char* name = SDL_GetGamepadNameForID(*gem);
    std::cout << *gem << "\n " << name;

    SDL_Gamepad* moruk = SDL_OpenGamepad(1);
    bool tf = SDL_GamepadConnected(moruk);

    std::cout << tf << " " << moruk;
    
    SDL_SensorType type = SDL_SENSOR_GYRO;
    bool cem = SDL_GamepadHasSensor(moruk, type);
    std::cout << "\n\n\n" << cem;

    bool isenab = SDL_GamepadSensorEnabled(moruk, type);
    std::cout << "\n isenab " << isenab;
    bool sen = SDL_SetGamepadSensorEnabled(moruk, type, true);
    std::cout << "\n isenab now " << sen;

    float rete = SDL_GetGamepadSensorDataRate(moruk, type);
    std::cout << "\n" << rete << "\n";

    float rate = SDL_GetGamepadSensorDataRate(moruk, type);
    std::cout << "\n" << rate << "\n";


    float data[2] = { 0.0f, 0.0f }; //REset yaw pitch 
    float DYNdata[2] = { 0.0f, 0.0f };

    if (!SDL_GamepadSensorEnabled(moruk, SDL_SENSOR_GYRO)) {
        std::cerr << "Gyroscope failed to enable!" << std::endl;
        return 1;
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Move cursor to center of screen
    float drift[2] = { 0.0f, 0.0f };
    float xDrift[10];
    float yDrift[10];

    for (int i = 0; i < 10; i++) //calibaration
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
    SetCursorPos(screenWidth / 2, screenHeight / 2);

    POINT cursorPos;

    INPUT inputs[1] = {};

    // Mouse down (press left button)
    inputs[0].type = INPUT_MOUSE;
    bool wasDown = false;
    while (true)
    {
        SDL_PumpEvents();
        if (SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_MISC1))
        {
            cursorPos.x =  screenWidth / 2;
            cursorPos.y = screenHeight / 2;
            SetCursorPos(screenWidth / 2, screenHeight / 2);
            float data[2] = { 0.0f, 0.0f };
            while (SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_MISC1))
            {
                SDL_PumpEvents();

                SDL_GetGamepadSensorData(moruk, type, DYNdata, 2);

                DYNdata[1] -= avgx;
                DYNdata[0] -= avgy;

                const float threashold = 0.01f; //0.1f

                if (DYNdata[0] > threashold || DYNdata[0] < -threashold)
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
                std::cout << "Gyro: X=" << DYNdata[0] << " Y=" << DYNdata[1] << std::endl;

                GetCursorPos(&cursorPos);

                cursorPos.y = -newposy + screenHeight / 2;
                cursorPos.x = -newposx + screenWidth / 2;

                // Move the mouse
                SetCursorPos(cursorPos.x, cursorPos.y);

                if (SDL_GetGamepadButton(moruk, SDL_GAMEPAD_BUTTON_EAST))
                {
                    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                    wasDown = true;
                }
                else if (wasDown)
                {
                    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    wasDown = false;
                }
                SendInput(1, inputs, sizeof(INPUT));
            }
        }
    }
}
