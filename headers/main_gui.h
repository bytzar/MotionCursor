#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
#include <Windows.h>
#define SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS" = 1

int mainRender(int, char**);
void UpdateCon(int pActiveConId);
extern std::atomic<bool> runningCal;
void Calibration();
extern std::vector<const char*> controllers;
//extern std::atomic<std::vector<const char*>*> controllersptr;
void UpdateConList();
extern std::vector<int> conIds;
extern bool first;
extern float sensitivity;
class hotkey
{
public:
	SDL_GamepadButton button = static_cast<SDL_GamepadButton>(-1);
	SDL_GamepadAxis axis = static_cast<SDL_GamepadAxis>(-1);
	bool trigger = false;
	bool listening = false;
	const char* activeLable = "<none>";

	bool isActive(SDL_Gamepad* pController)
	{
		return (SDL_GetGamepadButton(pController, button) && !trigger) || (SDL_GetGamepadAxis(pController, axis) && trigger && SDL_GetGamepadAxis(pController, axis) > 16000);
	}
};
void RemapHotkey(hotkey*);

extern hotkey activator;
extern hotkey click;
extern hotkey lock;
extern hotkey reset;

extern float stickX;
extern float stickY;

extern bool globalListening;

extern SDL_Gamepad* activeCon;

extern float avgDriftX;
extern float avgDriftY;

void UpdateLoop();
extern bool gyroExist;

extern SDL_Event event; //RUNMOVE

extern bool calibrated;
extern std::string calibratedConName;

extern bool NoCentering;
extern bool NoReqAcGyrocursor;
extern bool NoReqAcLeftClick;
extern bool NoGyroCursor;
extern bool NoLeftClick; //neue konvention weil cool irgendwie. der rat der high level bools
extern bool invX;
extern bool invY;
extern bool noLock;
extern bool noReset;

extern std::thread runUpdateLoop;

extern std::atomic<volatile bool> update;

extern int theListeningOne;

extern float fontSize;

//EXTERN NICHT VERGESSEN aq