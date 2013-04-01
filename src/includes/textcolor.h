/*
 * =======================================================================================
 *
 *      Filename:  textcolor.h
 *
 *      Description:  Header File textcolor Module. 
 *                    Allows toggling of terminal escape sequences for 
 *                    colored text.
 *
 *      Version:   3.0
 *      Released:  29.11.2012
 *
 *      Author:  Jan Treibig (jt), jan.treibig@gmail.com
 *      Project:  likwid
 *
 *      Copyright (c) 2012, Jan Treibig
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are met:
 *
 *      * Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *      ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *      DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *      FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *      DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *      SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *      OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =======================================================================================
 */

#ifndef TEXTCOLOR_H
#define TEXTCOLOR_H

#define RESET     0
#define BRIGHT    1
#define DIM       2
#define UNDERLINE 3
#define BLINK     4
#define REVERSE   7
#define HIDDEN    8

#define BLACK     0
#define RED       1
#define GREEN     2
#define YELLOW    3
#define BLUE      4
#define MAGENTA   5
#define CYAN      6
#define WHITE     7

static void color_on(int attr, int fg);
static void color_reset(void);

static void color_on(int attr, int fg)
{
    char command[13];

    sprintf(command, "%c[%d;%dm", 0x1B, attr, fg + 30);
    printf("%s", command);
}

static void color_reset()
{
    char command[13];

    sprintf(command, "%c[%dm", 0x1B, 0);
    printf("%s", command);

}

#endif /*TEXTCOLOR_H*/
