
#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"
#include "control.h"
#include "main_loop.hpp"
#include "utils/language.h"
#include <movie.h>
#include <DiabloUI/settingsmenu.h>

namespace devilution {
namespace {
int mainmenu_attract_time_out; // seconds
uint32_t dwAttractTicks;

/** The active music track id for the main menu. */
uint8_t menu_music_track_id = TMUSIC_INTRO;

void RefreshMusic()
{
	music_start(menu_music_track_id);

	if (gbIsSpawn && !gbIsHellfire) {
		return;
	}

	do {
		menu_music_track_id++;
		if (menu_music_track_id == NUM_MUSIC || (!gbIsHellfire && menu_music_track_id > TMUSIC_L4))
			menu_music_track_id = TMUSIC_L2;
		if (gbIsSpawn && menu_music_track_id > TMUSIC_L1)
			menu_music_track_id = TMUSIC_L5;
	} while (menu_music_track_id == TMUSIC_TOWN || menu_music_track_id == TMUSIC_L1);
}

bool InitMenu(_selhero_selections type)
{
	bool success;

	if (type == SELHERO_PREVIOUS)
		return true;

	music_stop();

	success = StartGame(type != SELHERO_CONTINUE, type != SELHERO_CONNECT);
	if (success)
		RefreshMusic();

	return success;
}

bool InitSinglePlayerMenu()
{
	gbIsMultiplayer = false;
	return InitMenu(SELHERO_NEW_DUNGEON);
}

bool InitMultiPlayerMenu()
{
	gbIsMultiplayer = true;
	return InitMenu(SELHERO_CONNECT);
}

void PlayIntro()
{
	music_stop();
	if (gbIsHellfire)
		play_movie("gendata\\Hellfire.smk", true);
	else
		play_movie("gendata\\diablo1.smk", true);
	RefreshMusic();
}

void WaitForButtonSound()
{
	SDL_FillRect(DiabloUiSurface(), nullptr, 0x000000);
	UiFadeIn();
	SDL_Delay(350); // delay to let button pressed sound finish playing
}

class MainMenuDialog : public MainLoopHandler {
public:
	MainMenuDialog(const char *name)
	    : name(name)
	{
	}

	~MainMenuDialog() override
	{

		instance_ = nullptr;
	}

	void Activated() override
	{
		mainmenu_attract_time_out = 5;
		mainmenu_restart_repintro(); // for automatic starts
		gfnSoundFunction = effects_play_sound;

		listItems_.push_back(std::make_unique<UiListItem>(_("Single Player"), MAINMENU_SINGLE_PLAYER));
		listItems_.push_back(std::make_unique<UiListItem>(_("Multi Player"), MAINMENU_MULTIPLAYER));
		listItems_.push_back(std::make_unique<UiListItem>(_("Settings"), MAINMENU_SETTINGS));
		listItems_.push_back(std::make_unique<UiListItem>(_("Support"), MAINMENU_SHOW_SUPPORT));
		listItems_.push_back(std::make_unique<UiListItem>(_("Show Credits"), MAINMENU_SHOW_CREDITS));
		listItems_.push_back(std::make_unique<UiListItem>(gbIsHellfire ? _("Exit Hellfire") : _("Exit Diablo"), MAINMENU_EXIT_DIABLO));

		if (!gbIsSpawn || gbIsHellfire) {
			if (gbIsHellfire)
				LoadArt("ui_art\\mainmenuw.pcx", &ArtBackgroundWidescreen);
			LoadBackgroundArt("ui_art\\mainmenu.pcx");
		} else {
			LoadBackgroundArt("ui_art\\swmmenu.pcx");
		}

		UiAddBackground(&items_);
		UiAddLogo(&items_);

		if (gbIsSpawn && gbIsHellfire) {
			SDL_Rect rect1 = { (Sint16)(PANEL_LEFT), (Sint16)(UI_OFFSET_Y + 145), 640, 30 };
			items_.push_back(std::make_unique<UiArtText>(_("Shareware"), rect1, UiFlags::FontSize30 | UiFlags::ColorUiSilver | UiFlags::AlignCenter, 8));
		}

		items_.push_back(std::make_unique<UiList>(listItems_, listItems_.size(), PANEL_LEFT + 64, (UI_OFFSET_Y + 192), 510, 43, UiFlags::FontSize42 | UiFlags::ColorUiGold | UiFlags::AlignCenter, 5));

		SDL_Rect rect2 = { 17, (Sint16)(gnScreenHeight - 36), 605, 21 };
		items_.push_back(std::make_unique<UiArtText>(name, rect2, UiFlags::FontSize12 | UiFlags::ColorUiSilverDark));

		instance_ = this;
		UiInitList(nullptr, UiMainMenuSelect, MainmenuEsc, items_, true);
	}

	void Deactivated() override
	{
		UiInitList_clear();
		ArtBackgroundWidescreen.Unload();
		ArtBackground.Unload();
		listItems_.clear();
		items_.clear();
	}

	void HandleEvent(SDL_Event &event) override
	{
		UiFocusNavigation(&event);
		UiHandleEvents(&event);
	}

	void Render() override
	{
		UiClearScreen();
		UiRender();
		if (SDL_GetTicks() >= dwAttractTicks && (diabdat_mpq || hellfire_mpq)) {
			PlayIntro();
		}
	}

	void Select(int value)
	{
		auto selected = (_mainmenu_selections)listItems_[value]->m_value;
		this->Deactivated(); // this line can be removed if everything is converted to MainLoopHandler
		switch (selected) {
		case MAINMENU_NONE:
			break;
		case MAINMENU_SINGLE_PLAYER:
			InitSinglePlayerMenu();
			break;
		case MAINMENU_MULTIPLAYER:
			InitMultiPlayerMenu();
			break;
		case MAINMENU_SHOW_CREDITS:
			UiCreditsDialog();
			break;
		case MAINMENU_SHOW_SUPPORT:
			UiSupportDialog();
			break;
		case MAINMENU_EXIT_DIABLO:
			WaitForButtonSound();
			Close();
			break;
		case MAINMENU_SETTINGS:
			UiSettingsMenu();
			break;
		}
		this->Activated(); // this line can be removed if everything is converted to MainLoopHandler
	}

	void Esc()
	{
		const size_t last = listItems_.size() - 1;
		if (SelectedItem == last) {
			Select(static_cast<int>(last));
		} else {
			SelectedItem = last;
		}
	}

private:
	static void UiMainMenuSelect(int value)
	{
		instance_->Select(value);
	}

	static void MainmenuEsc()
	{
		instance_->Esc();
	}

	static MainMenuDialog *instance_;

	std::vector<std::unique_ptr<UiItemBase>> items_;
	std::vector<std::unique_ptr<UiListItem>> listItems_;

	const char *name;
};

MainMenuDialog *MainMenuDialog::instance_ = nullptr;

} // namespace

void mainmenu_restart_repintro()
{
	dwAttractTicks = SDL_GetTicks() + mainmenu_attract_time_out * 1000;
}

bool UiMainMenuDialog(const char *name)
{
	AddMainLoopHandler(std::make_unique<MainMenuDialog>(name));
	return true;
}

} // namespace devilution
