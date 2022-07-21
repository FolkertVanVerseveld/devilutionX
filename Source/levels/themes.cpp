/**
 * @file themes.cpp
 *
 * Implementation of the theme room placing algorithms.
 */
#include "levels/themes.h"

#include <fmt/core.h>

#include "engine/path.h"
#include "engine/points_in_rectangle_range.hpp"
#include "engine/random.hpp"
#include "items.h"
#include "levels/trigs.h"
#include "monster.h"
#include "objects.h"
#include "quests.h"
#include "utils/str_cat.hpp"

namespace devilution {

int numthemes;
bool armorFlag;
bool weaponFlag;
bool treasureFlag;
bool mFountainFlag;
bool cauldronFlag;
bool tFountainFlag;
int zharlib;
int themex;
int themey;
int themeVar1;
ThemeStruct themes[MAXTHEMES];
bool pFountainFlag;
bool bFountainFlag;

/** Specifies the set of special theme IDs from which one will be selected at random. */
theme_id ThemeGood[4] = { THEME_GOATSHRINE, THEME_SHRINE, THEME_SKELROOM, THEME_LIBRARY };
/** Specifies a 5x5 area to fit theme objects. */
int trm5x[] = {
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2
};
/** Specifies a 5x5 area to fit theme objects. */
int trm5y[] = {
	-2, -2, -2, -2, -2,
	-1, -1, -1, -1, -1,
	0, 0, 0, 0, 0,
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2
};
bool TFit_Shrine(int i)
{
	int xp = 0;
	int yp = 0;
	int found = 0;

	while (found == 0) {
		Point testPosition { xp, yp };
		if (dTransVal[xp][yp] == themes[i].ttval) {
			if (TileHasAny(dPiece[xp][yp - 1], TileProperties::Trap)
			    && IsTileNotSolid(testPosition + Direction::NorthWest)
			    && IsTileNotSolid(testPosition + Direction::SouthEast)
			    && dTransVal[xp - 1][yp] == themes[i].ttval
			    && dTransVal[xp + 1][yp] == themes[i].ttval
			    && !IsObjectAtPosition(testPosition + Direction::North)
			    && !IsObjectAtPosition(testPosition + Direction::East)) {
				found = 1;
			}
			if (found == 0
			    && TileHasAny(dPiece[xp - 1][yp], TileProperties::Trap)
			    && IsTileNotSolid(testPosition + Direction::NorthEast)
			    && IsTileNotSolid(testPosition + Direction::SouthWest)
			    && dTransVal[xp][yp - 1] == themes[i].ttval
			    && dTransVal[xp][yp + 1] == themes[i].ttval
			    && !IsObjectAtPosition(testPosition + Direction::North)
			    && !IsObjectAtPosition(testPosition + Direction::West)) {
				found = 2;
			}
		}
		if (found == 0) {
			xp++;
			if (xp == MAXDUNX) {
				xp = 0;
				yp++;
				if (yp == MAXDUNY)
					return false;
			}
		}
	}
	themex = xp;
	themey = yp;
	themeVar1 = found;
	return true;
}

bool TFit_Obj5(int t)
{
	int xp = 0;
	int yp = 0;
	int r = GenerateRnd(5) + 1;
	int rs = r;

	while (r > 0) {
		bool found = false;
		if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp })) {
			found = true;
			for (int i = 0; found && i < 25; i++) {
				if (TileHasAny(dPiece[xp + trm5x[i]][yp + trm5y[i]], TileProperties::Solid)) {
					found = false;
				}
				if (dTransVal[xp + trm5x[i]][yp + trm5y[i]] != themes[t].ttval) {
					found = false;
				}
			}
		}

		if (!found) {
			xp++;
			if (xp == MAXDUNX) {
				xp = 0;
				yp++;
				if (yp == MAXDUNY) {
					if (r == rs) {
						return false;
					}
					yp = 0;
				}
			}
			continue;
		}

		r--;
	}

	themex = xp;
	themey = yp;

	return true;
}

bool TFit_SkelRoom(int t)
{
	if (IsNoneOf(leveltype, DTYPE_CATHEDRAL, DTYPE_CATACOMBS)) {
		return false;
	}

	for (size_t i = 0; i < LevelMonsterTypeCount; i++) {
		if (IsSkel(LevelMonsterTypes[i].type)) {
			themeVar1 = i;
			return TFit_Obj5(t);
		}
	}

	return false;
}

bool TFit_GoatShrine(int t)
{
	for (size_t i = 0; i < LevelMonsterTypeCount; i++) {
		if (IsGoat(LevelMonsterTypes[i].type)) {
			themeVar1 = i;
			return TFit_Obj5(t);
		}
	}

	return false;
}

bool CheckThemeObj3(Point origin, int8_t regionId, unsigned frequency = std::numeric_limits<unsigned>::max())
{
	const PointsInRectangleRange searchArea { Rectangle { origin, 1 } };
	return std::all_of(searchArea.cbegin(), searchArea.cend(), [regionId, frequency](Point testPosition) {
		if (!InDungeonBounds(testPosition)) {
			return false;
		}
		if (IsTileSolid(testPosition)) {
			return false;
		}
		// If the theme object would extend into a different region then it doesn't fit.
		if (dTransVal[testPosition.x][testPosition.y] != regionId) {
			return false;
		}
		if (IsObjectAtPosition(testPosition)) {
			return false;
		}
		if (frequency != std::numeric_limits<unsigned>::max() && FlipCoin(frequency)) {
			return false;
		}
		return true;
	});
}

bool TFit_Obj3(int8_t regionId)
{
	constexpr unsigned objrnd[4] = { 4, 4, 3, 5 };

	for (int yp = 1; yp < MAXDUNY - 1; yp++) {
		for (int xp = 1; xp < MAXDUNX - 1; xp++) {
			if (CheckThemeObj3({ xp, yp }, regionId, objrnd[leveltype - 1])) {
				themex = xp;
				themey = yp;
				return true;
			}
		}
	}

	return false;
}

bool CheckThemeReqs(theme_id t)
{
	switch (t) {
	case THEME_SHRINE:
	case THEME_SKELROOM:
	case THEME_LIBRARY:
		if (leveltype == DTYPE_CAVES || leveltype == DTYPE_HELL) {
			return false;
		}
		break;
	case THEME_BLOODFOUNTAIN:
		if (!bFountainFlag) {
			return false;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		if (!pFountainFlag) {
			return false;
		}
		break;
	case THEME_ARMORSTAND:
		if (leveltype == DTYPE_CATHEDRAL) {
			return false;
		}
		break;
	case THEME_CAULDRON:
		if (leveltype != DTYPE_HELL || !cauldronFlag) {
			return false;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		if (!mFountainFlag) {
			return false;
		}
		break;
	case THEME_TEARFOUNTAIN:
		if (!tFountainFlag) {
			return false;
		}
		break;
	case THEME_WEAPONRACK:
		if (leveltype == DTYPE_CATHEDRAL) {
			return false;
		}
		break;
	default:
		break;
	}

	return true;
}

static bool SpecialThemeFit(int i, theme_id t)
{
	bool rv;

	rv = CheckThemeReqs(t);
	switch (t) {
	case THEME_SHRINE:
	case THEME_LIBRARY:
		if (rv) {
			rv = TFit_Shrine(i);
		}
		break;
	case THEME_SKELROOM:
		if (rv) {
			rv = TFit_SkelRoom(i);
		}
		break;
	case THEME_BLOODFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(i);
		}
		if (rv) {
			bFountainFlag = false;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(i);
		}
		if (rv) {
			pFountainFlag = false;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(i);
		}
		if (rv) {
			mFountainFlag = false;
		}
		break;
	case THEME_TEARFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(i);
		}
		if (rv) {
			tFountainFlag = false;
		}
		break;
	case THEME_CAULDRON:
		if (rv) {
			rv = TFit_Obj5(i);
		}
		if (rv) {
			cauldronFlag = false;
		}
		break;
	case THEME_GOATSHRINE:
		if (rv) {
			rv = TFit_GoatShrine(i);
		}
		break;
	case THEME_TORTURE:
	case THEME_DECAPITATED:
	case THEME_ARMORSTAND:
	case THEME_BRNCROSS:
	case THEME_WEAPONRACK:
		if (rv) {
			rv = TFit_Obj3(themes[i].ttval);
		}
		break;
	case THEME_TREASURE:
		rv = treasureFlag;
		if (rv) {
			treasureFlag = false;
		}
		break;
	default:
		break;
	}

	return rv;
}

bool CheckThemeRoom(int tv)
{
	for (int i = 0; i < numtrigs; i++) {
		if (dTransVal[trigs[i].position.x][trigs[i].position.y] == tv)
			return false;
	}

	int tarea = 0;
	for (int j = 0; j < MAXDUNY; j++) {
		for (int i = 0; i < MAXDUNX; i++) {
			if (dTransVal[i][j] != tv)
				continue;
			if (TileContainsSetPiece({ i, j }))
				return false;

			tarea++;
		}
	}

	if (leveltype == DTYPE_CATHEDRAL && (tarea < 9 || tarea > 100))
		return false;

	for (int j = 0; j < MAXDUNY; j++) {
		for (int i = 0; i < MAXDUNX; i++) {
			if (dTransVal[i][j] != tv || TileHasAny(dPiece[i][j], TileProperties::Solid))
				continue;
			if (dTransVal[i - 1][j] != tv && IsTileNotSolid({ i - 1, j }))
				return false;
			if (dTransVal[i + 1][j] != tv && IsTileNotSolid({ i + 1, j }))
				return false;
			if (dTransVal[i][j - 1] != tv && IsTileNotSolid({ i, j - 1 }))
				return false;
			if (dTransVal[i][j + 1] != tv && IsTileNotSolid({ i, j + 1 }))
				return false;
		}
	}

	return true;
}

void InitThemes()
{
	zharlib = -1;
	numthemes = 0;
	armorFlag = true;
	bFountainFlag = true;
	cauldronFlag = true;
	mFountainFlag = true;
	pFountainFlag = true;
	tFountainFlag = true;
	treasureFlag = true;
	weaponFlag = true;

	if (currlevel == 16 || IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT)) {
		return;
	}

	if (leveltype == DTYPE_CATHEDRAL) {
		for (size_t i = 0; i < 256 && numthemes < MAXTHEMES; i++) {
			if (CheckThemeRoom(i)) {
				themes[numthemes].ttval = i;
				theme_id j = ThemeGood[GenerateRnd(4)];
				while (!SpecialThemeFit(numthemes, j)) {
					j = (theme_id)GenerateRnd(17);
				}
				themes[numthemes].ttype = j;
				numthemes++;
			}
		}
		return;
	}

	for (int i = 0; i < themeCount; i++) {
		themes[i].ttype = THEME_NONE;
	}

	if (Quests[Q_ZHAR].IsAvailable()) {
		for (int j = 0; j < themeCount; j++) {
			themes[j].ttval = themeLoc[j].ttval;
			if (SpecialThemeFit(j, THEME_LIBRARY)) {
				themes[j].ttype = THEME_LIBRARY;
				zharlib = j;
				break;
			}
		}
	}
	for (int i = 0; i < themeCount; i++) {
		if (themes[i].ttype == THEME_NONE) {
			themes[i].ttval = themeLoc[i].ttval;
			theme_id j = ThemeGood[GenerateRnd(4)];
			while (!SpecialThemeFit(i, j)) {
				j = (theme_id)GenerateRnd(17);
			}
			themes[i].ttype = j;
		}
	}
	numthemes += themeCount;
}

void HoldThemeRooms()
{
	if (currlevel == 16 || IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT)) {
		return;
	}

	if (leveltype != DTYPE_CATHEDRAL) {
		DRLG_HoldThemeRooms();
		return;
	}

	for (int i = 0; i < numthemes; i++) {
		int8_t v = themes[i].ttval;
		for (int y = 0; y < MAXDUNY; y++) {
			for (int x = 0; x < MAXDUNX; x++) {
				if (dTransVal[x][y] == v) {
					dFlags[x][y] |= DungeonFlag::Populated;
				}
			}
		}
	}
}

/**
 * PlaceThemeMonsts places theme monsters with the specified frequency.
 *
 * @param t theme number (index into themes array).
 * @param f frequency (1/f likelihood of adding monster).
 */
void PlaceThemeMonsts(int t, int f)
{
	int scattertypes[138];

	int numscattypes = 0;
	for (size_t i = 0; i < LevelMonsterTypeCount; i++) {
		if ((LevelMonsterTypes[i].placeFlags & PLACE_SCATTER) != 0) {
			scattertypes[numscattypes] = i;
			numscattypes++;
		}
	}
	int mtype = scattertypes[GenerateRnd(numscattypes)];
	for (int yp = 0; yp < MAXDUNY; yp++) {
		for (int xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp }) && dItem[xp][yp] == 0 && !IsObjectAtPosition({ xp, yp })) {
				if (FlipCoin(f)) {
					AddMonster({ xp, yp }, static_cast<Direction>(GenerateRnd(8)), mtype, true);
				}
			}
		}
	}
}

/**
 * Theme_Barrel initializes the barrel theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Barrel(int t)
{
	int barrnd[4] = { 2, 6, 4, 8 };
	int monstrnd[4] = { 5, 7, 3, 9 };

	for (int yp = 0; yp < MAXDUNY; yp++) {
		for (int xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp })) {
				if (FlipCoin(barrnd[leveltype - 1])) {
					_object_id r = FlipCoin(barrnd[leveltype - 1]) ? OBJ_BARREL : OBJ_BARRELEX;
					AddObject(r, { xp, yp });
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_Shrine initializes the shrine theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Shrine(int t)
{
	int monstrnd[4] = { 6, 6, 3, 9 };

	TFit_Shrine(t);
	if (themeVar1 == 1) {
		AddObject(OBJ_CANDLE2, { themex - 1, themey });
		AddObject(OBJ_SHRINER, { themex, themey });
		AddObject(OBJ_CANDLE2, { themex + 1, themey });
	} else {
		AddObject(OBJ_CANDLE2, { themex, themey - 1 });
		AddObject(OBJ_SHRINEL, { themex, themey });
		AddObject(OBJ_CANDLE2, { themex, themey + 1 });
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_MonstPit initializes the monster pit theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_MonstPit(int t)
{
	int monstrnd[4] = { 6, 7, 3, 9 };

	int r = GenerateRnd(100) + 1;
	int ixp = 0;
	int iyp = 0;
	while (r > 0) {
		if (dTransVal[ixp][iyp] == themes[t].ttval && IsTileNotSolid({ ixp, iyp })) {
			--r;
		}
		if (r <= 0)
			continue;
		ixp++;
		if (ixp == MAXDUNX) {
			ixp = 0;
			iyp++;
			if (iyp == MAXDUNY) {
				iyp = 0;
			}
		}
	}
	CreateRndItem({ ixp, iyp }, true, false, true);
	ItemNoFlippy();
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

namespace {
void SpawnObjectOrSkeleton(unsigned frequency, _object_id objectType, Point tile)
{
	if (FlipCoin(frequency)) {
		AddObject(objectType, tile);
	} else {
		Monster *skeleton = PreSpawnSkeleton();
		if (skeleton != nullptr)
			ActivateSkeleton(*skeleton, tile);
	}
}
} // namespace

/**
 * Theme_SkelRoom initializes the skeleton room theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_SkelRoom(int t)
{
	constexpr unsigned monstrnd[4] = { 6, 7, 3, 9 };

	TFit_SkelRoom(t);

	int xp = themex;
	int yp = themey;

	AddObject(OBJ_SKFIRE, { xp, yp });

	SpawnObjectOrSkeleton(monstrnd[leveltype - 1], OBJ_BANNERL, { xp - 1, yp - 1 });

	{
		Monster *skeleton = PreSpawnSkeleton();
		if (skeleton != nullptr)
			ActivateSkeleton(*skeleton, { xp, yp - 1 });
	}

	SpawnObjectOrSkeleton(monstrnd[leveltype - 1], OBJ_BANNERR, { xp + 1, yp - 1 });

	SpawnObjectOrSkeleton(monstrnd[leveltype - 1], OBJ_BANNERM, { xp - 1, yp });

	SpawnObjectOrSkeleton(monstrnd[leveltype - 1], OBJ_BANNERM, { xp + 1, yp });

	SpawnObjectOrSkeleton(monstrnd[leveltype - 1], OBJ_BANNERR, { xp - 1, yp + 1 });

	{
		Monster *skeleton = PreSpawnSkeleton();
		if (skeleton != nullptr)
			ActivateSkeleton(*skeleton, { xp, yp + 1 });
	}

	SpawnObjectOrSkeleton(monstrnd[leveltype - 1], OBJ_BANNERL, { xp + 1, yp + 1 });

	if (!IsObjectAtPosition({ xp, yp - 3 })) {
		AddObject(OBJ_SKELBOOK, { xp, yp - 2 });
	}
	if (!IsObjectAtPosition({ xp, yp + 3 })) {
		AddObject(OBJ_SKELBOOK, { xp, yp + 2 });
	}
}

/**
 * Theme_Treasure initializes the treasure theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Treasure(int t)
{
	int treasrnd[4] = { 4, 9, 7, 10 };
	int monstrnd[4] = { 6, 8, 3, 7 };

	AdvanceRndSeed();
	for (int yp = 0; yp < MAXDUNY; yp++) {
		for (int xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp })) {
				int8_t treasureType = treasrnd[leveltype - 1];
				int rv = GenerateRnd(treasureType);
				// BUGFIX: this used to be `2*GenerateRnd(treasureType) == 0` however 2*0 has no effect, should probably be `FlipCoin(2*treasureType)`
				if (FlipCoin(treasureType)) {
					CreateTypeItem({ xp, yp }, false, ItemType::Gold, IMISC_NONE, false, true);
					ItemNoFlippy();
				}
				if (rv == 0) {
					CreateRndItem({ xp, yp }, false, false, true);
					ItemNoFlippy();
				}
				if (rv >= treasureType - 2 && leveltype != DTYPE_CATHEDRAL) {
					Item &item = Items[ActiveItems[ActiveItemCount - 1]];
					if (item.IDidx == IDI_GOLD) {
						item._ivalue = std::max(item._ivalue / 2, 1);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_Library initializes the library theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Library(int t)
{
	constexpr unsigned librnd[4] = { 1, 2, 2, 5 };
	int monstrnd[4] = { 5, 7, 3, 9 };

	TFit_Shrine(t);

	if (themeVar1 == 1) {
		AddObject(OBJ_BOOKCANDLE, { themex - 1, themey });
		AddObject(OBJ_BOOKCASER, { themex, themey });
		AddObject(OBJ_BOOKCANDLE, { themex + 1, themey });
	} else {
		AddObject(OBJ_BOOKCANDLE, { themex, themey - 1 });
		AddObject(OBJ_BOOKCASEL, { themex, themey });
		AddObject(OBJ_BOOKCANDLE, { themex, themey + 1 });
	}

	for (int yp = 1; yp < MAXDUNY - 1; yp++) {
		for (int xp = 1; xp < MAXDUNX - 1; xp++) {
			if (CheckThemeObj3({ xp, yp }, themes[t].ttval) && dMonster[xp][yp] == 0 && FlipCoin(librnd[leveltype - 1])) {
				AddObject(OBJ_BOOKSTAND, { xp, yp });
				if (!FlipCoin(2 * librnd[leveltype - 1])) {
					Object *bookstand = ObjectAtPosition({ xp, yp });
					if (bookstand != nullptr) {
						bookstand->_oSelFlag = 0;
						bookstand->_oAnimFrame += 2;
					}
				}
			}
		}
	}

	if (Quests[Q_ZHAR].IsAvailable() && t == zharlib) {
		return;
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_Torture initializes the torture theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Torture(int t)
{
	constexpr unsigned tortrnd[4] = { 6, 8, 3, 8 };
	int monstrnd[4] = { 6, 8, 3, 9 };

	for (int yp = 1; yp < MAXDUNY - 1; yp++) {
		for (int xp = 1; xp < MAXDUNX - 1; xp++) {
			if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp })) {
				if (CheckThemeObj3({ xp, yp }, themes[t].ttval)) {
					if (FlipCoin(tortrnd[leveltype - 1])) {
						AddObject(OBJ_TNUDEM2, { xp, yp });
					}
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_BloodFountain initializes the blood fountain theme.
 * @param t Theme number (index into themes array).
 */
void Theme_BloodFountain(int t)
{
	int monstrnd[4] = { 6, 8, 3, 9 };

	TFit_Obj5(t);
	AddObject(OBJ_BLOODFTN, { themex, themey });
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_Decap initializes the decapitated theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Decap(int t)
{
	constexpr unsigned decaprnd[4] = { 6, 8, 3, 8 };
	int monstrnd[4] = { 6, 8, 3, 9 };

	for (int yp = 1; yp < MAXDUNY - 1; yp++) {
		for (int xp = 1; xp < MAXDUNX - 1; xp++) {
			if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp })) {
				if (CheckThemeObj3({ xp, yp }, themes[t].ttval)) {
					if (FlipCoin(decaprnd[leveltype - 1])) {
						AddObject(OBJ_DECAP, { xp, yp });
					}
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_PurifyingFountain initializes the purifying fountain theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_PurifyingFountain(int t)
{
	int monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(t);
	AddObject(OBJ_PURIFYINGFTN, { themex, themey });
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_ArmorStand initializes the armor stand theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_ArmorStand(int t)
{
	constexpr unsigned armorrnd[4] = { 6, 8, 3, 8 };
	int monstrnd[4] = { 6, 7, 3, 9 };

	if (armorFlag) {
		TFit_Obj3(themes[t].ttval);
		AddObject(OBJ_ARMORSTAND, { themex, themey });
	}
	for (int yp = 0; yp < MAXDUNY; yp++) {
		for (int xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == themes[t].ttval && IsTileNotSolid({ xp, yp })) {
				if (CheckThemeObj3({ xp, yp }, themes[t].ttval)) {
					if (FlipCoin(armorrnd[leveltype - 1])) {
						AddObject(OBJ_ARMORSTANDN, { xp, yp });
					}
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
	armorFlag = false;
}

/**
 * Theme_GoatShrine initializes the goat shrine theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_GoatShrine(int t)
{
	TFit_GoatShrine(t);
	AddObject(OBJ_GOATSHRINE, { themex, themey });
	for (int yy = themey - 1; yy <= themey + 1; yy++) {
		for (int xx = themex - 1; xx <= themex + 1; xx++) {
			if (dTransVal[xx][yy] == themes[t].ttval && IsTileNotSolid({ xx, yy }) && (xx != themex || yy != themey)) {
				AddMonster({ xx, yy }, Direction::SouthWest, themeVar1, true);
			}
		}
	}
}

/**
 * Theme_Cauldron initializes the cauldron theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_Cauldron(int t)
{
	int monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(t);
	AddObject(OBJ_CAULDRON, { themex, themey });
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_MurkyFountain initializes the murky fountain theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_MurkyFountain(int t)
{
	int monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(t);
	AddObject(OBJ_MURKYFTN, { themex, themey });
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_TearFountain initializes the tear fountain theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_TearFountain(int t)
{
	int monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(t);
	AddObject(OBJ_TEARFTN, { themex, themey });
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_BrnCross initializes the burning cross theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_BrnCross(int t)
{
	int8_t regionId = themes[t].ttval;
	int monstrnd[4] = { 6, 8, 3, 9 };
	constexpr unsigned bcrossrnd[4] = { 5, 7, 3, 8 };

	for (int yp = 0; yp < MAXDUNY; yp++) {
		for (int xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == regionId && IsTileNotSolid({ xp, yp })) {
				if (CheckThemeObj3({ xp, yp }, regionId)) {
					if (FlipCoin(bcrossrnd[leveltype - 1])) {
						AddObject(OBJ_TBCROSS, { xp, yp });
					}
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
}

/**
 * Theme_WeaponRack initializes the weapon rack theme.
 *
 * @param t theme number (index into themes array).
 */
void Theme_WeaponRack(int t)
{
	int8_t regionId = themes[t].ttval;
	constexpr unsigned weaponrnd[4] = { 6, 8, 5, 8 };
	int monstrnd[4] = { 6, 7, 3, 9 };

	if (weaponFlag) {
		TFit_Obj3(regionId);
		AddObject(OBJ_WEAPONRACK, { themex, themey });
	}
	for (int yp = 0; yp < MAXDUNY; yp++) {
		for (int xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == regionId && IsTileNotSolid({ xp, yp })) {
				if (CheckThemeObj3({ xp, yp }, regionId)) {
					if (FlipCoin(weaponrnd[leveltype - 1])) {
						AddObject(OBJ_WEAPONRACKN, { xp, yp });
					}
				}
			}
		}
	}
	PlaceThemeMonsts(t, monstrnd[leveltype - 1]);
	weaponFlag = false;
}

/**
 * UpdateL4Trans sets each value of the transparency map to 1.
 */
void UpdateL4Trans()
{
	for (int j = 0; j < MAXDUNY; j++) {
		for (int i = 0; i < MAXDUNX; i++) { // NOLINT(modernize-loop-convert)
			if (dTransVal[i][j] != 0) {
				dTransVal[i][j] = 1;
			}
		}
	}
}

void CreateThemeRooms()
{
	if (currlevel == 16 || IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT)) {
		return;
	}

	ApplyObjectLighting = true;
	for (int i = 0; i < numthemes; i++) {
		themex = 0;
		themey = 0;
		switch (themes[i].ttype) {
		case THEME_BARREL:
			Theme_Barrel(i);
			break;
		case THEME_SHRINE:
			Theme_Shrine(i);
			break;
		case THEME_MONSTPIT:
			Theme_MonstPit(i);
			break;
		case THEME_SKELROOM:
			Theme_SkelRoom(i);
			break;
		case THEME_TREASURE:
			Theme_Treasure(i);
			break;
		case THEME_LIBRARY:
			Theme_Library(i);
			break;
		case THEME_TORTURE:
			Theme_Torture(i);
			break;
		case THEME_BLOODFOUNTAIN:
			Theme_BloodFountain(i);
			break;
		case THEME_DECAPITATED:
			Theme_Decap(i);
			break;
		case THEME_PURIFYINGFOUNTAIN:
			Theme_PurifyingFountain(i);
			break;
		case THEME_ARMORSTAND:
			Theme_ArmorStand(i);
			break;
		case THEME_GOATSHRINE:
			Theme_GoatShrine(i);
			break;
		case THEME_CAULDRON:
			Theme_Cauldron(i);
			break;
		case THEME_MURKYFOUNTAIN:
			Theme_MurkyFountain(i);
			break;
		case THEME_TEARFOUNTAIN:
			Theme_TearFountain(i);
			break;
		case THEME_BRNCROSS:
			Theme_BrnCross(i);
			break;
		case THEME_WEAPONRACK:
			Theme_WeaponRack(i);
			break;
		case THEME_NONE:
			app_fatal(StrCat("Unknown theme type: ", static_cast<int>(themes[i].ttype)));
		}
	}
	ApplyObjectLighting = false;
	if (leveltype == DTYPE_HELL && themeCount > 0) {
		UpdateL4Trans();
	}
}

} // namespace devilution
