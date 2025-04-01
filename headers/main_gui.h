#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>

int mainRender(int, char**);
void UpdateCon(int pActiveConId);
extern std::atomic<bool> runningCal;
void Calibration();
extern std::vector<const char*>* controllersptr;