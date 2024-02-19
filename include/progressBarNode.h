#ifndef PROGRESSBARNODE_H
#define PROGRESSBARNODE_H
#include "progressBar.h"

typedef struct ProgressBarNode
{
    ProgressBar *progressBar;
    struct ProgressBarNode *next;
} ProgressBarNode;

#endif