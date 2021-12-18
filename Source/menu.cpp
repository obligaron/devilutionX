#include "menu.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/settingsmenu.h"
#include "engine/demomode.h"
#include "init.h"
#include "main_loop.hpp"
#include "movie.h"
#include "options.h"
#include "pfile.h"
#include "storm/storm_net.hpp"
#include "utils/language.h"

namespace devilution {

uint32_t gSaveNumber;

bool DummyGetHeroInfo(_uiheroinfo * /*pInfo*/)
{
	return true;
}

bool mainmenu_select_hero_dialog(GameData *gameData)
{
	uint32_t *pSaveNumberFromOptions = nullptr;
	_selhero_selections dlgresult = SELHERO_NEW_DUNGEON;
	if (demo::IsRunning()) {
		pfile_ui_set_hero_infos(DummyGetHeroInfo);
		gbLoadGame = true;
	} else if (!gbIsMultiplayer) {
		pSaveNumberFromOptions = gbIsHellfire ? &sgOptions.Hellfire.lastSinglePlayerHero : &sgOptions.Diablo.lastSinglePlayerHero;
		gSaveNumber = *pSaveNumberFromOptions;
		UiSelHeroSingDialog(
		    pfile_ui_set_hero_infos,
		    pfile_ui_save_create,
		    pfile_delete_save,
		    pfile_ui_set_class_stats,
		    &dlgresult,
		    &gSaveNumber,
		    &gameData->nDifficulty);

		gbLoadGame = (dlgresult == SELHERO_CONTINUE);
	} else {
		pSaveNumberFromOptions = gbIsHellfire ? &sgOptions.Hellfire.lastMultiplayerHero : &sgOptions.Diablo.lastMultiplayerHero;
		gSaveNumber = *pSaveNumberFromOptions;
		UiSelHeroMultDialog(
		    pfile_ui_set_hero_infos,
		    pfile_ui_save_create,
		    pfile_delete_save,
		    pfile_ui_set_class_stats,
		    &dlgresult,
		    &gSaveNumber);
	}
	if (dlgresult == SELHERO_PREVIOUS) {
		SErrSetLastError(1223);
		return false;
	}

	if (pSaveNumberFromOptions != nullptr)
		*pSaveNumberFromOptions = gSaveNumber;

	pfile_read_player_from_save(gSaveNumber, Players[MyPlayerId]);

	return true;
}

} // namespace devilution
