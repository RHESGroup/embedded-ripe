/*
    Copyright 2022 Gabriele Serra (gabrieleserra.ml)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// -----------------------------------------------------------------------------------------------------------------------
//  Target supported boards
//      - Board define must reflect the name of the folder in which BSP is stored
// -----------------------------------------------------------------------------------------------------------------------

#define qemu 1
#define discovery 2
#define nucleo 3

// -----------------------------------------------------------------------------------------------------------------------
//  Include library
// -----------------------------------------------------------------------------------------------------------------------

#ifndef TARGET_BOARD
#error "You must specify a target board."
#endif

#if TARGET_BOARD == qemu
#include "stm32f405xx.h"
#elif TARGET_BOARD == discovery
#include "stm32f407xx.h"
#elif TARGET_BOARD == nucleo
#include "stm32f407xx.h"
#else
#error "Target not supported."
#endif