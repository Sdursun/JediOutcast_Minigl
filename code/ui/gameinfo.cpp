//
// gameinfo.c
//

// *** This file is used by both the game and the user interface ***


// ... and for that reason is excluded from PCH usage for the moment =Ste.


#include "gameinfo.h"
#include "../game/weapons.h"


static gameinfo_import_t	gameinfo_gi;	// not gi: the game module has its own (a game_import_t)

weaponData_t weaponData[WP_NUM_WEAPONS];
ammoData_t ammoData[AMMO_MAX];

extern void WP_LoadWeaponParms (void);

//
// Initialization - Read in files and parse into infos
//

/*
===============
GI_Init
===============
*/
void GI_Init( gameinfo_import_t *import ) {
	gameinfo_gi = *import;

	AMIGA_TRACE("GI_Init: WP_LoadWeaponParms");
	WP_LoadWeaponParms ();
	AMIGA_TRACE("GI_Init: done");
}
