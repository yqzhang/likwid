/*
 * =======================================================================================
 *
 *      Filename:  tree.c
 *
 *      Description:  Module implementing a tree data structure
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

#include <error.h>
#include <tree.h>

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
tree_init(TreeNode** root, int id)
{
    *root = (TreeNode*) malloc(sizeof(TreeNode));
    (*root)->id = id;
    (*root)->llink = NULL;
    (*root)->rlink = NULL;
}

void
tree_print(TreeNode* nodePtr)
{
    if (nodePtr != NULL) 
    {
        printf("%d ", nodePtr->id);
        TreeNode* walker;    
        walker = nodePtr->llink;

        while (walker != NULL)
        {
            tree_print(walker);
            walker = walker->rlink;
        }
    }
}

void
tree_insertNode(TreeNode* nodePtr, int id)
{
    TreeNode* currentNode;
    TreeNode* tmpNode;

    if (nodePtr == NULL)
    {
        ERROR_PLAIN_PRINT(Node invalid);
    }

    if (nodePtr->llink == NULL)
    {
        nodePtr->llink = (TreeNode*) malloc(sizeof(TreeNode));
        nodePtr->llink->id = id;
        nodePtr->llink->llink = NULL;
        nodePtr->llink->rlink = NULL;
    }
    else
    {
        currentNode = nodePtr->llink;

        while (currentNode->rlink != NULL)
        {
            if (id < currentNode->rlink->id)
            {
                tmpNode = currentNode->rlink;
                currentNode->rlink = (TreeNode*) malloc(sizeof(TreeNode));
                currentNode->rlink->id = id;
                currentNode->rlink->llink = NULL;
                currentNode->rlink->rlink = tmpNode;
                return;
            }
            currentNode = currentNode->rlink;
        }


        if (id > currentNode->id)
        {
            currentNode->rlink = (TreeNode*) malloc(sizeof(TreeNode));
            currentNode->rlink->id = id;
            currentNode->rlink->llink = NULL;
            currentNode->rlink->rlink = NULL;
        }
        else
        {
            tmpNode = currentNode;
            nodePtr->llink = (TreeNode*) malloc(sizeof(TreeNode));
            nodePtr->llink->id = id;
            nodePtr->llink->llink = NULL;
            nodePtr->llink->rlink = tmpNode;
        }
    }
}

int
tree_nodeExists(TreeNode* nodePtr, int id)
{
    TreeNode* walker;

    if (nodePtr == NULL)
    {
        ERROR_PLAIN_PRINT(Node invalid);
    }

    walker = nodePtr->llink;

    while (walker != NULL)
    {
        if (walker->id == id)
        {
            return 1;
        }
        else
        {
            walker = walker->rlink;
        }
    }

    return 0;
}

int
tree_countChildren(TreeNode* nodePtr)
{
    TreeNode* walker;
    int count=0;

    if (nodePtr == NULL)
    {
        ERROR_PLAIN_PRINT(Node invalid);
    }
    if (nodePtr->llink == NULL)
    {
        return 0;
    }

    walker = nodePtr->llink;

    while (walker != NULL)
    {
        count++;
        walker = walker->rlink;
    }

    return count;
}

TreeNode*
tree_getNode(TreeNode* nodePtr, int id)
{
    TreeNode* walker;

    if (nodePtr == NULL)
    {
        ERROR_PLAIN_PRINT(Node invalid);
    }
    if (nodePtr->llink == NULL)
    {
        return NULL;
    }

    walker = nodePtr->llink;

    while (walker != NULL)
    {
        if (walker->id == id)
        {
            return walker;
        }
        else
        {
            walker = walker->rlink;
        }
    }

    return NULL;
}

TreeNode*
tree_getChildNode(TreeNode* nodePtr)
{
    if (nodePtr == NULL)
    {
        ERROR_PLAIN_PRINT(Node invalid);
    }
    if (nodePtr->llink == NULL)
    {
        return NULL;
    }

    return nodePtr->llink;
}

TreeNode*
tree_getNextNode(TreeNode* nodePtr)
{
    if (nodePtr == NULL)
    {
        ERROR_PLAIN_PRINT(Node invalid);
    }

    if (nodePtr->rlink == NULL)
    {
        return NULL;
    }

    return nodePtr->rlink;
}



