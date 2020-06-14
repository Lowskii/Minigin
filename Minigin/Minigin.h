#pragma once
#include "MiniginPCH.h"

struct SDL_Window;

namespace minigin
{
	class Minigin
	{
	public:
		void Initialize();
		void Cleanup();
		void Run();
		static void QuitProgram();
	private:
		SDL_Window* m_Window{};
		static bool m_Continue;
	};
}
