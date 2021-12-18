/**
 * @file movie.cpp
 *
 * Implementation of video playback.
 */

#include "diablo.h"
#include "effects.h"
#include "engine/demomode.h"
#include "hwcursor.hpp"
#include "main_loop.hpp"
#include "sound.h"
#include "storm/storm_svid.h"
#include "utils/display.h"

namespace devilution {

/** Should the movie continue playing. */
bool movie_playing;
/** Should the movie play in a loop. */
bool loop_movie;

namespace {
class MovieMainLoopHandler : public MainLoopHandler {
public:
	MovieMainLoopHandler(const char *moviePath, bool loop, bool userCanClose)
	    : moviePath(moviePath), loop(loop), userCanClose_(userCanClose)
	{

	}

	~MovieMainLoopHandler() override
	{
	}

	void Activated() override
	{
		sound_disable_music(true);
		stream_stop();
		effects_play_sound("Sfx\\Misc\\blank.wav");

		if (IsHardwareCursorEnabled()) {
			SetHardwareCursorVisible(false);
		}

		if (SVidPlayBegin(moviePath, loop ? 0x100C0808 : 0x10280808)) {
			movie_playing = true;
		} else {
			Close();
		}
	}

	void Deactivated() override
	{
		if (movie_playing) {
			movie_playing = false;
			SVidPlayEnd();
		}
		sound_disable_music(false);
		SDL_GetMouseState(&MousePosition.x, &MousePosition.y);
		OutputToLogical(&MousePosition.x, &MousePosition.y);
	}

	void HandleEvent(SDL_Event &event) override
	{
		tagMSG msg;
		if (!movie_playing || !FetchMessage(&event, &msg)) {
			Close();
			return;
		}
		switch (msg.message) {
		case DVL_WM_KEYDOWN:
		case DVL_WM_LBUTTONDOWN:
		case DVL_WM_RBUTTONDOWN:
			if (userCanClose_ || (msg.message == DVL_WM_KEYDOWN && msg.wParam == DVL_VK_ESCAPE)) {
				Close();
				return;
			}
			break;
		}
	}

	void Render() override
	{
		if (!SVidPlayContinue()) {
			Close();
		}
	}

private:
	const char *moviePath;
	bool loop;
	bool userCanClose_;
};

class AfterInGameMovie : public MainLoopHandler {
public:
	AfterInGameMovie()
	{
		ClearScreenBuffer();
		force_redraw = 255;
		scrollrt_draw_game_screen();
		PaletteFadeIn(8);
		force_redraw = 255;
	}
};

} // namespace

void play_movie(const char *pszMovie, bool userCanClose)
{
	if (demo::IsRunning()) {
		return;
	}

	AddMainLoopHandler(std::make_unique<MovieMainLoopHandler>(pszMovie, loop_movie, userCanClose));
}

void PlayInGameMovie(const char *pszMovie)
{
	if (demo::IsRunning()) {
		return;
	}

	PaletteFadeOut(8);
	AddMainLoopHandler(std::make_unique<AfterInGameMovie>());
	play_movie(pszMovie, false);
}

} // namespace devilution
