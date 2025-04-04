#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#define SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS" = 1

int mainRender(int, char**);
void UpdateCon(int pActiveConId);
extern std::atomic<bool> runningCal;
void Calibration();
extern std::vector<const char*> controllers;
//extern std::atomic<std::vector<const char*>*> controllersptr;
void UpdateConList();
extern std::vector<int> conIds;
void DEBUG();
extern bool first;
extern float sensitivity;
void RemapActivator();
void RemapClick();
extern bool listening;
extern bool listeningClick;

extern SDL_GamepadButton buttonActivator;
extern SDL_GamepadButton buttonClick;
extern SDL_GamepadAxis axisActivator;
extern SDL_GamepadAxis axisClick;
extern bool triggerAct;
extern bool triggerClick;

void ReplayMacro(int pPosX, int pPosY, bool pPreview);

extern std::vector<int> macrosX;
extern std::vector<int> macrosY;
extern SDL_Gamepad* activeCon;

class Macro
{
public:
	SDL_GamepadButton buttonMac;
	SDL_GamepadAxis axisMac;
	bool triggerMacro;
	int cursorX;
	int cursorY;
	std::string buttonLable;
	bool listening; //for gui only, internal listening will be handled by global listening variable
	bool isDown = false;

	bool isPressed()
	{
		isDown = (SDL_GetGamepadButton(activeCon, buttonMac) && !triggerMacro) || (SDL_GetGamepadAxis(activeCon, axisMac) && triggerMacro && SDL_GetGamepadAxis(activeCon, axisMac) > 16000);
		return isDown;
	}
};
extern std::vector<Macro> macros;
void RemapButton(Macro* pMacro);
void ReplayMacro2(Macro* pMacro, bool pPreview);

void UpdateLoop();
//EXTERN NICHT VERGESSEN aq