/*
 * =======================================================================================
 *
 *      Filename:  likwid_f90_interface.c
 *
 *      Description: F90 interface for marker API
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

#include <stdlib.h>
#include <string.h>

#include <likwid.h>

void
likwid_markerinit_(void)
{
    likwid_markerInit();
}

void
likwid_markerclose_(void)
{
    likwid_markerClose();
}

void
likwid_markerstartregion_(char* regionTag, int* len)
{
    char* tmp = (char*) malloc(((*len)+1) * sizeof(char));
    strncpy(tmp, regionTag, (*len));
    tmp[(*len)] = 0;
    likwid_markerStartRegion( tmp );
	free(tmp);
}

void
likwid_markerstopregion_(char* regionTag, int* len)
{
    char* tmp = (char*) malloc(((*len)+1) * sizeof(char));
    strncpy(tmp, regionTag, (*len));
    tmp[(*len)] = 0;
    likwid_markerStopRegion( tmp );
	free(tmp);
}

