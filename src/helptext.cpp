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
	/*Online Help*/
	strcpy(helpTxt[0], "~Next Level~  Select the next level to play (usually there is only one selection).");
	strcpy(helpTxt[1], "~DATA~  Here you read the datacubes you collect during some levels.  Some cubes have certain... hints.  Be sure to read them!  Select DONE when you are finished reading your cubes.");
	strcpy(helpTxt[2], "~Player Input~  Select the input device you want to use by pushing left or right.  Tyrian will not allow both players to use the same input device.");
	strcpy(helpTxt[3], "-nogood-");
	strcpy(helpTxt[4], "~Upgrade Ship~  You can configure many aspects of your ship.  Upgrade points for your ship are gained by shooting enemies and collecting various items.");
	strcpy(helpTxt[5], "~Purchasing~  Use the up/down arrow keys to select the item you want to view.  When you purchase the item, a bar will move to show the item you have purchased.  You may then exit the menu using done or ESC.");
	strcpy(helpTxt[6], "The amount of money you will have available after purchasing the item selected is shown at the bottom.  Items that are darkened out cost more than you can afford and you will be unable to select them.");
	strcpy(helpTxt[7], "~Ship Type~  Each ship has a different armor rating.  Armor is your last line of protection from enemy attack and is not easily repaired in the middle of combat.");
	strcpy(helpTxt[8], "~Front Gun~/~Rear Gun~  Your ship can carry both a front and rear gun.  You can change your weapons or upgrade them from here.");
	strcpy(helpTxt[9], "To ~upgrade~ or ~downgrade~ your existing gun, use the right and left arrow keys or click on the red arrows below your ship.  The bars in the center show you what power level your weapons are at.");
	strcpy(helpTxt[10], "The red arrows and cost displays indicate when you can upgrade or downgrade your weapons.  If the right arrow is darkened or the cost is displayed in red, you cannot upgrade further.");
	strcpy(helpTxt[11], "-nogood-");
	strcpy(helpTxt[12], "Be sure to watch the power bar to see how quickly your energy level will drop while firing.");
	strcpy(helpTxt[13], "~Shield~  Your ship is protected by a shield, which you upgrade in the same manner as your ship. Just select the shield you like with the up/down arrow keys, and buy it by pressing the \"enter\" key or the space bar.");
	strcpy(helpTxt[14], "~Generator~  You need power to fly that ship you're in. Your generator is the most important part of your ship. It determines how many shots you can fire at a time as well as how fast your shield recharges.");
	strcpy(helpTxt[15], "Buy a better generator and you'll have a much better edge. Select your option in the same manner as the other items.");
	strcpy(helpTxt[16], "~Sidekicks~  You can have both a left and right sidekick.  These are special weapons that fly alongside your ship.");
	strcpy(helpTxt[17], "They range from just a simple cannon to the awesome \"Atom\" bomb. Some are much more powerful than others.  Many have limited ammo, but they will recharge slowly during a level!");
	strcpy(helpTxt[18], "-nogood-");
	strcpy(helpTxt[19], "~Done~  This selection allows you to return to the Main Menu. Click on it when you're ready to fly!");
	strcpy(helpTxt[20], "~Options Menu~  From this menu you may configure your joystick or keyboard, and load and save your games.");
	strcpy(helpTxt[21], "~Save Game~  The game you have in progress is automatically saved when you finish the level. So if you forget to save your game, just load \"Last Level\" and the last level you played will be loaded.");
	strcpy(helpTxt[22], "To save a game select \"save game\" from the Options menu, find an empty slot, press the Enter key or spacebar, and enter the name of your choice.");
	strcpy(helpTxt[23], "~Load Game~  To load a game, follow the same procedure except all you need to do is highlight the name of the game you wish to load and press Enter or hit the spacebar.");
	strcpy(helpTxt[24], "~Keyboard~  To configure your keyboard select \"Keyboard\" from the Options menu. Then, hit the key you wish to use for the function that is highlighted (example: when \"up\" is highlighted, hit the up arrow on your numeric keypad).");
	strcpy(helpTxt[25], "~Joystick~  To configure your joystick, simply select \"Joystick\" from the Options menu and follow the on-screen instructions.");
	strcpy(helpTxt[26], "~Volume~  You may also change the volume of the music or the sound in the game by highlighting either \"Music\" or \"Sound\" in the Options menu and using the left and right arrow keys to change the volume.");
	strcpy(helpTxt[27], "~Quit Game~  Select this to quit your game.  You will be returned to the title screen.  Use Alt-X for a quick way to quit the game.");
	strcpy(helpTxt[28], "To exit the game from the title screen simply select \"Quit Game\" again and press Enter or the spacebar.");
	strcpy(helpTxt[29], "UNUSED ORDERING INFO MESSAGE.");
	strcpy(helpTxt[30], "~Power Bar~\xfe Current power available.  Your weapons and shields draw power from here.");
	strcpy(helpTxt[31], "~Weapons~\xfe The two bars show your weapon power for both front and rear weapons.");
	strcpy(helpTxt[32], "~Weapon Mode~\xfe Some rear weapons can have their firing pattern changed with the keyboard command ~change fire~ or a joystick button assigned to ~Change rear weapon~.");
	strcpy(helpTxt[33], "~Sidekicks~\xfe You can have up to two side weapons, usually fired with separate buttons.  The number of shots left are shown under the weapon name.");
	strcpy(helpTxt[34], "~Shield/Armor~\xfe These bars show how much damage you can take.  Shields will recharge with time from your ~power bar~ and a repair ship may appear if your armor is low - shoot it!");
	strcpy(helpTxt[35], "~Side-Panel~  Top: Player 1    Bottom: Player 2");
	strcpy(helpTxt[36], "~Weapon Power~\xfe This shows the level of power for each player's main weapon.");
	strcpy(helpTxt[37], "~Linking~\xfe Move both ships together to combine them.  Player 1 disengages by holding down a sidekick firing button and moving up.  Player 2 disengages by moving without firing the main gun.");
	strcpy(helpTxt[38], "~Rotation Cannon~\xfe Player 2 can aim the cannon by holding down the fire button and moving in any direction.");

	/*Planet names*/
	strcpy(pName[0], "Tyrian");
	strcpy(pName[1], "Asteroid 1");
	strcpy(pName[2], "Asteroid 2");
	strcpy(pName[3], "Asteroid 3");
	strcpy(pName[4], "Soh Jin");
	strcpy(pName[5], "Camanis");
	strcpy(pName[6], "Gryphon");
	strcpy(pName[7], "Savara");
	strcpy(pName[8], "Gyges");
	strcpy(pName[9], "Deliani");
	strcpy(pName[10], "Ixmucane");
	strcpy(pName[11], "Torm");
	strcpy(pName[12], "Unknown");
	strcpy(pName[13], "Unknown");
	strcpy(pName[14], "Bonus");
	strcpy(pName[15], "Fleet");
	strcpy(pName[16], "Botany A");
	strcpy(pName[17], "Botany B");
	strcpy(pName[18], "AstCity");
	strcpy(pName[19], "Gauntlet");
	strcpy(pName[20], "Skip It");

	/*Miscellaneous text*/
	strcpy(miscText[0], "Last Level Completed");
	strcpy(miscText[1], "~Save name:  ~");
	strcpy(miscText[2], "EMPTY SLOT");
	strcpy(miscText[3], "Cubes collected:");
	strcpy(miscText[4], "Press a key");
	strcpy(miscText[5], "Exit to Game Menu");
	strcpy(miscText[6], "Press C to recalibrate");
	strcpy(miscText[7], "INSERT COIN");
	strcpy(miscText[8], "Joystick recalibrated");
	strcpy(miscText[9], "OK");
	strcpy(miscText[10], "CANCEL");
	strcpy(miscText[11], "Read:");
	strcpy(miscText[12], "EXIT");
	strcpy(miscText[13], "DONE");
	strcpy(miscText[14], "None");
	strcpy(miscText[15], "No Data is available at this time.");
	strcpy(miscText[16], "Warping to secret level!");
	strcpy(miscText[17], "SOUND OFF");
	strcpy(miscText[18], "SOUND ON");
	strcpy(miscText[19], "NEW GAME");
	strcpy(miscText[20], "Good luck...");
	strcpy(miscText[21], "GAME OVER");
	strcpy(miscText[22], "PAUSED");
	strcpy(miscText[23], "Enter your beta password:");
	strcpy(miscText[24], "PART");
	strcpy(miscText[25], "Page");
	strcpy(miscText[26], "Completed:");
	strcpy(miscText[27], "Current Score:");
	strcpy(miscText[28], "Are you sure you want to exit?");
	strcpy(miscText[29], "You will quit this level.");
	strcpy(miscText[30], "You will be returned to the main menu.");
	strcpy(miscText[31], "Use arrow keys to select and press Enter.");
	strcpy(miscText[32], "Press the key to use for this function.");
	strcpy(miscText[33], "Exit to Main Menu");
	strcpy(miscText[34], "Beta Test Version");
	strcpy(miscText[35], "MUSIC OFF");
	strcpy(miscText[36], "MUSIC ON");
	strcpy(miscText[37], "Your total Score:");
	strcpy(miscText[38], "One Player Saved Games");
	strcpy(miscText[39], "Two Player Saved Games");
	strcpy(miscText[40], "Player 1 Score:");
	strcpy(miscText[41], "Player 2 Score:");
	strcpy(miscText[42], "Player 1 got");
	strcpy(miscText[43], "Player 2 got");
	strcpy(miscText[44], "Front Weapon Power-Up");
	strcpy(miscText[45], "Rear Weapon Power-Up");
	strcpy(miscText[46], "One Player");
	strcpy(miscText[47], "Two Player");
	strcpy(miscText[48], "Player 1");
	strcpy(miscText[49], "Player 2");
	strcpy(miscText[50], "High Scores");
	strcpy(miscText[51], "Congratulations!");
	strcpy(miscText[52], "You have earned a high score!");
	strcpy(miscText[53], "Enter your name:");
	strcpy(miscText[54], "Levels in Episode  1:17  2:12  3:12  4:20");
	strcpy(miscText[55], "~Left~ or ~right~ for 1 or 2 player game.");
	strcpy(miscText[56], "~Left~ or ~right~ to select episode.");
	strcpy(miscText[57], "Player 1 has earned a high score!");
	strcpy(miscText[58], "Player 2 has earned a high score!");
	strcpy(miscText[59], "Secret Level!");
	strcpy(miscText[60], "* Game Saved *");
	strcpy(miscText[61], "Exiting:");
	strcpy(miscText[62], "Destruction:");
	strcpy(miscText[63], "You got the");
	strcpy(miscText[64], "Press F1 for help.");
	strcpy(miscText[65], "F10 toggles left CPU/Human player.");
	strcpy(miscText[66], "TIME REMAINING");
	strcpy(miscText[67], ">> Bonus Game <<");

	/*Little Miscellaneous text*/
	strcpy(miscTextB[0], "Ep");
	strcpy(miscTextB[1], "Episode");
	strcpy(miscTextB[2], "Last Level");
	strcpy(miscTextB[3], "got");
	strcpy(miscTextB[4], "SUPER");

	/*Key names*/
	strcpy(menuInt[6][0], "KEY CONFIG");
	strcpy(menuInt[6][1], "UP");
	strcpy(menuInt[6][2], "DOWN");
	strcpy(menuInt[6][3], "LEFT");
	strcpy(menuInt[6][4], "RIGHT");
	strcpy(menuInt[6][5], "FIRE");
	strcpy(menuInt[6][6], "CHANGE FIRE");
	strcpy(menuInt[6][7], "LEFT SIDEKICK");
	strcpy(menuInt[6][8], "RIGHT SIDEKICK");
	strcpy(menuInt[6][9], "Reset to Defaults");
	strcpy(menuInt[6][10], "Done");

	/*Main Menu*/
	strcpy(menuText[0], "Start New Game");
	strcpy(menuText[1], "Load Game");
	strcpy(menuText[2], "High Scores");
	strcpy(menuText[3], "Instructions");
	strcpy(menuText[4], "Ordering Info");
	strcpy(menuText[5], "Demo");
	strcpy(menuText[6], "Quit");

	/*Event text*/
	strcpy(outputs[0], "Enemy approaching from behind.");
	strcpy(outputs[1], "Large mass detected ahead!");
	strcpy(outputs[2], "Intercepting enemy aircraft.");
	strcpy(outputs[3], "Cleared enemy platforms.");
	strcpy(outputs[4], "Approaching enemy platforms...");
	strcpy(outputs[5], "~WARNING:~ Spikes ahead!!");
	strcpy(outputs[6], "Afterburners activated!");
	strcpy(outputs[7], "** Danger! **");
	strcpy(outputs[8], ">>> Bonus Level <<<");

	/*Help topics*/
	strcpy(topicName[0], "Help Menu");
	strcpy(topicName[1], "One-Player Game Menu");
	strcpy(topicName[2], "Two-Player Game Menu");
	strcpy(topicName[3], "Upgrade Ship");
	strcpy(topicName[4], "Options");
	strcpy(topicName[5], "EXIT HELP");

	/*Main Menu Help*/
	strcpy(mainMenuHelp[0], "Communications : Read incoming messages");
	strcpy(mainMenuHelp[1], "Purchase new weapons and ship components.");
	strcpy(mainMenuHelp[2], "Configuration and Load/Save game");
	strcpy(mainMenuHelp[3], "Journey onward to another level.");
	strcpy(mainMenuHelp[4], "Quit the game.");
	strcpy(mainMenuHelp[5], "Each ship has a different armor rating.");
	strcpy(mainMenuHelp[6], "Change and upgrade your forward weapon.");
	strcpy(mainMenuHelp[7], "Change and upgrade your rear/side weapon.");
	strcpy(mainMenuHelp[8], "Shields are your first line of defense.");
	strcpy(mainMenuHelp[9], "Generators power your weapons and shields.");
	strcpy(mainMenuHelp[10], "Take a chance : Purchase a special weapon.");
	strcpy(mainMenuHelp[11], "Return to the previous menu.");
	strcpy(mainMenuHelp[12], "Load a saved game.  (Shortcut is ALT-L)");
	strcpy(mainMenuHelp[13], "Save your game for later.  (Shortcut is ALT-S)");
	strcpy(mainMenuHelp[14], "Change volume with the left/right arrow keys.");
	strcpy(mainMenuHelp[15], "Configure your Joystick buttons.");
	strcpy(mainMenuHelp[16], "Change the keys used to play the game.");
	strcpy(mainMenuHelp[17], "Select this location to visit next.");
	strcpy(mainMenuHelp[18], "Select the item you want.");
	strcpy(mainMenuHelp[19], "Press ENTER to change this key.");
	strcpy(mainMenuHelp[20], "Press up or down to select the save slot you want.");
	strcpy(mainMenuHelp[21], "Press up or down to select the cube you want to read.");
	strcpy(mainMenuHelp[22], "Press ESC to exit.");
	strcpy(mainMenuHelp[23], "Select a weapon.  Left/right arrow keys to change power level.");
	strcpy(mainMenuHelp[24], "Reset all key selections to the original settings.");
	strcpy(mainMenuHelp[25], "Continue playing the level.");
	strcpy(mainMenuHelp[26], "Quit playing the level.");
	strcpy(mainMenuHelp[27], "Change detail level with the left/right arrow keys.");
	strcpy(mainMenuHelp[28], "Change game speed with the left/right arrow keys.");
	strcpy(mainMenuHelp[29], "Use the arrow keys to select the interface device to use.");
	strcpy(mainMenuHelp[30], "Change the function with the left/right arrow keys.");
	strcpy(mainMenuHelp[31], "Select this if the position mark is not in the center rectangle.");
	strcpy(mainMenuHelp[32], "Leave the joystick in the center position and push a button.");
	strcpy(mainMenuHelp[33], "Access schematics and detailed info on your ship.");

	/*Menu 1 - Main*/
	strcpy(menuInt[1][0], "Game Menu");
	strcpy(menuInt[1][1], "Data");
	strcpy(menuInt[1][2], "Ship Specs");
	strcpy(menuInt[1][3], "Upgrade Ship");
	strcpy(menuInt[1][4], "Options");
	strcpy(menuInt[1][5], "Play Next Level");
	strcpy(menuInt[1][6], "Quit Game");

	/*Menu 2 - Items*/
	strcpy(menuInt[2][0], "Upgrade Ship");
	strcpy(menuInt[2][1], "Ship Type");
	strcpy(menuInt[2][2], "Front Gun");
	strcpy(menuInt[2][3], "Rear Gun");
	strcpy(menuInt[2][4], "Shield");
	strcpy(menuInt[2][5], "Generator");
	strcpy(menuInt[2][6], "Left Sidekick");
	strcpy(menuInt[2][7], "Right Sidekick");
	strcpy(menuInt[2][8], "Done");

	/*Menu 3 - Options*/
	strcpy(menuInt[3][0], "Options");
	strcpy(menuInt[3][1], "Load");
	strcpy(menuInt[3][2], "Save");
	strcpy(menuInt[3][3], "Music");
	strcpy(menuInt[3][4], "Sound");
	strcpy(menuInt[3][5], "Joystick");
	strcpy(menuInt[3][6], "Keyboard");
	strcpy(menuInt[3][7], "Done");

	/*InGame Menu*/
	strcpy(inGameText[0], "Music Volume");
	strcpy(inGameText[1], "Sound Volume");
	strcpy(inGameText[2], "Detail Level");
	strcpy(inGameText[3], "Game Speed");
	strcpy(inGameText[4], "Return to Game");
	strcpy(inGameText[5], "Quit Level");

	/*Detail Level*/
	strcpy(detailLevel[0], "Low");
	strcpy(detailLevel[1], "Normal");
	strcpy(detailLevel[2], "High");
	strcpy(detailLevel[3], "Pentium");
	strcpy(detailLevel[4], "Laptop VGA");
	strcpy(detailLevel[5], "Wild");

	/*Game speed text*/
	strcpy(gameSpeedText[0], "Slug Mode");
	strcpy(gameSpeedText[1], "Slower");
	strcpy(gameSpeedText[2], "Slow");
	strcpy(gameSpeedText[3], "Normal");
	strcpy(gameSpeedText[4], "Turbo");

	// episode names
	strcpy(episode_name[0], "Select an Episode");
	strcpy(episode_name[1], "Episode 1: Escape");
	strcpy(episode_name[2], "Episode 2: Treachery ");
	strcpy(episode_name[3], "Episode 3: Mission: Suicide");
	strcpy(episode_name[4], "Episode 4: An End to Fate");
	strcpy(episode_name[5], "Episode ?: Bonus");

	// difficulty names
	strcpy(difficulty_name[0], "Difficulty Level");
	strcpy(difficulty_name[1], "Easy");
	strcpy(difficulty_name[2], "Normal");
	strcpy(difficulty_name[3], "Hard");
	strcpy(difficulty_name[4], "Impossible");
	strcpy(difficulty_name[5], "Suicide");
	strcpy(difficulty_name[6], "Lord of Game");

	// gameplay mode names
	strcpy(gameplay_name[0], "Players");
	strcpy(gameplay_name[1], "1 Player Full Game");
	strcpy(gameplay_name[2], "1 Player Arcade");
	strcpy(gameplay_name[3], "2 Player Arcade");
	strcpy(gameplay_name[4], "Modem/Network Game");

	/*Menu 10 - 2Player Main*/
	strcpy(menuInt[10][0], "Game Menu");
	strcpy(menuInt[10][1], "Play Next Level");
	strcpy(menuInt[10][2], "Player 1 Input");
	strcpy(menuInt[10][3], "Player 2 Input");
	strcpy(menuInt[10][4], "Options");
	strcpy(menuInt[10][5], "Quit Game");

	/*Input Devices*/
	strcpy(inputDevices[0], "Keyboard");
	strcpy(inputDevices[1], "Mouse");
	strcpy(inputDevices[2], "Joystick");

	/*Network text*/
	strcpy(networkText[0], "has aborted");
	strcpy(networkText[1], "a practice game.");
	strcpy(networkText[2], "a two-player game.");
	strcpy(networkText[3], "Waiting for player 1.");

	/*Menu 11 - 2Player Network*/
	strcpy(menuInt[11][0], "Game Menu");
	strcpy(menuInt[11][1], "Play Next Level");
	strcpy(menuInt[11][2], "Options");
	strcpy(menuInt[11][3], "Quit Game");

	/*HighScore Difficulty Names*/
	strcpy(difficultyNameB[0], "Unranked");
	strcpy(difficultyNameB[1], "Wimp");
	strcpy(difficultyNameB[2], "Easy");
	strcpy(difficultyNameB[3], "Normal");
	strcpy(difficultyNameB[4], "Hard");
	strcpy(difficultyNameB[5], "Impossible");
	strcpy(difficultyNameB[6], "Insanity");
	strcpy(difficultyNameB[7], "Suicide");
	strcpy(difficultyNameB[8], "Maniacal");
	strcpy(difficultyNameB[9], "Zinglon");
	strcpy(difficultyNameB[10], "Nortaneous");

	/*Menu 12 - Network Options*/
	strcpy(menuInt[12][0], "Options");
	strcpy(menuInt[12][1], "Joystick");
	strcpy(menuInt[12][2], "Keyboard");
	strcpy(menuInt[12][3], "Music");
	strcpy(menuInt[12][4], "Sound");
	strcpy(menuInt[12][5], "Done");

	/*Menu 13 - Joystick*/
	strcpy(menuInt[13][0], "Joystick");
	strcpy(menuInt[13][1], "Button1");
	strcpy(menuInt[13][2], "Button2");
	strcpy(menuInt[13][3], "Button3");
	strcpy(menuInt[13][4], "Button4");
	strcpy(menuInt[13][5], "Calibrate");
	strcpy(menuInt[13][6], "Done");

	/*Joystick Button Assignments*/
	strcpy(joyButtonNames[0], "Fire main weapons");
	strcpy(joyButtonNames[1], "Fire Left Sidekick");
	strcpy(joyButtonNames[2], "Fire Right Sidekick");
	strcpy(joyButtonNames[3], "Fire BOTH Sidekicks");
	strcpy(joyButtonNames[4], "Change rear weapon");

	/*SuperShips - For Super Arcade Mode*/
	strcpy(superShips[0], "Super Arcade Mode");
	strcpy(superShips[1], "Ninja Star");
	strcpy(superShips[2], "StormWind - The Elemental");
	strcpy(superShips[3], "The Experimental PQZ");
	strcpy(superShips[4], "Captured U-Fighter");
	strcpy(superShips[5], "FoodShip Nine");
	strcpy(superShips[6], "TX SilverCloud");
	strcpy(superShips[7], "Nort-Ship Z");
	strcpy(superShips[8], "Main Weapon:");
	strcpy(superShips[9], "Special Weapon:");
	strcpy(superShips[10], "Stalker 21.126");

	/*SuperShips - For Super Arcade Mode*/
	strcpy(specialName[0], "STEALTH");
	strcpy(specialName[1], "STORMWIND");
	strcpy(specialName[2], "TECHNO");
	strcpy(specialName[3], "ENEMY");
	strcpy(specialName[4], "WEIRD");
	strcpy(specialName[5], "UNKNOWN");
	strcpy(specialName[6], "NORTSHIPZ");
	strcpy(specialName[7], "DESTRUCT");
	strcpy(specialName[8], "ENGAGE");

	/*Secret DESTRUCT game*/
	strcpy(destructHelp[0], "Left Player");
	strcpy(destructHelp[1], "(Toggle CPU with F10)");
	strcpy(destructHelp[2], "SHIFT or X");
	strcpy(destructHelp[3], "Fire");
	strcpy(destructHelp[4], "CTRL or space");
	strcpy(destructHelp[5], "Change weapon");
	strcpy(destructHelp[6], "ALT");
	strcpy(destructHelp[7], "Switch vehicle");
	strcpy(destructHelp[8], "A/Z");
	strcpy(destructHelp[9], "Change velocity");
	strcpy(destructHelp[10], "C/V");
	strcpy(destructHelp[11], "Change angle");
	strcpy(destructHelp[12], "Right Player");
	strcpy(destructHelp[13], "(Human - Numeric pad)");
	strcpy(destructHelp[14], "INSERT or ENTER");
	strcpy(destructHelp[15], "Fire");
	strcpy(destructHelp[16], "PGUP and PGDN");
	strcpy(destructHelp[17], "Change weapon");
	strcpy(destructHelp[18], "CENTER (5)");
	strcpy(destructHelp[19], "Change vehicle");
	strcpy(destructHelp[20], "UP or DOWN");
	strcpy(destructHelp[21], "Change velocity");
	strcpy(destructHelp[22], "LEFT or RIGHT");
	strcpy(destructHelp[23], "Change angle");
	strcpy(destructHelp[24], "BACKSPACE restarts");

	/*Secret DESTRUCT weapons*/
	strcpy(weaponNames[0], "Tracer");
	strcpy(weaponNames[1], "Small");
	strcpy(weaponNames[2], "Large");
	strcpy(weaponNames[3], "Micro");
	strcpy(weaponNames[4], "Super");
	strcpy(weaponNames[5], "Demolisher");
	strcpy(weaponNames[6], "Small Nuke");
	strcpy(weaponNames[7], "Large Nuke");
	strcpy(weaponNames[8], "Small Dirt");
	strcpy(weaponNames[9], "Large Dirt");
	strcpy(weaponNames[10], "Repulsor");
	strcpy(weaponNames[11], "MiniLaser");
	strcpy(weaponNames[12], "MegaLaser");
	strcpy(weaponNames[13], "Tracer");
	strcpy(weaponNames[14], "MegaBlast");
	strcpy(weaponNames[15], "Mini");
	strcpy(weaponNames[16], "Bomb");

	/*Secret DESTRUCT modes*/
	strcpy(destructModeName[0], "5-Card War");
	strcpy(destructModeName[1], "Traditional");
	strcpy(destructModeName[2], "Heli Assault");
	strcpy(destructModeName[3], "Heli Defense");
	strcpy(destructModeName[4], "Outgunned");

	/*NEW: Ship Info*/
	strcpy(shipInfo[0][0], "The ~USP Talon~ provides you with ~one~ ~full~ ~inch~ of armor plating and a generous full-sized generator and engine area.  With both forward and rear mounting systems and 2 cargo bays, it's just like a real ~Microsol~ fighter!");
	strcpy(shipInfo[0][1], "The craft is especially designed for speed and maneuverability; it gives you an unparalleled 3G base thrust addition!  All this for only ~6000~ cr!!");
	strcpy(shipInfo[1][0], "The ~SuperCarrot FoodShip 9~ is one of the most advanced edible fighters ever produced.  You can't go wrong with a ~FoodShip~!");
	strcpy(shipInfo[1][1], "~FoodShips~ are usually complemented by either the ~Hotdog Mustard-spread Technology~, or the dreaded ~SuperBanana Bombs~.");
	strcpy(shipInfo[2][0], "The ~Gencore Phoenix~ is the main advanced ~Gencore~ ~Solo~ ~Fighter~.  It's an extremely reliable craft that can take a good deal of punishment.");
	strcpy(shipInfo[2][1], "Standard on the ~Gencore~ ~Solo~ ~Fighter~ line is the new particle redefinition technology, which allows you to materialize extra armor on your outer hull.  Pilots are warned that the system is easily triggered and it completely drains your shields.");
	strcpy(shipInfo[3][0], "The ~Gencore Maelstrom~ is a refined ~Phoenix~ with an additional ~2cm~ of main hull.  It also features deluxe plush seating with ~extra-wide~ ~armrests~ and ~synth-velour~ ~lining~.");
	strcpy(shipInfo[3][1], "Standard on the ~Gencore~ ~Solo~ ~Fighter~ line is the new particle redefinition technology, which allows you to materialize extra armor on your outer hull.  Pilots are warned that the system is easily triggered and it completely drains your shields.");
	strcpy(shipInfo[4][0], "~Microsol~.  What more do you need to convince you that their ~Stalker Solo Fighter~ is the ship you want?  Not so fast, though.  You'll pay a hefty premium for this ship or have to fill out a stack of forms two feet high.");
	strcpy(shipInfo[4][1], "With its dense hull alloys and overall excellent defensive capability, you'll sleep safer at night with one of these on your side.");
	strcpy(shipInfo[5][0], "~Microsol~ continues the ~Stalker~ line with the ~Stalker-B~.  This is the deluxe model preferred by Microsol's own Outer Scouts.  It's the ship of choice for those long, solo flights.");
	strcpy(shipInfo[5][1], "The ~Stalker-B~ adds an additional ~2~ ~structural~ ~units~ with its refined alloys, for an unheard-of ~28 SUs~ in a solo craft.");
	strcpy(shipInfo[6][0], "~TOP SECRET:~ The quest continues for the strongest ship available with the most powerful backup defense systems.  This is one of the only ships in existence with 3 defense bays!");
	strcpy(shipInfo[6][1], "An additional SU has been added to the armor value of this ship, and Microsol has retrofitted a ~Gencore~ ~auto-repair~ ~system~ into each ship, making these the definite superiority fighter!");
	strcpy(shipInfo[7][0], "~Microsol~.  What more do you need to convince you that their ~Stalker Solo Fighter~ is the ship you want?  Not so fast, though.  You'll pay a hefty premium for this ship or have to fill out a stack of forms two feet high.");
	strcpy(shipInfo[7][1], "With its dense hull alloys and overall excellent defensive capability, you'll sleep safer at night with one of these on your side.");
	strcpy(shipInfo[8][0], "The ~USP Fang~ is a standard ~Talon~ that has been retrofitted with additional armor and a more powerful secondary backup defense system.");
	strcpy(shipInfo[8][1], "As always, the ~USP~ invulnerability system is unique in its field and, if used properly, will let you take out anything else in the universe.");
	strcpy(shipInfo[9][0], "The ~U-Drone~ is a standard mining drone which has been retrofitted with a manual-drive cockpit and twin rapid-fire vulcan cannons.  Space is limited on this craft, so it's limited to a single backup defense unit.");
	strcpy(shipInfo[9][1], "This ship isn't meant for flying around in, but with an infrared scrambler on board, it's a perfect way to sneak in behind enemy lines!");
	strcpy(shipInfo[10][0], "No info available on Player 1.");
	strcpy(shipInfo[10][1], "Used in Arcade Mode.");
	strcpy(shipInfo[11][0], "~TOP SECRET:~ This ship is on loan from ~Nortaneous'~ personal hangar.  The rent on this ship is very high, but it's incredibly powerful.");
	strcpy(shipInfo[11][1], "The only way to describe what this ship does is to fly it and see for yourself.  Nothing on the screen is safe with its ~special~ ~weapon~ active.");
	strcpy(shipInfo[12][0], "After destroying the tyrannical might of ~Vykromod~ a second time, Trent found himself locked into the future through the Stargate.  But that's another story.  Now, though, he has returned with his modified ~Stalker C~ to reclaim his past.");
	strcpy(shipInfo[12][1], "The ~Modified Stalker~ is equipped with an advanced molecular generator which simulates many conventional weapons.  But, it also has the power to instantly generate any one of a number of side-ships and front attachments.  It's the power of the future.");

	/*Menu 12 - Network Options*/
	strcpy(menuInt[14][0], "Game Menu");
	strcpy(menuInt[14][1], "Play Next Level");
	strcpy(menuInt[14][2], "Ship Specs");
	strcpy(menuInt[14][3], "Options");
	strcpy(menuInt[14][4], "Quit Game");
}

