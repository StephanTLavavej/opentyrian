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
#ifndef MUSMAST_H
#define MUSMAST_H

#include "opentyr.h"

#define SONG_ASTEROID2     0 // "Asteroid Dance Part 2"
#define SONG_ASTEROID1     1 // "Asteroid Dance Part 1"
#define DEFAULT_SONG_BUY   2 // "Buy/Sell Music"
#define SONG_DELISHOP1     5 // "Deli Shop Quartet"
#define SONG_ENDGAME2      8 // "Ending Number 2"
#define SONG_LEVELEND      9 // "End of Level"
#define SONG_GAMEOVER     10 // "Game Over Solo"
#define SONG_GRYPHONS     11 // "Gryphons of the West"
#define SONG_GRYPHONE     12 // "Somebody pick up the Gryphone"
#define SONG_GYGESHELP    13 // "Gyges, Will You Please Help Me?"
#define SONG_TUNNELING    16 // "Tunneling Trolls"
#define SONG_MAPVIEW      19 // "The Navigator"
#define SONG_AGAINSAVARA  21 // "Come Back again to Savara"
#define SONG_JOURNEY1     22 // "Space Journey 1"
#define SONG_JOURNEY2     23 // "Space Journey 2"
#define SONG_START5       25 // "START5"
#define SONG_PARLANCE     26 // "Parlance"
#define SONG_TORM         27 // "Torm - The Gathering"
#define SONG_TRANSON      28 // "TRANSON"
#define SONG_TITLE        29 // "Tyrian: The Song"
#define SONG_ZANAC3       30 // "ZANAC3"
#define SONG_ZANACS       31 // "ZANACS"
#define SONG_RETURNSAVARA 32 // "Return me to Savara"
#define SONG_HIGHSCORE    33 // "High Score Table"
#define SONG_FIELDMAG     36 // "A Field for Mag"
#define SONG_BEER         40 // "BEER"

#define MUSIC_NUM 41

extern JE_byte songBuy;
extern const char musicTitle[MUSIC_NUM][48];
extern JE_boolean musicFade;

#endif /* MUSMAST_H */

