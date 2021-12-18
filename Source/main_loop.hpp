#pragma once

#include <functional>
#include <initializer_list>
#include <memory>

#include <SDL.h>

namespace devilution {

// A top-level loop handler that responds to events and handles rendering.
class MainLoopHandler {
public:
	virtual ~MainLoopHandler() = default;

	virtual void Activated()
	{
	}

	virtual void Deactivated()
	{
	}

	virtual void HandleEvent([[maybe_unused]] SDL_Event &event)
	{
	}

	virtual void Render()
	{
	}

	bool IsClosed()
	{
		return isClosed;
	}
protected:
	virtual void Close()
	{
		isClosed = true;
	}

private:
	bool isClosed = false;
};

// Replaces the current main loop handler with the given one.
void AddMainLoopHandler(std::unique_ptr<MainLoopHandler> handler);

// The function to call once the main loop has finished or on `MainLoopQuit`.
void SetMainLoopQuitFn(std::function<void(int status)> fn);

// Ends the main loop with the given status.
void MainLoopQuit(int status);

// Runs the main loop. This should only be called once by the executable
// and nothing should be called after it.
void RunMainLoop();

} // namespace devilution
