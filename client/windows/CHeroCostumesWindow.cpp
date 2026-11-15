/*
 * CHeroCostumesWindow.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "CHeroCostumesWindow.h"

#include "../CPlayerInterface.h"
#include "../GameInstance.h"
#include "../gui/Shortcut.h"

#include "../widgets/Buttons.h"
#include "../widgets/Images.h"
#include "../widgets/TextControls.h"

#include "../../lib/CPlayerState.h"
#include "../../lib/GameLibrary.h"
#include "../../lib/callback/CCallback.h"
#include "../../lib/entities/artifact/ArtifactUtils.h"
#include "../../lib/mapObjects/CGHeroInstance.h"
#include "../../lib/texts/CGeneralTextHandler.h"

namespace
{
	constexpr size_t costumeSlotsCount = 10;
}

CHeroCostumesWindow::CHeroCostumesWindow(const CGHeroInstance * hero)
	: CWindowObject(PLAYER_COLORED_BORDERED_STATUSBAR, ImagePath::builtin("heroCostumesDialog"))
	, hero(hero)
{
	OBJECT_CONSTRUCTION;

	title = std::make_shared<CLabel>(pos.w / 2, 15, FONT_SMALL, ETextAlignment::CENTER, Colors::YELLOW,
		LIBRARY->generaltexth->translate("vcmi.heroWindow.artifactCostumes.title"));

	// each row consists of costume label and two buttons, laid out left-to-right within the window width
	const Point rowOffset(22, 38);
	const int rowStride = 30; // button height + spacing
	const int loadButtonOffsetX = 142;
	const int saveButtonOffsetX = 267;

	rowLabels = std::make_shared<CLabelGroup>(EFonts::FONT_SMALL, ETextAlignment::CENTERLEFT, Colors::WHITE);

	for(size_t idx = 0; idx < costumeSlotsCount; ++idx)
	{
		const int rowTop = rowOffset.y + rowStride * idx;

		const std::string labelText = boost::str(boost::format(LIBRARY->generaltexth->translate("vcmi.heroWindow.artifactCostumes.slot")) % (idx + 1));
		rowLabels->add(rowOffset.x, rowTop + 12, labelText);

		loadButtons.emplace_back(std::make_shared<CButton>(Point(rowOffset.x + loadButtonOffsetX, rowTop), AnimationPath::builtin("heroBackpackSort"),
			CButton::tooltipLocalized("vcmi.heroWindow.artifactCostumes.load"),
			[this, idx]() { loadCostume(idx); },
			static_cast<EShortcut>(vstd::to_underlying(EShortcut::HERO_COSTUME_LOAD_0) + idx)));
		updateLoadButtonState(idx, hasCostume(idx));

		saveButtons.emplace_back(std::make_shared<CButton>(Point(rowOffset.x + saveButtonOffsetX, rowTop), AnimationPath::builtin("heroBackpackSort"),
			CButton::tooltipLocalized("vcmi.heroWindow.artifactCostumes.save"),
			[this, idx]() { saveCostume(idx); },
			static_cast<EShortcut>(vstd::to_underlying(EShortcut::HERO_COSTUME_SAVE_0) + idx)));
		saveButtons.back()->setTextOverlay(LIBRARY->generaltexth->translate("vcmi.heroWindow.artifactCostumes.save.hover"), EFonts::FONT_SMALL, Colors::YELLOW);
	}

	statusbar = CGStatusBar::create(std::make_shared<CPicture>(background->getSurface(), Rect(8, pos.h - 26, pos.w - 16, 19), 8, pos.h - 26));

	addUsedEvents(LCLICK | KEYBOARD);
	center();
}

void CHeroCostumesWindow::notFocusedClick()
{
	close();
}

void CHeroCostumesWindow::keyPressed(EShortcut key)
{
	if(key == EShortcut::GLOBAL_RETURN)
		close();
}

bool CHeroCostumesWindow::hasCostume(size_t idx) const
{
	const auto * playerState = GAME->interface()->cb->getPlayerState(GAME->interface()->playerID);
	if(playerState == nullptr)
		return false;

	const auto & costumes = playerState->costumesArtifacts;
	const auto costume = costumes.find(idx);
	return costume != costumes.end() && !costume->second.empty();
}

bool CHeroCostumesWindow::heroHasEquippedArtifacts() const
{
	for(const auto & slot : ArtifactUtils::commonWornSlots())
	{
		if(const auto * slotInfo = hero->getSlot(slot); slotInfo != nullptr && !slotInfo->locked)
			return true;
	}
	return false;
}

void CHeroCostumesWindow::saveCostume(size_t idx)
{
	GAME->interface()->cb->manageHeroCostume(hero->id, idx, true);
	// client state is not updated yet, enable the button based on what is about to be saved
	updateLoadButtonState(idx, heroHasEquippedArtifacts());
}

void CHeroCostumesWindow::updateLoadButtonState(size_t idx, bool enabled)
{
	loadButtons[idx]->block(!enabled);
	loadButtons[idx]->setTextOverlay(LIBRARY->generaltexth->translate("vcmi.heroWindow.artifactCostumes.load.hover"), EFonts::FONT_SMALL,
		enabled ? Colors::YELLOW : Colors::ORANGE);
}

void CHeroCostumesWindow::loadCostume(size_t idx)
{
	GAME->interface()->cb->manageHeroCostume(hero->id, idx, false);
}
