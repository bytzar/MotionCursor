#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_sensor.h>
#include <iostream>
#include <cmath>
#include <Windows.h>
#include <numeric>
#include "main_gui.h"
#define SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS" = 1

//TODO setting: Enable clicking even when motion activation is off
//update only every 1s/datarate



SDL_Gamepad* activeCon;

float avgDriftX;
float avgDriftY;

std::atomic<bool> runningCal = false;
std::atomic<bool> update = true;

std::thread runUpdateLoop;
 //atomic?
std::vector<const char*> controllers;

void Calibration()
{
	if (!runningCal && activeCon)
	{
	runningCal = true;
    float drift[2] = { 0.0f, 0.0f };
    float xDrift[10];
    float yDrift[10];
    for (int i = 0; i < 10; i++) //calibaration, takes 10 samples of idle gyro for avg and counters later TODO: needs feedack for end of calibration
    {
        SDL_PumpEvents();

        SDL_GetGamepadSensorData(activeCon, SDL_SENSOR_GYRO, drift, 2);
        xDrift[i] = drift[1];
        yDrift[i] = drift[0];

        std::cout << "\nx: " << xDrift[i] << "y: " << yDrift[i] << "\n";
        SDL_Delay(5); // auch beim letzten aber jucki TODO je nach datarate
		if (!runningCal) //muss ich aborten wenn das ganze keine 100ms dauert? ich mein je nach datarate aber trotzdem
		{
			std::cout << "abort";
			return;
		}
    }
    avgDriftX = std::accumulate(xDrift + 1, xDrift + 10, 0.0f) / 9.0f; //skips first value because it is 0 for ome reason. then divides by 9 as ve avergae 9 values
    avgDriftY = std::accumulate(yDrift + 1, yDrift + 10, 0.0f) / 9.0f;
	runningCal = false;
	}
}

void UpdateLoop()
{
    float data[2] = { 0.0f, 0.0f }; //Reset delta yaw and pitch for cursor
    float DYNdata[2] = { 0.0f, 0.0f }; //Reset read delta yaw and pitch

	int screenWidth = GetSystemMetrics(SM_CXSCREEN); //später wenn main in einzelnen aufgaben zerstückelt und rendermain wahrscheinlich zu main wird, machen, dass imgui window sized virtel des bildschirms groß ist. irgendwie wichtig wann man resolution hold weil manchmal dpi dependent manchaml independent sau komisch
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); //was wenn in benutzung bildschirm res oder dpi geändert wird............. neeeeeeeeeeeee macht keiner

	POINT cursorPos;
	INPUT inputs[1] = {};

	inputs[0].type = INPUT_MOUSE;
	bool wasDown = false;

	//was wenn controller gewechselt wird, tesen könnte einfach klappen muss mit freund weil hab kein anderen vernünftig gyro der nicht joycon weirdness hat joycon hat in sdl ja eigene joycon gyro zeug nervig
	while (update) //muss immer true sein sonst terminated mainthread einfach und das schlimm!oh aber für interne zwekc könnte gut sein, für graceful shutdown zb müsste ich den loop beenden können -> nur mit variable einfach point blank kill execution ist auch nicht feine art//immer tru? bzw ist intended behaviour nicht dass es immer true ist. man soll ja mitten drinn nicht das ding pausieren können, wenn du das willst mach programm aus ist die idee. soll nicht so ein dummes program sein wo man es erst startet und dann muss man noch knopf drücken um funktionalität zu starten so wenn er nicht wollte hätte er program niht angemacht wenn er nicht mehr weill mach halt aus wa
	{
		if (activeCon) //CHECKPOINT ok also hab das geschrieben weil ich dachte auf active con mit anderen thread zu schreiben während hier rennt macht proboeme mit deinitialisierung oder so jedenfalls villeicht activecon atomic machen baer das wahre problem war akku ist einfach tod gegangen muss gucken dass das registriert und so eigenlich hat program alles richtig gemacht keine crashes und so weiter (nicht viel getestet) nur muss dieses dropdown aktualisiert werden und active zu <none>
		{
			SDL_PumpEvents();
			if (SDL_GetGamepadButton(activeCon, SDL_GAMEPAD_BUTTON_MISC1)) //checks for activation button be pressed
			{
				cursorPos.x = screenWidth / 2;
				cursorPos.y = screenHeight / 2;
				SetCursorPos(screenWidth / 2, screenHeight / 2);
				float data[2] = { 0.0f, 0.0f };
				while (SDL_GetGamepadButton(activeCon, SDL_GAMEPAD_BUTTON_MISC1))
				{
					SDL_PumpEvents();

					SDL_GetGamepadSensorData(activeCon, SDL_SENSOR_GYRO, DYNdata, 2);

					//subtracts drift
					DYNdata[1] -= avgDriftX;
					DYNdata[0] -= avgDriftY;

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

					if (SDL_GetGamepadButton(activeCon, SDL_GAMEPAD_BUTTON_EAST) && !wasDown)
					{
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						SendInput(1, inputs, sizeof(INPUT));
						wasDown = true;
					}
					else if (wasDown && !SDL_GetGamepadButton(activeCon, SDL_GAMEPAD_BUTTON_EAST))
					{
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
						SendInput(1, inputs, sizeof(INPUT));
						wasDown = false;
					}
				}
			}
		}
	}
}

void UpdateCon(int pActiveConId)
{
	activeCon = SDL_OpenGamepad(pActiveConId + 1);
}

void UpdateConList()
{
	update = false;
	if (runUpdateLoop.joinable())
	{
		runUpdateLoop.join();
	}
	std::cout << "\ni am cout";
	if (SDL_Init(SDL_INIT_SENSOR | SDL_INIT_GAMEPAD) < 0) //Initializes controller and checks for available sensors, also checks for error
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return;
	}

	int countControllers;
	SDL_JoystickID* gem = SDL_GetGamepads(&countControllers);


	if (countControllers == 0)
	{
		std::cout << "no Sdl controllers connected.";
		return;
	}

	controllers.resize(countControllers);

	SDL_JoystickID kem;
	for (int i = 0; i < countControllers; i++)
	{
		SDL_Gamepad* game = SDL_OpenGamepad(gem[i]);
		kem = SDL_GetGamepadID(game);	
		controllers[i] = SDL_GetGamepadNameForID(kem);
	}

	

	

	if (countControllers > 0)
	{
		//const char* name = SDL_GetGamepadNameForID(*gem);
		//std::cout << *gem << "\n " << name;

		//activeCon = SDL_OpenGamepad(1); //provisorisch ne doch auto select den ersten beim ersten mal falls oh... falls vorhanden
		UpdateCon(kem - 1);

		//bool tf = SDL_GamepadConnected(activeCon);

		//std::cout << tf << " " << activeCon;

		SDL_SensorType type = SDL_SENSOR_GYRO;
		bool cem = SDL_GamepadHasSensor(activeCon, type); //add gyro joycon support
		std::cout << "\n\n\n" << cem;

		bool isenab = SDL_GamepadSensorEnabled(activeCon, type);
		std::cout << "\n isenab " << isenab;
		bool sen = SDL_SetGamepadSensorEnabled(activeCon, type, true);
		std::cout << "\n isenab now " << sen;

		float rete = SDL_GetGamepadSensorDataRate(activeCon, type);
		std::cout << "\n" << rete << "\n ";


		if (!SDL_GamepadSensorEnabled(activeCon, SDL_SENSOR_GYRO)) {
			std::cerr << "Gyroscope failed to enable!" << std::endl;
			return;
		}
		update = true;
		runUpdateLoop = std::thread(UpdateLoop);
	}
}

int main() //soon to be int init()
{
	
 
    if (SDL_Init(SDL_INIT_SENSOR | SDL_INIT_GAMEPAD) < 0) //Initializes controller and checks for available sensors, also checks for error
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "hi\n";

    int countControllers;
    SDL_JoystickID* gem = SDL_GetGamepads(&countControllers);

    if (countControllers == 0)
    {
        std::cout << "no Sdl controllers connected.";
        //return 3;
    }

    controllers.resize(countControllers);

    for (int i = 0; i < countControllers; i++)
    {
        controllers[i] = SDL_GetGamepadNameForID(i + 1);
    }

    std::thread guirenderer(mainRender, NULL, nullptr);

	if (countControllers > 0)
	{
		//const char* name = SDL_GetGamepadNameForID(*gem);
		//std::cout << *gem << "\n " << name;

		//activeCon = SDL_OpenGamepad(1); //provisorisch ne doch auto select den ersten beim ersten mal falls oh... falls vorhanden
		UpdateCon(0);

		//bool tf = SDL_GamepadConnected(activeCon);

		//std::cout << tf << " " << activeCon;
    
		SDL_SensorType type = SDL_SENSOR_GYRO;
		bool cem = SDL_GamepadHasSensor(activeCon, type); //add gyro joycon support
		std::cout << "\n\n\n" << cem;

		bool isenab = SDL_GamepadSensorEnabled(activeCon, type);
		std::cout << "\n isenab " << isenab;
		bool sen = SDL_SetGamepadSensorEnabled(activeCon, type, true);
		std::cout << "\n isenab now " << sen;

		float rete = SDL_GetGamepadSensorDataRate(activeCon, type);
		std::cout << "\n" << rete << "\n ";



		if (!SDL_GamepadSensorEnabled(activeCon, SDL_SENSOR_GYRO)) {
			std::cerr << "Gyroscope failed to enable!" << std::endl;
			return 1;
		}
	}

	if (runUpdateLoop.joinable())
	{
		runUpdateLoop.join();
	}
	//runUpdateLoop = std::thread(UpdateLoop);
	while (true)
	{

	}
    //Calibration();
	//UpdateLoop(); //ohne diese line ist prgram fert also main thread ist fertig deswegn crashen alle anderen threads einfach. kein problem wenn ich aus main einfach nur noch init methode mace
}
