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


extern SDL_Gamepad* activeCon;

extern float avgDriftX;
extern float avgDriftY;

void UpdateLoop();

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

extern std::thread runUpdateLoop;

extern std::atomic<volatile bool> update;

extern int theListeningOne;

extern float fontSize;

//EXTERN NICHT VERGESSEN aq