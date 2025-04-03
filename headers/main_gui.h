#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>


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

//EXTERN NICHT VERGESSEN aq