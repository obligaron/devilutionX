#include "main_loop.hpp"

#include <utility>
#include <vector>

#include "utils/log.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// #define DEBUG_MAIN_LOOP

namespace devilution {

namespace {
std::vector<std::unique_ptr<MainLoopHandler>> Handlers;
MainLoopHandler *pLastActiveHandler = nullptr;

std::function<void(int status)> QuitFn;
int QuitStatus;

bool CheckIfHandlerIsClosed(MainLoopHandler *pHandler)
{
	if (!pHandler->IsClosed())
		return false;
	pHandler->Deactivated();
	Handlers.erase(std::find_if(Handlers.begin(), Handlers.end(), [&](const std::unique_ptr<MainLoopHandler> &handler) {
		return handler.get() == pHandler;
	}));
	return true;
}

// Runs an iteration of the main loop.
// Return true if the loop should continue.
bool RunMainLoopIteration()
{
	if (Handlers.empty()) {
		QuitFn(QuitStatus);
		return false;
	}
	size_t handlerIndex = Handlers.size() - 1;
	auto *pHandler = Handlers.at(handlerIndex).get();

	if (pHandler != pLastActiveHandler) {
		pHandler->Activated();
		pLastActiveHandler = pHandler;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			Handlers.clear();
			pHandler = nullptr;
		}
		if (pHandler == nullptr) {
			QuitFn(QuitStatus);
			return false;
		}
		pHandler->HandleEvent(event);
		if (CheckIfHandlerIsClosed(pHandler))
			return true;
	}
	pHandler->Render();
	if (CheckIfHandlerIsClosed(pHandler))
		return true;
	if (pHandler == nullptr) {
		QuitFn(QuitStatus);
		return false;
	}
	return true;
}

#ifdef __EMSCRIPTEN__
extern "C" EM_BOOL RunMainLoopIterationEmscripten(void *userData)
{
	return RunMainLoopIteration() ? EM_TRUE : EM_FALSE;
}
#endif

} // namespace

void SetMainLoopQuitFn(std::function<void(int status)> fn)
{
	QuitFn = std::move(fn);
}

void MainLoopQuit(int status)
{
	QuitStatus = status;
	//if (Handler == nullptr) {
		QuitFn(QuitStatus);
	//} else {
		//Handler = nullptr;
	//}
}

void AddMainLoopHandler(std::unique_ptr<MainLoopHandler> handler)
{
#ifdef DEBUG_MAIN_LOOP
	if (Handler == nullptr) {
		Log("[MAIN_LOOP] Handler set to {}", handler != nullptr ? typeid(*handler).name() : "nullptr");
	} else {
		Log("[MAIN_LOOP] Handler changed from {} to {}", typeid(*Handler).name(), handler != nullptr ? typeid(*handler).name() : "nullptr");
	}
#endif

	if (!Handlers.empty()) {
		size_t handlerIndex = Handlers.size() - 1;
		auto *Handler = Handlers.at(handlerIndex).get();
		if (pLastActiveHandler != nullptr) {
			pLastActiveHandler->Deactivated();
			pLastActiveHandler = nullptr;
		}
	}

	Handlers.push_back(std::move(handler));
}

void RunMainLoop()
{
#ifdef __EMSCRIPTEN__
	void emscripten_set_main_loop(RunMainLoopIterationEmscripten, /*fps=*/0, /*simulate_infinite_loop=*/0);
#else
	while (RunMainLoopIteration()) { }
#endif
}

} // namespace devilution
