/*
 * =======================================================================================
 *
 *      Filename:  asciiBoxes.c
 *
 *      Description:  Module implementing output of nested ascii art boxes
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
/* #####   HEADER FILE INCLUDES   ######################################### */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <error.h>
#include <types.h>
#include <asciiBoxes.h>

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

BoxContainer*
asciiBoxes_allocateContainer(int numLines, int numColumns)
{
    BoxContainer* container;

    container = (BoxContainer*) malloc(sizeof(BoxContainer));
    container->numLines = numLines;
    container->numColumns = numColumns;

    container->boxes = (Box**) malloc(numLines * sizeof(Box*));

    for ( int i=0; i < numLines; i++ )
    {
        container->boxes[i] = (Box*) malloc(numColumns * sizeof(Box));
    }

    for(int i=0; i<numLines; i++)
    {
        for(int j=0; j<numColumns; j++)
        {
            container->boxes[i][j].width = 0;
            container->boxes[i][j].label = NULL;
        }
    }

    return container;
}

void 
asciiBoxes_addBox(BoxContainer* container, int line, int column, bstring label)
{
    if ( line >= container->numLines )
    {
        ERROR_PRINT(line id %d too large,line);
    }
    if ( column >= container->numColumns )
    {
        ERROR_PRINT(column id %d too large,column);
    }

    container->boxes[line][column].width = 1;
    container->boxes[line][column].label = bstrcpy(label);
}


void
asciiBoxes_addJoinedBox(
        BoxContainer* container,
        int line,
        int startColumn,
        int endColumn,
        bstring label)
{
    if ( line >= container->numLines )
    {
        ERROR_PRINT(line id %d too large,line);
    }

    if ( endColumn >= container->numColumns )
    {
        ERROR_PRINT(column id %d too large,endColumn);
    }

    container->boxes[line][startColumn].width = (endColumn-startColumn)+1;
    container->boxes[line][startColumn].label = bstrcpy(label);
}

void
asciiBoxes_print(BoxContainer* container)
{
    int width;
    int boxwidth=0; /* box width is inner width of box */

    /* determine maximum label width */
    for ( int i=0; i < container->numLines; i++ )
    {
        for ( int j=0; j < container->numColumns; j++ )
        {
            btrimws(container->boxes[i][j].label);
            boxwidth = MAX(boxwidth,blength(container->boxes[i][j].label));

            /* if box is joined increase counter */
            if ( container->boxes[i][j].width > 1 )
            {
                j +=  container->boxes[i][j].width;
            }
        }
    }
    boxwidth += 2;  /* add one space each side */

    /* top line */
    printf("+");

    for ( int i=0; i < (container->numColumns * (boxwidth+2) +
                (container->numColumns+1));  /* one space between boxes */
            i++ )
    {
        printf("-");
    }
    printf("+\n");

    for ( int i=0; i < container->numLines; i++ )
    {
        /* Box top line */
        printf("| ");

        for ( int j=0; j < container->numColumns; j++ )
        {
            printf("+");

            if ( container->boxes[i][j].width == 1 )
            {
                for ( int k=0; k < boxwidth; k++ )
                {
                    printf("-");
                }
            }
            else 
            {
                for ( int k=0; k < (container->boxes[i][j].width * boxwidth +
                            (container->boxes[i][j].width-1)*3);
                        k++)
                {
                    printf("-");
                }
                j += container->boxes[i][j].width-1;
            }
            printf("+ ");
        }
        printf("|\n");
        printf("| ");

        /* Box label line */
        for ( int j=0; j < container->numColumns; j++ )
        {
            int offset=0;

            /* center label */
            if ( container->boxes[i][j].width == 1 )
            {
                width = (boxwidth - blength(container->boxes[i][j].label))/2;
                offset = (boxwidth - blength(container->boxes[i][j].label))%2;
            }
            else
            {
                width = (container->boxes[i][j].width * boxwidth +
                        ((container->boxes[i][j].width-1)*3) -
                        blength(container->boxes[i][j].label))/2;

                offset = (container->boxes[i][j].width * boxwidth +
                        ((container->boxes[i][j].width-1)*3) -
                        blength(container->boxes[i][j].label))%2;
            }
            printf("|");

            for ( int k=0; k < (width+offset); k++ )
            {
                printf(" ");
            }

            printf("%s",container->boxes[i][j].label->data);

            for ( int k=0; k < width; k++ )
            {
                printf(" ");
            }
            printf("| ");

            if ( container->boxes[i][j].width != 1 )
            {
                j+= container->boxes[i][j].width-1;
            }
        }
        printf("|\n");
        printf("| ");

        /* Box bottom line */
        for ( int j=0; j < container->numColumns; j++ )
        {
            printf("+");

            if ( container->boxes[i][j].width == 1 )
            {
                for ( int k=0; k < boxwidth; k++ )
                {
                    printf("-");
                }
            }
            else 
            {
                for ( int k=0; k < (container->boxes[i][j].width * boxwidth +
                            (container->boxes[i][j].width-1)*3);
                        k++ )
                {
                    printf("-");
                }
                j+= container->boxes[i][j].width-1;
            }
            printf("+ ");
        }
        printf("|\n");
    }

    /* bottom line */
    printf("+");
    for ( int i=0; i < (container->numColumns * (boxwidth+2) + 
                container->numColumns+1); i++ )
    {
        printf("-");
    }
    printf("+\n");
}

