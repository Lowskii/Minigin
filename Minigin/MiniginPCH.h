#pragma once

#include <stdio.h>
#include <iostream> // cout
#include <sstream> // stringstream
#include <memory> // smart pointers
#include <vector>


//*FMOD (Sounds)
//www.fmod.org
#pragma warning(push)
#pragma warning(disable: 4505 26812)
#include "fmod.hpp" 
#include "fmod_errors.h"
#pragma warning(pop)



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Logger.h"
#include "SoundManager.h"

using namespace std;