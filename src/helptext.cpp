/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "helptext.h"

#include "config.h"
#include "episodes.h"
#include "file.h"
#include "fonthand.h"
#include "menus.h"
#include "opentyr.h"
#include "video.h"

#include <assert.h>
#include <string.h>


const JE_byte menuHelp[MENU_MAX][11] = /* [1..maxmenu, 1..11] */
{
	{  1, 34,  2,  3,  4,  5,                  0, 0, 0, 0, 0 },
	{  6,  7,  8,  9, 10, 11, 11, 12,                0, 0, 0 },
	{ 13, 14, 15, 15, 16, 17, 12,                 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{  4, 30, 30,  3,  5,                   0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 16, 17, 15, 15, 12,                   0, 0, 0, 0, 0, 0 },
	{ 31, 31, 31, 31, 32, 12,                  0, 0, 0, 0, 0 },
	{  4, 34,  3,  5,                    0, 0, 0, 0, 0, 0, 0 }
};

JE_byte verticalHeight = 7;
JE_byte helpBoxColor = 12;
JE_byte helpBoxBrightness = 1;
JE_byte helpBoxShadeType = FULL_SHADE;

char helpTxt[39][231];                                                   /* [1..39] of string [230] */
char pName[21][16];                                                      /* [1..21] of string [15] */
char miscText[HELPTEXT_MISCTEXT_COUNT][42];                              /* [1..68] of string [41] */
char miscTextB[HELPTEXT_MISCTEXTB_COUNT][HELPTEXT_MISCTEXTB_SIZE];       /* [1..5] of string [10] */
char keyName[8][18];                                                     /* [1..8] of string [17] */
char menuText[7][HELPTEXT_MENUTEXT_SIZE];                                /* [1..7] of string [20] */
char outputs[9][31];                                                     /* [1..9] of string [30] */
char topicName[6][21];                                                   /* [1..6] of string [20] */
char mainMenuHelp[HELPTEXT_MAINMENUHELP_COUNT][66];                      /* [1..34] of string [65] */
char inGameText[6][21];                                                  /* [1..6] of string [20] */
char detailLevel[6][13];                                                 /* [1..6] of string [12] */
char gameSpeedText[5][13];                                               /* [1..5] of string [12] */
char inputDevices[3][13];                                                /* [1..3] of string [12] */
char networkText[HELPTEXT_NETWORKTEXT_COUNT][HELPTEXT_NETWORKTEXT_SIZE]; /* [1..4] of string [20] */
char difficultyNameB[11][21];                                            /* [0..9] of string [20] */
char joyButtonNames[5][21];                                              /* [1..5] of string [20] */
char superShips[HELPTEXT_SUPERSHIPS_COUNT][26];                          /* [0..10] of string [25] */
char specialName[HELPTEXT_SPECIALNAME_COUNT][10];                        /* [1..9] of string [9] */
char destructHelp[25][22];                                               /* [1..25] of string [21] */
char weaponNames[17][17];                                                /* [1..17] of string [16] */
char destructModeName[DESTRUCT_MODES][13];                               /* [1..destructmodes] of string [12] */
char shipInfo[HELPTEXT_SHIPINFO_COUNT][2][256];                          /* [1..13, 1..2] of string */
char menuInt[MENU_MAX+1][11][18];                                        /* [0..14, 1..11] of string [17] */


static void decrypt_string( char *s, size_t len )
{
	static const unsigned char crypt_key[] = { 204, 129, 63, 255, 71, 19, 25, 62, 1, 99 };

	if (len == 0)
		return;

	for (size_t i = len - 1; ; --i)
	{
		s[i] ^= crypt_key[i % sizeof(crypt_key)];
		if (i == 0)
			break;
		s[i] ^= s[i - 1];
	}
}

void read_encrypted_pascal_string( char *s, size_t size, FILE *f )
{
	Uint8 len;
	char buffer[255];

	fread_u8_die(&len, 1, f);
	fread_die(buffer, 1, len, f);

	if (size == 0)
		return;

	decrypt_string(buffer, len);

	assert(len < size);

	len = MIN(len, size - 1);
	memcpy(s, buffer, len);
	s[len] = '\0';
}

void skip_pascal_string( FILE *f )
{
	Uint8 len;
	char buffer[255];

	fread_u8_die(&len, 1, f);
	fread_die(buffer, 1, len, f);
}

void JE_helpBox( SDL_Surface *screen,  int x, int y, const char *message, unsigned int boxwidth )
{
	JE_byte startpos, endpos, pos;
	JE_boolean endstring;

	char substring[256];

	if (strlen(message) == 0)
	{
		return;
	}

	pos = 1;
	endpos = 0;
	endstring = false;

	do
	{
		startpos = endpos + 1;

		do
		{
			endpos = pos;
			do
			{
				pos++;
				if (pos == strlen(message))
				{
					endstring = true;
					if ((unsigned)(pos - startpos) < boxwidth)
					{
						endpos = pos + 1;
					}
				}

			} while (!(message[pos-1] == ' ' || endstring));

		} while (!((unsigned)(pos - startpos) > boxwidth || endstring));

		SDL_strlcpy(substring, message + startpos - 1, MIN((size_t)(endpos - startpos + 1), sizeof(substring)));
		JE_textShade(screen, x, y, substring, helpBoxColor, helpBoxBrightness, helpBoxShadeType);

		y += verticalHeight;

	} while (!endstring);

	if (endpos != pos + 1)
	{
		JE_textShade(screen, x, y, message + endpos, helpBoxColor, helpBoxBrightness, helpBoxShadeType);
	}

	helpBoxColor = 12;
	helpBoxShadeType = FULL_SHADE;
}

void JE_HBox( SDL_Surface *screen, int x, int y, unsigned int  messagenum, unsigned int boxwidth )
{
	JE_helpBox(screen, x, y, helpTxt[messagenum-1], boxwidth);
}

void JE_loadHelpText( void )
{
	const unsigned int menuInt_entries[MENU_MAX + 1] = { (unsigned int)-1, 7, 9, 8, (unsigned int)-1, (unsigned int)-1, 11, (unsigned int)-1, (unsigned int)-1, (unsigned int)-1, 6, 4, 6, 7, 5 };
	
	FILE *f = dir_fopen_die(data_dir(), "tyrian.hdt", "rb");
	JE_longint episode1DataLoc;
	fread_s32_die(&episode1DataLoc, 1, f);

	/*Online Help*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(helpTxt); ++i)
		read_encrypted_pascal_string(helpTxt[i], sizeof(helpTxt[i]), f);
	skip_pascal_string(f);

	/*Planet names*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(pName); ++i)
		read_encrypted_pascal_string(pName[i], sizeof(pName[i]), f);
	skip_pascal_string(f);

	/*Miscellaneous text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(miscText); ++i)
		read_encrypted_pascal_string(miscText[i], sizeof(miscText[i]), f);
	skip_pascal_string(f);

	/*Little Miscellaneous text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(miscTextB); ++i)
		read_encrypted_pascal_string(miscTextB[i], sizeof(miscTextB[i]), f);
	skip_pascal_string(f);

	/*Key names*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[6]; ++i)
		read_encrypted_pascal_string(menuInt[6][i], sizeof(menuInt[6][i]), f);
	skip_pascal_string(f);

	/*Main Menu*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(menuText); ++i)
		read_encrypted_pascal_string(menuText[i], sizeof(menuText[i]), f);
	skip_pascal_string(f);

	/*Event text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(outputs); ++i)
		read_encrypted_pascal_string(outputs[i], sizeof(outputs[i]), f);
	skip_pascal_string(f);

	/*Help topics*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(topicName); ++i)
		read_encrypted_pascal_string(topicName[i], sizeof(topicName[i]), f);
	skip_pascal_string(f);

	/*Main Menu Help*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(mainMenuHelp); ++i)
		read_encrypted_pascal_string(mainMenuHelp[i], sizeof(mainMenuHelp[i]), f);
	skip_pascal_string(f);

	/*Menu 1 - Main*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[1]; ++i)
		read_encrypted_pascal_string(menuInt[1][i], sizeof(menuInt[1][i]), f);
	skip_pascal_string(f);

	/*Menu 2 - Items*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[2]; ++i)
		read_encrypted_pascal_string(menuInt[2][i], sizeof(menuInt[2][i]), f);
	skip_pascal_string(f);

	/*Menu 3 - Options*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[3]; ++i)
		read_encrypted_pascal_string(menuInt[3][i], sizeof(menuInt[3][i]), f);
	skip_pascal_string(f);

	/*InGame Menu*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(inGameText); ++i)
		read_encrypted_pascal_string(inGameText[i], sizeof(inGameText[i]), f);
	skip_pascal_string(f);

	/*Detail Level*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(detailLevel); ++i)
		read_encrypted_pascal_string(detailLevel[i], sizeof(detailLevel[i]), f);
	skip_pascal_string(f);

	/*Game speed text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(gameSpeedText); ++i)
		read_encrypted_pascal_string(gameSpeedText[i], sizeof(gameSpeedText[i]), f);
	skip_pascal_string(f);

	// episode names
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(episode_name); ++i)
		read_encrypted_pascal_string(episode_name[i], sizeof(episode_name[i]), f);
	skip_pascal_string(f);

	// difficulty names
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(difficulty_name); ++i)
		read_encrypted_pascal_string(difficulty_name[i], sizeof(difficulty_name[i]), f);
	skip_pascal_string(f);

	// gameplay mode names
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(gameplay_name); ++i)
		read_encrypted_pascal_string(gameplay_name[i], sizeof(gameplay_name[i]), f);
	skip_pascal_string(f);

	/*Menu 10 - 2Player Main*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[10]; ++i)
		read_encrypted_pascal_string(menuInt[10][i], sizeof(menuInt[10][i]), f);
	skip_pascal_string(f);

	/*Input Devices*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(inputDevices); ++i)
		read_encrypted_pascal_string(inputDevices[i], sizeof(inputDevices[i]), f);
	skip_pascal_string(f);

	/*Network text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(networkText); ++i)
		read_encrypted_pascal_string(networkText[i], sizeof(networkText[i]), f);
	skip_pascal_string(f);

	/*Menu 11 - 2Player Network*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[11]; ++i)
		read_encrypted_pascal_string(menuInt[11][i], sizeof(menuInt[11][i]), f);
	skip_pascal_string(f);

	/*HighScore Difficulty Names*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(difficultyNameB); ++i)
		read_encrypted_pascal_string(difficultyNameB[i], sizeof(difficultyNameB[i]), f);
	skip_pascal_string(f);

	/*Menu 12 - Network Options*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[12]; ++i)
		read_encrypted_pascal_string(menuInt[12][i], sizeof(menuInt[12][i]), f);
	skip_pascal_string(f);

	/*Menu 13 - Joystick*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[13]; ++i)
		read_encrypted_pascal_string(menuInt[13][i], sizeof(menuInt[13][i]), f);
	skip_pascal_string(f);

	/*Joystick Button Assignments*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(joyButtonNames); ++i)
		read_encrypted_pascal_string(joyButtonNames[i], sizeof(joyButtonNames[i]), f);
	skip_pascal_string(f);

	/*SuperShips - For Super Arcade Mode*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(superShips); ++i)
		read_encrypted_pascal_string(superShips[i], sizeof(superShips[i]), f);
	skip_pascal_string(f);

	/*SuperShips - For Super Arcade Mode*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(specialName); ++i)
		read_encrypted_pascal_string(specialName[i], sizeof(specialName[i]), f);
	skip_pascal_string(f);

	/*Secret DESTRUCT game*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(destructHelp); ++i)
		read_encrypted_pascal_string(destructHelp[i], sizeof(destructHelp[i]), f);
	skip_pascal_string(f);

	/*Secret DESTRUCT weapons*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(weaponNames); ++i)
		read_encrypted_pascal_string(weaponNames[i], sizeof(weaponNames[i]), f);
	skip_pascal_string(f);

	/*Secret DESTRUCT modes*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(destructModeName); ++i)
		read_encrypted_pascal_string(destructModeName[i], sizeof(destructModeName[i]), f);
	skip_pascal_string(f);

	/*NEW: Ship Info*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(shipInfo); ++i)
	{
		read_encrypted_pascal_string(shipInfo[i][0], sizeof(shipInfo[i][0]), f);
		read_encrypted_pascal_string(shipInfo[i][1], sizeof(shipInfo[i][1]), f);
	}
	skip_pascal_string(f);

	/*Menu 12 - Network Options*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[14]; ++i)
		read_encrypted_pascal_string(menuInt[14][i], sizeof(menuInt[14][i]), f);

	fclose(f);

	assert(0 == strcmp(helpTxt[0], "~Next Level~  Select the next level to play (usually there is only one selection)."));
	assert(0 == strcmp(helpTxt[1], "~DATA~  Here you read the datacubes you collect during some levels.  Some cubes have certain... hints.  Be sure to read them!  Select DONE when you are finished reading your cubes."));
	assert(0 == strcmp(helpTxt[2], "~Player Input~  Select the input device you want to use by pushing left or right.  Tyrian will not allow both players to use the same input device."));
	assert(0 == strcmp(helpTxt[3], "-nogood-"));
	assert(0 == strcmp(helpTxt[4], "~Upgrade Ship~  You can configure many aspects of your ship.  Upgrade points for your ship are gained by shooting enemies and collecting various items."));
	assert(0 == strcmp(helpTxt[5], "~Purchasing~  Use the up/down arrow keys to select the item you want to view.  When you purchase the item, a bar will move to show the item you have purchased.  You may then exit the menu using done or ESC."));
	assert(0 == strcmp(helpTxt[6], "The amount of money you will have available after purchasing the item selected is shown at the bottom.  Items that are darkened out cost more than you can afford and you will be unable to select them."));
	assert(0 == strcmp(helpTxt[7], "~Ship Type~  Each ship has a different armor rating.  Armor is your last line of protection from enemy attack and is not easily repaired in the middile of combat."));
	assert(0 == strcmp(helpTxt[8], "~Front Gun~/~Rear Gun~  Your ship can carry both a front and rear gun.  You can change your weapons or upgrade them from here."));
	assert(0 == strcmp(helpTxt[9], "To ~upgrade~ or ~downgrade~ your existing gun, use the right and left arrow keys or click on the red arrows below your ship.  The bars in the center show you what power level your weapons are at."));
	assert(0 == strcmp(helpTxt[10], "The red arrows and cost displays indicate when you can upgrade or downgrade your weapons.  If the right arrow is darkened or the cost is displayed in red, you cannot upgrade further."));
	assert(0 == strcmp(helpTxt[11], "-nogood-"));
	assert(0 == strcmp(helpTxt[12], "Be sure to watch the power bar to see how quickly your energy level will drop while firing."));
	assert(0 == strcmp(helpTxt[13], "~Shield~  Your ship is protected by a shield, which you upgrade in the same manner as your ship. Just select the shield you like with the up/down arrow keys, and buy it by pressing the \"enter\" key or the space bar."));
	assert(0 == strcmp(helpTxt[14], "~Generator~  You need power to fly that ship you're in. Your generator is the most important part of your ship. It determines how many shots you can fire at a time as well as how fast your shield recharges."));
	assert(0 == strcmp(helpTxt[15], "Buy a better generator and you'll have a much better edge. Select your option in the same manner as the other items."));
	assert(0 == strcmp(helpTxt[16], "~Sidekicks~  You can have both a left and right sidekick.  These are special weapons that fly alongside your ship."));
	assert(0 == strcmp(helpTxt[17], "They range from just a simple cannon to the awesome \"Atom\" bomb. Some are much more powerful than others.  Many have limited ammo, but they will recharge slowly during a level!"));
	assert(0 == strcmp(helpTxt[18], "-nogood-"));
	assert(0 == strcmp(helpTxt[19], "~Done~  This selection allows you to return to the Main Menu. Click on it when you're ready to fly!"));
	assert(0 == strcmp(helpTxt[20], "~Options Menu~  From this menu you may configure your joystick or keyboard, and load and save your games."));
	assert(0 == strcmp(helpTxt[21], "~Save Game~  The game you have in progress is automatically saved when you finish the level. So if you forget to save your game, just load \"Last Level\" and the last level you played will be loaded."));
	assert(0 == strcmp(helpTxt[22], "To save a game select \"save game\" from the Options menu, find an empty slot, press the Enter key or spacebar, and enter the name of your choice."));
	assert(0 == strcmp(helpTxt[23], "~Load Game~  To load a game, follow the same procedure except all you need to do is highlight the name of the game you wish to load and press Enter or hit the spacebar."));
	assert(0 == strcmp(helpTxt[24], "~Keyboard~  To configure your keyboard select \"Keyboard\" from the Options menu. Then, hit the key you wish to use for the function that is highlighted (example: when \"up\" is highlighted, hit the up arrow on your numeric keypad)."));
	assert(0 == strcmp(helpTxt[25], "~Joystick~  To configure your joystick, simply select \"Joystick\" from the Options menu and follow the on-screen instructions."));
	assert(0 == strcmp(helpTxt[26], "~Volume~  You may also change the volume of the music or the sound in the game by highlighting either \"Music\" or \"Sound\" in the Options menu and using the left and right arrow keys to change the volume."));
	assert(0 == strcmp(helpTxt[27], "~Quit Game~  Select this to quit your game.  You will be returned to the title screen.  Use Alt-X for a quick way to quit the game."));
	assert(0 == strcmp(helpTxt[28], "To exit the game from the title screen simply select \"Quit Game\" again and press Enter or the spacebar."));
	assert(0 == strncmp(helpTxt[29], "Select Ordering Info from Main Menu to find out how to order", 60));
	assert(0 == strcmp(helpTxt[30], "~Power Bar~\xfe Current power available.  Your weapons and shields draw power from here."));
	assert(0 == strcmp(helpTxt[31], "~Weapons~\xfe The two bars show your weapon power for both front and rear weapons."));
	assert(0 == strcmp(helpTxt[32], "~Weapon Mode~\xfe Some rear weapons can have their firing pattern changed with the keyboard command ~change fire~ or a joystick button assigned to ~Change rear weapon~."));
	assert(0 == strcmp(helpTxt[33], "~Sidekicks~\xfe You can have up to two side weapons, usually fired with separate buttons.  The number of shots left are shown under the weapon name."));
	assert(0 == strcmp(helpTxt[34], "~Shield/Armor~\xfe These bars show how much damage you can take.  Shields will recharge with time from your ~power bar~ and a repair ship may appear if your armor is low - shoot it!"));
	assert(0 == strcmp(helpTxt[35], "~Side-Panel~  Top: Player 1    Bottom: Player 2"));
	assert(0 == strcmp(helpTxt[36], "~Weapon Power~\xfe This shows the level of power for each player's main weapon."));
	assert(0 == strcmp(helpTxt[37], "~Linking~\xfe Move both ships together to combine them.  Player 1 disengages by holding down a sidekick firing button and moving up.  Player 2 disengages by moving without firing the main gun."));
	assert(0 == strcmp(helpTxt[38], "~Rotation Cannon~\xfe Player 2 can aim the cannon by holding down the fire button and moving in any direction."));
	assert(0 == strcmp(pName[0], "Tyrian"));
	assert(0 == strcmp(pName[1], "Asteroid 1"));
	assert(0 == strcmp(pName[2], "Asteroid 2"));
	assert(0 == strcmp(pName[3], "Asteroid 3"));
	assert(0 == strcmp(pName[4], "Soh Jin"));
	assert(0 == strcmp(pName[5], "Camanis"));
	assert(0 == strcmp(pName[6], "Gryphon"));
	assert(0 == strcmp(pName[7], "Savara"));
	assert(0 == strcmp(pName[8], "Gyges"));
	assert(0 == strcmp(pName[9], "Deliani"));
	assert(0 == strcmp(pName[10], "Ixmucane"));
	assert(0 == strcmp(pName[11], "Torm"));
	assert(0 == strcmp(pName[12], "Unknown"));
	assert(0 == strcmp(pName[13], "Unknown"));
	assert(0 == strcmp(pName[14], "Bonus"));
	assert(0 == strcmp(pName[15], "Fleet"));
	assert(0 == strcmp(pName[16], "Botany A"));
	assert(0 == strcmp(pName[17], "Botany B"));
	assert(0 == strcmp(pName[18], "AstCity"));
	assert(0 == strcmp(pName[19], "Gauntlet"));
	assert(0 == strcmp(pName[20], "Skip It"));
	assert(0 == strcmp(miscText[0], "Last Level Completed"));
	assert(0 == strcmp(miscText[1], "~Save name:  ~"));
	assert(0 == strcmp(miscText[2], "EMPTY SLOT"));
	assert(0 == strcmp(miscText[3], "Cubes collected:"));
	assert(0 == strcmp(miscText[4], "Press a key"));
	assert(0 == strcmp(miscText[5], "Exit to Game Menu"));
	assert(0 == strcmp(miscText[6], "Press C to recalibrate"));
	assert(0 == strcmp(miscText[7], "INSERT COIN"));
	assert(0 == strcmp(miscText[8], "Joystick recalibrated"));
	assert(0 == strcmp(miscText[9], "OK"));
	assert(0 == strcmp(miscText[10], "CANCEL"));
	assert(0 == strcmp(miscText[11], "Read:"));
	assert(0 == strcmp(miscText[12], "EXIT"));
	assert(0 == strcmp(miscText[13], "DONE"));
	assert(0 == strcmp(miscText[14], "None"));
	assert(0 == strcmp(miscText[15], "No Data is available at this time."));
	assert(0 == strcmp(miscText[16], "Warping to secret level!"));
	assert(0 == strcmp(miscText[17], "SOUND OFF"));
	assert(0 == strcmp(miscText[18], "SOUND ON"));
	assert(0 == strcmp(miscText[19], "NEW GAME"));
	assert(0 == strcmp(miscText[20], "Good luck..."));
	assert(0 == strcmp(miscText[21], "GAME OVER"));
	assert(0 == strcmp(miscText[22], "PAUSED"));
	assert(0 == strcmp(miscText[23], "Enter your beta password:"));
	assert(0 == strcmp(miscText[24], "PART"));
	assert(0 == strcmp(miscText[25], "Page"));
	assert(0 == strcmp(miscText[26], "Completed:"));
	assert(0 == strcmp(miscText[27], "Current Score:"));
	assert(0 == strcmp(miscText[28], "Are you sure you want to exit?"));
	assert(0 == strcmp(miscText[29], "You will quit this level."));
	assert(0 == strcmp(miscText[30], "You will be returned to the main menu."));
	assert(0 == strcmp(miscText[31], "Use arrow keys to select and press Enter."));
	assert(0 == strcmp(miscText[32], "Press the key to use for this function."));
	assert(0 == strcmp(miscText[33], "Exit to Main Menu"));
	assert(0 == strcmp(miscText[34], "Beta Test Version"));
	assert(0 == strcmp(miscText[35], "MUSIC OFF"));
	assert(0 == strcmp(miscText[36], "MUSIC ON"));
	assert(0 == strcmp(miscText[37], "Your total Score:"));
	assert(0 == strcmp(miscText[38], "One Player Saved Games"));
	assert(0 == strcmp(miscText[39], "Two Player Saved Games"));
	assert(0 == strcmp(miscText[40], "Player 1 Score:"));
	assert(0 == strcmp(miscText[41], "Player 2 Score:"));
	assert(0 == strcmp(miscText[42], "Player 1 got"));
	assert(0 == strcmp(miscText[43], "Player 2 got"));
	assert(0 == strcmp(miscText[44], "Front Weapon Power-Up"));
	assert(0 == strcmp(miscText[45], "Rear Weapon Power-Up"));
	assert(0 == strcmp(miscText[46], "One Player"));
	assert(0 == strcmp(miscText[47], "Two Player"));
	assert(0 == strcmp(miscText[48], "Player 1"));
	assert(0 == strcmp(miscText[49], "Player 2"));
	assert(0 == strcmp(miscText[50], "High Scores"));
	assert(0 == strcmp(miscText[51], "Congratulations!"));
	assert(0 == strcmp(miscText[52], "You have earned a high score!"));
	assert(0 == strcmp(miscText[53], "Enter your name:"));
	assert(0 == strcmp(miscText[54], "Levels in Episode  1:17  2:12  3:12  4:20"));
	assert(0 == strcmp(miscText[55], "~Left~ or ~right~ for 1 or 2 player game."));
	assert(0 == strcmp(miscText[56], "~Left~ or ~right~ to select episode."));
	assert(0 == strcmp(miscText[57], "Player 1 has earned a high score!"));
	assert(0 == strcmp(miscText[58], "Player 2 has earned a high score!"));
	assert(0 == strcmp(miscText[59], "Secret Level!"));
	assert(0 == strcmp(miscText[60], "* Game Saved *"));
	assert(0 == strcmp(miscText[61], "Exiting:"));
	assert(0 == strcmp(miscText[62], "Destruction:"));
	assert(0 == strcmp(miscText[63], "You got the"));
	assert(0 == strcmp(miscText[64], "Press F1 for help."));
	assert(0 == strcmp(miscText[65], "F10 toggles left CPU/Human player."));
	assert(0 == strcmp(miscText[66], "TIME REMAINING"));
	assert(0 == strcmp(miscText[67], ">> Bonus Game <<"));
	assert(0 == strcmp(miscTextB[0], "Ep"));
	assert(0 == strcmp(miscTextB[1], "Episode"));
	assert(0 == strcmp(miscTextB[2], "Last Level"));
	assert(0 == strcmp(miscTextB[3], "got"));
	assert(0 == strcmp(miscTextB[4], "SUPER"));
	assert(0 == strcmp(menuInt[6][0], "KEY CONFIG"));
	assert(0 == strcmp(menuInt[6][1], "UP"));
	assert(0 == strcmp(menuInt[6][2], "DOWN"));
	assert(0 == strcmp(menuInt[6][3], "LEFT"));
	assert(0 == strcmp(menuInt[6][4], "RIGHT"));
	assert(0 == strcmp(menuInt[6][5], "FIRE"));
	assert(0 == strcmp(menuInt[6][6], "CHANGE FIRE"));
	assert(0 == strcmp(menuInt[6][7], "LEFT SIDEKICK"));
	assert(0 == strcmp(menuInt[6][8], "RIGHT SIDEKICK"));
	assert(0 == strcmp(menuInt[6][9], "Reset to Defaults"));
	assert(0 == strcmp(menuInt[6][10], "Done"));
	assert(0 == strcmp(menuText[0], "Start New Game"));
	assert(0 == strcmp(menuText[1], "Load Game"));
	assert(0 == strcmp(menuText[2], "High Scores"));
	assert(0 == strcmp(menuText[3], "Instructions"));
	assert(0 == strcmp(menuText[4], "Ordering Info"));
	assert(0 == strcmp(menuText[5], "Demo"));
	assert(0 == strcmp(menuText[6], "Quit"));
	assert(0 == strcmp(outputs[0], "Enemy approaching from behind."));
	assert(0 == strcmp(outputs[1], "Large mass detected ahead!"));
	assert(0 == strcmp(outputs[2], "Intercepting enemy aircraft."));
	assert(0 == strcmp(outputs[3], "Cleared enemy platforms."));
	assert(0 == strcmp(outputs[4], "Approaching enemy platforms..."));
	assert(0 == strcmp(outputs[5], "~WARNING:~ Spikes ahead!!"));
	assert(0 == strcmp(outputs[6], "Afterburners activated!"));
	assert(0 == strcmp(outputs[7], "** Danger! **"));
	assert(0 == strcmp(outputs[8], ">>> Bonus Level <<<"));
	assert(0 == strcmp(topicName[0], "Help Menu"));
	assert(0 == strcmp(topicName[1], "One-Player Game Menu"));
	assert(0 == strcmp(topicName[2], "Two-Player Game Menu"));
	assert(0 == strcmp(topicName[3], "Upgrade Ship"));
	assert(0 == strcmp(topicName[4], "Options"));
	assert(0 == strcmp(topicName[5], "EXIT HELP"));
	assert(0 == strcmp(mainMenuHelp[0], "Communications : Read incoming messages"));
	assert(0 == strcmp(mainMenuHelp[1], "Purchase new weapons and ship components."));
	assert(0 == strcmp(mainMenuHelp[2], "Configuration and Load/Save game"));
	assert(0 == strcmp(mainMenuHelp[3], "Journey onward to another level."));
	assert(0 == strcmp(mainMenuHelp[4], "Quit the game."));
	assert(0 == strcmp(mainMenuHelp[5], "Each ship has a different armor rating."));
	assert(0 == strcmp(mainMenuHelp[6], "Change and upgrade your forward weapon."));
	assert(0 == strcmp(mainMenuHelp[7], "Change and upgrade your rear/side weapon."));
	assert(0 == strcmp(mainMenuHelp[8], "Shields are your first line of defense."));
	assert(0 == strcmp(mainMenuHelp[9], "Generators power your weapons and shields."));
	assert(0 == strcmp(mainMenuHelp[10], "Take a chance : Purchase a special weapon."));
	assert(0 == strcmp(mainMenuHelp[11], "Return to the previous menu."));
	assert(0 == strcmp(mainMenuHelp[12], "Load a saved game.  (Shortcut is ALT-L)"));
	assert(0 == strcmp(mainMenuHelp[13], "Save your game for later.  (Shortcut is ALT-S)"));
	assert(0 == strcmp(mainMenuHelp[14], "Change volume with the left/right arrow keys."));
	assert(0 == strcmp(mainMenuHelp[15], "Configure your Joystick buttons."));
	assert(0 == strcmp(mainMenuHelp[16], "Change the keys used to play the game."));
	assert(0 == strcmp(mainMenuHelp[17], "Select this location to visit next."));
	assert(0 == strcmp(mainMenuHelp[18], "Select the item you want."));
	assert(0 == strcmp(mainMenuHelp[19], "Press ENTER to change this key."));
	assert(0 == strcmp(mainMenuHelp[20], "Press up or down to select the save slot you want."));
	assert(0 == strcmp(mainMenuHelp[21], "Press up or down to select the cube you want to read."));
	assert(0 == strcmp(mainMenuHelp[22], "Press ESC to exit."));
	assert(0 == strcmp(mainMenuHelp[23], "Select a weapon.  Left/right arrow keys to change power level."));
	assert(0 == strcmp(mainMenuHelp[24], "Reset all key selections to the original settings."));
	assert(0 == strcmp(mainMenuHelp[25], "Continue playing the level."));
	assert(0 == strcmp(mainMenuHelp[26], "Quit playing the level."));
	assert(0 == strcmp(mainMenuHelp[27], "Change detail level with the left/right arrow keys."));
	assert(0 == strcmp(mainMenuHelp[28], "Change game speed with the left/right arrow keys."));
	assert(0 == strcmp(mainMenuHelp[29], "Use the arrow keys to select the interface device to use."));
	assert(0 == strcmp(mainMenuHelp[30], "Change the function with the left/right arrow keys."));
	assert(0 == strcmp(mainMenuHelp[31], "Select this if the position mark is not in the center rectangle."));
	assert(0 == strcmp(mainMenuHelp[32], "Leave the joystick in the center position and push a button."));
	assert(0 == strcmp(mainMenuHelp[33], "Access schematics and detailed info on your ship."));
	assert(0 == strcmp(menuInt[1][0], "Game Menu"));
	assert(0 == strcmp(menuInt[1][1], "Data"));
	assert(0 == strcmp(menuInt[1][2], "Ship Specs"));
	assert(0 == strcmp(menuInt[1][3], "Upgrade Ship"));
	assert(0 == strcmp(menuInt[1][4], "Options"));
	assert(0 == strcmp(menuInt[1][5], "Play Next Level"));
	assert(0 == strcmp(menuInt[1][6], "Quit Game"));
	assert(0 == strcmp(menuInt[2][0], "Upgrade Ship"));
	assert(0 == strcmp(menuInt[2][1], "Ship Type"));
	assert(0 == strcmp(menuInt[2][2], "Front Gun"));
	assert(0 == strcmp(menuInt[2][3], "Rear Gun"));
	assert(0 == strcmp(menuInt[2][4], "Shield"));
	assert(0 == strcmp(menuInt[2][5], "Generator"));
	assert(0 == strcmp(menuInt[2][6], "Left Sidekick"));
	assert(0 == strcmp(menuInt[2][7], "Right Sidekick"));
	assert(0 == strcmp(menuInt[2][8], "Done"));
	assert(0 == strcmp(menuInt[3][0], "Options"));
	assert(0 == strcmp(menuInt[3][1], "Load"));
	assert(0 == strcmp(menuInt[3][2], "Save"));
	assert(0 == strcmp(menuInt[3][3], "Music"));
	assert(0 == strcmp(menuInt[3][4], "Sound"));
	assert(0 == strcmp(menuInt[3][5], "Joystick"));
	assert(0 == strcmp(menuInt[3][6], "Keyboard"));
	assert(0 == strcmp(menuInt[3][7], "Done"));
	assert(0 == strcmp(inGameText[0], "Music Volume"));
	assert(0 == strcmp(inGameText[1], "Sound Volume"));
	assert(0 == strcmp(inGameText[2], "Detail Level"));
	assert(0 == strcmp(inGameText[3], "Game Speed"));
	assert(0 == strcmp(inGameText[4], "Return to Game"));
	assert(0 == strcmp(inGameText[5], "Quit Level"));
	assert(0 == strcmp(detailLevel[0], "Low"));
	assert(0 == strcmp(detailLevel[1], "Normal"));
	assert(0 == strcmp(detailLevel[2], "High"));
	assert(0 == strcmp(detailLevel[3], "Pentium"));
	assert(0 == strcmp(detailLevel[4], "Laptop VGA"));
	assert(0 == strcmp(detailLevel[5], "Wild"));
	assert(0 == strcmp(gameSpeedText[0], "Slug Mode"));
	assert(0 == strcmp(gameSpeedText[1], "Slower"));
	assert(0 == strcmp(gameSpeedText[2], "Slow"));
	assert(0 == strcmp(gameSpeedText[3], "Normal"));
	assert(0 == strcmp(gameSpeedText[4], "Turbo"));
	assert(0 == strcmp(episode_name[0], "Select an Episode"));
	assert(0 == strcmp(episode_name[1], "Episode 1: Escape"));
	assert(0 == strcmp(episode_name[2], "Episode 2: Treachery "));
	assert(0 == strcmp(episode_name[3], "Episode 3: Mission: Suicide"));
	assert(0 == strcmp(episode_name[4], "Episode 4: An End to Fate"));
	assert(0 == strcmp(episode_name[5], "Episode ?: Bonus"));
	assert(0 == strcmp(difficulty_name[0], "Difficulty Level"));
	assert(0 == strcmp(difficulty_name[1], "Easy"));
	assert(0 == strcmp(difficulty_name[2], "Normal"));
	assert(0 == strcmp(difficulty_name[3], "Hard"));
	assert(0 == strcmp(difficulty_name[4], "Impossible"));
	assert(0 == strcmp(difficulty_name[5], "Suicide"));
	assert(0 == strcmp(difficulty_name[6], "Lord of Game"));
	assert(0 == strcmp(gameplay_name[0], "Players"));
	assert(0 == strcmp(gameplay_name[1], "1 Player Full Game"));
	assert(0 == strcmp(gameplay_name[2], "1 Player Arcade"));
	assert(0 == strcmp(gameplay_name[3], "2 Player Arcade"));
	assert(0 == strcmp(gameplay_name[4], "Modem/Network Game"));
	assert(0 == strcmp(menuInt[10][0], "Game Menu"));
	assert(0 == strcmp(menuInt[10][1], "Play Next Level"));
	assert(0 == strcmp(menuInt[10][2], "Player 1 Input"));
	assert(0 == strcmp(menuInt[10][3], "Player 2 Input"));
	assert(0 == strcmp(menuInt[10][4], "Options"));
	assert(0 == strcmp(menuInt[10][5], "Quit Game"));
	assert(0 == strcmp(inputDevices[0], "Keyboard"));
	assert(0 == strcmp(inputDevices[1], "Mouse"));
	assert(0 == strcmp(inputDevices[2], "Joystick"));
	assert(0 == strcmp(networkText[0], "has aborted"));
	assert(0 == strcmp(networkText[1], "a practice game."));
	assert(0 == strcmp(networkText[2], "a two-player game."));
	assert(0 == strcmp(networkText[3], "Waiting for player 1."));
	assert(0 == strcmp(menuInt[11][0], "Game Menu"));
	assert(0 == strcmp(menuInt[11][1], "Play Next Level"));
	assert(0 == strcmp(menuInt[11][2], "Options"));
	assert(0 == strcmp(menuInt[11][3], "Quit Game"));
	assert(0 == strcmp(difficultyNameB[0], "Unranked"));
	assert(0 == strcmp(difficultyNameB[1], "Wimp"));
	assert(0 == strcmp(difficultyNameB[2], "Easy"));
	assert(0 == strcmp(difficultyNameB[3], "Normal"));
	assert(0 == strcmp(difficultyNameB[4], "Hard"));
	assert(0 == strcmp(difficultyNameB[5], "Impossible"));
	assert(0 == strcmp(difficultyNameB[6], "Insanity"));
	assert(0 == strcmp(difficultyNameB[7], "Suicide"));
	assert(0 == strcmp(difficultyNameB[8], "Maniacal"));
	assert(0 == strcmp(difficultyNameB[9], "Zinglon"));
	assert(0 == strcmp(difficultyNameB[10], "Nortaneous"));
	assert(0 == strcmp(menuInt[12][0], "Options"));
	assert(0 == strcmp(menuInt[12][1], "Joystick"));
	assert(0 == strcmp(menuInt[12][2], "Keyboard"));
	assert(0 == strcmp(menuInt[12][3], "Music"));
	assert(0 == strcmp(menuInt[12][4], "Sound"));
	assert(0 == strcmp(menuInt[12][5], "Done"));
	assert(0 == strcmp(menuInt[13][0], "Joystick"));
	assert(0 == strcmp(menuInt[13][1], "Button1"));
	assert(0 == strcmp(menuInt[13][2], "Button2"));
	assert(0 == strcmp(menuInt[13][3], "Button3"));
	assert(0 == strcmp(menuInt[13][4], "Button4"));
	assert(0 == strcmp(menuInt[13][5], "Calibrate"));
	assert(0 == strcmp(menuInt[13][6], "Done"));
	assert(0 == strcmp(joyButtonNames[0], "Fire main weapons"));
	assert(0 == strcmp(joyButtonNames[1], "Fire Left Sidekick"));
	assert(0 == strcmp(joyButtonNames[2], "Fire Right Sidekick"));
	assert(0 == strcmp(joyButtonNames[3], "Fire BOTH Sidekicks"));
	assert(0 == strcmp(joyButtonNames[4], "Change rear weapon"));
	assert(0 == strcmp(superShips[0], "Super Arcade Mode"));
	assert(0 == strcmp(superShips[1], "Ninja Star"));
	assert(0 == strcmp(superShips[2], "StormWind - The Elemental"));
	assert(0 == strcmp(superShips[3], "The Experimental PQZ"));
	assert(0 == strcmp(superShips[4], "Captured U-Fighter"));
	assert(0 == strcmp(superShips[5], "FoodShip Nine"));
	assert(0 == strcmp(superShips[6], "TX SilverCloud"));
	assert(0 == strcmp(superShips[7], "Nort-Ship Z"));
	assert(0 == strcmp(superShips[8], "Main Weapon:"));
	assert(0 == strcmp(superShips[9], "Special Weapon:"));
	assert(0 == strcmp(superShips[10], "Stalker 21.126"));
	assert(0 == strcmp(specialName[0], "STEALTH"));
	assert(0 == strcmp(specialName[1], "STORMWIND"));
	assert(0 == strcmp(specialName[2], "TECHNO"));
	assert(0 == strcmp(specialName[3], "ENEMY"));
	assert(0 == strcmp(specialName[4], "WEIRD"));
	assert(0 == strcmp(specialName[5], "UNKNOWN"));
	assert(0 == strcmp(specialName[6], "NORTSHIPZ"));
	assert(0 == strcmp(specialName[7], "DESTRUCT"));
	assert(0 == strcmp(specialName[8], "ENGAGE"));
	assert(0 == strcmp(destructHelp[0], "Left Player"));
	assert(0 == strcmp(destructHelp[1], "(Toggle CPU with F10)"));
	assert(0 == strcmp(destructHelp[2], "SHIFT or X"));
	assert(0 == strcmp(destructHelp[3], "Fire"));
	assert(0 == strcmp(destructHelp[4], "CTRL or space"));
	assert(0 == strcmp(destructHelp[5], "Change weapon"));
	assert(0 == strcmp(destructHelp[6], "ALT"));
	assert(0 == strcmp(destructHelp[7], "Switch vehicle"));
	assert(0 == strcmp(destructHelp[8], "A/Z"));
	assert(0 == strcmp(destructHelp[9], "Change velocity"));
	assert(0 == strcmp(destructHelp[10], "C/V"));
	assert(0 == strcmp(destructHelp[11], "Change angle"));
	assert(0 == strcmp(destructHelp[12], "Right Player"));
	assert(0 == strcmp(destructHelp[13], "(Human - Numeric pad)"));
	assert(0 == strcmp(destructHelp[14], "INSERT or ENTER"));
	assert(0 == strcmp(destructHelp[15], "Fire"));
	assert(0 == strcmp(destructHelp[16], "PGUP and PGDN"));
	assert(0 == strcmp(destructHelp[17], "Change weapon"));
	assert(0 == strcmp(destructHelp[18], "CENTER (5)"));
	assert(0 == strcmp(destructHelp[19], "Change vehicle"));
	assert(0 == strcmp(destructHelp[20], "UP or DOWN"));
	assert(0 == strcmp(destructHelp[21], "Change velocity"));
	assert(0 == strcmp(destructHelp[22], "LEFT or RIGHT"));
	assert(0 == strcmp(destructHelp[23], "Change angle"));
	assert(0 == strcmp(destructHelp[24], "BACKSPACE restarts"));
	assert(0 == strcmp(weaponNames[0], "Tracer"));
	assert(0 == strcmp(weaponNames[1], "Small"));
	assert(0 == strcmp(weaponNames[2], "Large"));
	assert(0 == strcmp(weaponNames[3], "Micro"));
	assert(0 == strcmp(weaponNames[4], "Super"));
	assert(0 == strcmp(weaponNames[5], "Demolisher"));
	assert(0 == strcmp(weaponNames[6], "Small Nuke"));
	assert(0 == strcmp(weaponNames[7], "Large Nuke"));
	assert(0 == strcmp(weaponNames[8], "Small Dirt"));
	assert(0 == strcmp(weaponNames[9], "Large Dirt"));
	assert(0 == strcmp(weaponNames[10], "Repulsor"));
	assert(0 == strcmp(weaponNames[11], "MiniLaser"));
	assert(0 == strcmp(weaponNames[12], "MegaLaser"));
	assert(0 == strcmp(weaponNames[13], "Tracer"));
	assert(0 == strcmp(weaponNames[14], "MegaBlast"));
	assert(0 == strcmp(weaponNames[15], "Mini"));
	assert(0 == strcmp(weaponNames[16], "Bomb"));
	assert(0 == strcmp(destructModeName[0], "5-Card War"));
	assert(0 == strcmp(destructModeName[1], "Traditional"));
	assert(0 == strcmp(destructModeName[2], "Heli Assault"));
	assert(0 == strcmp(destructModeName[3], "Heli Defense"));
	assert(0 == strcmp(destructModeName[4], "Outgunned"));
	assert(0 == strcmp(shipInfo[0][0], "The ~USP Talon~ provides you with ~one~ ~full~ ~inch~ of armor plating and a generous full-sized generator and engine area.  With both forward and rear mounting systems and 2 cargo bays, it's just like a real ~Microsol~ fighter!"));
	assert(0 == strcmp(shipInfo[0][1], "The craft is especially designed for speed and maneuverability; it gives you an unparalleled 3G base thrust addition!  All this for only ~6000~ cr!!"));
	assert(0 == strcmp(shipInfo[1][0], "The ~SuperCarrot FoodShip 9~ is one of the most advanced edible fighters ever produced.  You can't go wrong with a ~FoodShip~!"));
	assert(0 == strcmp(shipInfo[1][1], "~Foodships~ are usually complemented by either the ~Hotdog Mustard-spread Technology~, or the dreaded ~SuperBanana Bombs~."));
	assert(0 == strcmp(shipInfo[2][0], "The ~Gencore Phoenix~ is the main advanced ~Gencore~ ~Solo~ ~Fighter~.  It's an extremely reliable craft that can take a good deal of punishment."));
	assert(0 == strcmp(shipInfo[2][1], "Standard on the ~Gencore~ ~Solo~ ~Fighter~ line is the new particle redefinition technology, which allows you to materialize extra armor on your outer hull.  Pilots are warned that the system is easily triggered and it completely drains your shields."));
	assert(0 == strcmp(shipInfo[3][0], "The ~Gencore Maelstrom~ is a refined ~Phoenix~ with an additional ~2cm~ of main hull.  It also features deluxe plush seating with ~extra-wide~ ~armrests~ and ~synth-velour~ ~lining~."));
	assert(0 == strcmp(shipInfo[3][1], "Standard on the ~Gencore~ ~Solo~ ~Fighter~ line is the new particle redefinition technology, which allows you to materialize extra armor on your outer hull.  Pilots are warned that the system is easily triggered and it completely drains your shields."));
	assert(0 == strcmp(shipInfo[4][0], "~Microsol~.  What more do you need to convince you that their ~Stalker Solo Fighter~ is the ship you want?  Not so fast, though.  You'll pay a hefty premium for this ship or have to fill out a stack of forms two feet high."));
	assert(0 == strcmp(shipInfo[4][1], "With its dense hull alloys and overall excellent defensive capability, you'll sleep safer at night with one of these on your side."));
	assert(0 == strcmp(shipInfo[5][0], "~Microsol~ continues the ~Stalker~ line with the ~Stalker-B~.  This is the deluxe model preferred by Microsol's own Outer Scouts.  It's the ship of choice for those long, solo flights."));
	assert(0 == strcmp(shipInfo[5][1], "The ~Stalker-B~ adds an additional ~2~ ~structural~ ~units~ with its refined alloys, for an unheard-of ~28 SUs~ in a solo craft."));
	assert(0 == strcmp(shipInfo[6][0], "~TOP SECRET:~ The quest continues for the strongest ship available with the most powerful backup defense systems.  This is one of the only ships in existence with 3 defense bays!"));
	assert(0 == strcmp(shipInfo[6][1], "An additional SU has been added to the armor value of this ship, and Microsol has retrofitted a ~Gencore~ ~auto-repair~ ~system~ into each ship, making these the definite superiority fighter!"));
	assert(0 == strcmp(shipInfo[7][0], "~Microsol~.  What more do you need to convince you that their ~Stalker Solo Fighter~ is the ship you want?  Not so fast, though.  You'll pay a hefty premium for this ship or have to fill out a stack of forms two feet high."));
	assert(0 == strcmp(shipInfo[7][1], "With its dense hull alloys and overall excellent defensive capability, you'll sleep safer at night with one of these on your side."));
	assert(0 == strcmp(shipInfo[8][0], "The ~USP Fang~ is a standard ~Talon~ that has been retrofitted with additional armor and a more powerful secondary backup defense system."));
	assert(0 == strcmp(shipInfo[8][1], "As always, the ~USP~ invulnerability system is unique in its field and, if used properly, will let you take out anything else in the universe."));
	assert(0 == strcmp(shipInfo[9][0], "The ~U-Drone~ is a standard mining drone which has been retrofitted with a manual-drive cockpit and twin rapid-fire vulcan cannons.  Space is limited on this craft, so it's limited to a single backup defense unit."));
	assert(0 == strcmp(shipInfo[9][1], "This ship isn't meant for flying around in, but with an infrared scrambler on board, it's a perfect way to sneak in behind enemy lines!"));
	assert(0 == strcmp(shipInfo[10][0], "No info available on Player 1."));
	assert(0 == strcmp(shipInfo[10][1], "Used in Arcade Mode."));
	assert(0 == strcmp(shipInfo[11][0], "~TOP SECRET:~ This ship is on loan from ~Nortaneous'~ personal hanger.  The rent on this ship is very high, but it's incredibly powerful."));
	assert(0 == strcmp(shipInfo[11][1], "The only way to describe what this ship does is to fly it and see for yourself.  Nothing on the screen is safe with its ~special~ ~weapon~ active."));
	assert(0 == strcmp(shipInfo[12][0], "After destroying the tyrranical might of ~Vykromod~ a second time, Trent found himself locked into the future through the Stargate.  But that's another story.  Now, though, he has returned with his modified ~Stalker C~ to reclaim his past."));
	assert(0 == strcmp(shipInfo[12][1], "The ~Modified Stalker~ is equipped with an advanced molecular generator which simulates many conventional weapons.  But, it also has the power to instantly generate any one of a number of side-ships and front attachments.  It's the power of the future."));
	assert(0 == strcmp(menuInt[14][0], "Game Menu"));
	assert(0 == strcmp(menuInt[14][1], "Play Next Level"));
	assert(0 == strcmp(menuInt[14][2], "Ship Specs"));
	assert(0 == strcmp(menuInt[14][3], "Options"));
	assert(0 == strcmp(menuInt[14][4], "Quit Game"));
}

