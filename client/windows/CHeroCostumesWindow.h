/*
 * CHeroCostumesWindow.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CWindowObject.h"

class CButton;
class CLabel;
class CLabelGroup;
class CGHeroInstance;

/// Window for managing artifact costumes of a hero
class CHeroCostumesWindow : public CStatusbarWindow
{
public:
	CHeroCostumesWindow(const CGHeroInstance * hero);
	void notFocusedClick() override;
	void keyPressed(EShortcut key) override;

protected:
	const CGHeroInstance * hero;

	std::shared_ptr<CLabel> title;
	std::shared_ptr<CLabelGroup> rowLabels;
	std::vector<std::shared_ptr<CButton>> loadButtons;
	std::vector<std::shared_ptr<CButton>> saveButtons;

	bool hasCostume(size_t idx) const;
	bool heroHasEquippedArtifacts() const;
	void updateLoadButtonState(size_t idx, bool enabled);
	void saveCostume(size_t idx);
	void loadCostume(size_t idx);
};
