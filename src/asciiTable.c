/*
 * =======================================================================================
 *
 *      Filename:  asciiTable.c
 *
 *      Description:  Module implementing output of ascii table.
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
#include <strUtil.h>
#include <asciiTable.h>

/* #####   LOCAL VARIABLES   ########################################### */

static FILE* OUTPUT;

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
asciiTable_setOutput(FILE* stream)
{
    OUTPUT = stream;
}

TableContainer*
asciiTable_allocate(int numRows,int numColumns, bstrList* headerLabels)
{
    int i;
    TableContainer* container;
    OUTPUT = stdout;

    container = (TableContainer*) malloc(sizeof(TableContainer));
    container->numRows = numRows;
    container->numColumns = numColumns;
    container->currentRow = 0;
    container->printed = 0;

    if (numColumns != headerLabels->qty)
    {
        ERROR_PRINT(Number of columns %d not equal to number of header labels %d,numColumns,headerLabels->qty);
    }

    container->header = bstrListCreate();
    bstrListAlloc (container->header, numColumns);

    for(i=0; i<numColumns; i++)
    {
        container->header->entry[i] = bstrcpy(headerLabels->entry[i]);
    }

    container->rows = (bstrList**) malloc( numRows * sizeof(bstrList*));

    for(i=0; i<numRows; i++)
    {
        container->rows[i] = bstrListCreate();
        bstrListAlloc (container->rows[i], numColumns);
    }

    return container;
}

void 
asciiTable_free(TableContainer* container)
{
    int i;

    if(container == NULL)
    {
        ERROR_PLAIN_PRINT(Cannot free NULL reference);
    }

    bstrListDestroy(container->header);

    for(i=0; i<container->numRows; i++)
    {
        bstrListDestroy(container->rows[i]);
    }

    free(container->rows);
}

void
asciiTable_insertRow(TableContainer* container, int row, bstrList* fields)
{
    int i;

    if (container->numColumns != fields->qty)
    {
        ERROR_PRINT(Number of colummns %d not equal to number of field labels %d,container->numColumns,fields->qty);
    }

    if (row >= container->numRows)
    {
        ERROR_PRINT(Number of Rows %d smaller than requested row index %d, container->numRows,row);
    }

    for(i=0; i<container->numColumns; i++)
    {
        container->rows[row]->entry[i] = bstrcpy(fields->entry[i]);
        container->rows[row]->qty++;
    }
}

void
asciiTable_appendRow(TableContainer* container, bstrList* fields)
{
    asciiTable_insertRow(container, container->currentRow++, fields);
}

void
asciiTable_setCurrentRow(TableContainer* container, int row)
{
    container->currentRow = row;
}

void
asciiTable_print(TableContainer* container)
{
    int i;
    int j;
    int* boxwidth;

    boxwidth = (int*) malloc(container->numColumns * sizeof(int));

    for (j=0; j<container->numColumns; j++) boxwidth[j] = 0;

    for (j=0; j<container->numColumns; j++)
    {
        boxwidth[j] = MAX(boxwidth[j],blength(container->header->entry[j]));
    }

    /* determine maximum label width in each column */
    for (i=0; i<container->numRows; i++)
    {
        for (j=0; j<container->numColumns; j++)
        {
            //           btrimws(container->rows[i]->entry[j]);
            boxwidth[j] = MAX(boxwidth[j],blength(container->rows[i]->entry[j]));
        }
    }

    if (! container->printed)
    {
        /* Increase boxwidth with two spaces */
        for (j=0; j<container->numColumns; j++) boxwidth[j] +=2;
    }

    /* print header */

    for (j=0; j<container->numColumns; j++)
    {
        fprintf(OUTPUT,"+");
        for (i=0;i<boxwidth[j];i++)
        {
            fprintf(OUTPUT,"-");
        }
    }
    fprintf(OUTPUT,"+\n");

    for (j=0; j<container->numColumns; j++)
    {
        fprintf(OUTPUT,"|");
        bJustifyCenter(container->header->entry[j],boxwidth[j]);
        fprintf(OUTPUT,"%s",bdata(container->header->entry[j]));
    }
    fprintf(OUTPUT,"|\n");

    for (j=0; j<container->numColumns; j++)
    {
        fprintf(OUTPUT,"+");
        for (i=0;i<boxwidth[j];i++)
        {
            fprintf(OUTPUT,"-");
        }
    }
    fprintf(OUTPUT,"+\n");

    for (i=0; i<container->numRows; i++)
    {
        for (j=0; j<container->numColumns; j++)
        {
            fprintf(OUTPUT,"|");
            bJustifyCenter(container->rows[i]->entry[j],boxwidth[j]);
            fprintf(OUTPUT,"%s",bdata(container->rows[i]->entry[j]));
        }
        fprintf(OUTPUT,"|\n");
    }

    for (j=0; j<container->numColumns; j++)
    {
        fprintf(OUTPUT,"+");
        for (i=0;i<boxwidth[j];i++)
        {
            fprintf(OUTPUT,"-");
        }
    }
    fprintf(OUTPUT,"+\n");
    container->printed = 1;

    free(boxwidth);
}




