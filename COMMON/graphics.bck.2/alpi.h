#include <stdio.h>
#include <Intrinsic.h>
#include <StringDefs.h>
#include <Label.h>
#include <Command.h>
#include <Form.h>
#include <Box.h>
#include <Shell.h>
#include <Scrollbar.h>
#include <strings.h>

#define NMAX_QWRS     100

#define NMAXCHAR 64
#define BUF_SIZE 1024

#define OFF    0
#define ON     1
#define STUB   2
#define NONE 999

#define LOCAL  0
#define REMOTE 1
#define LOCK   2

#define NEVERUSED 2

#define BLANK_STRING " "
#define EMPTY_STRING ""
#define MAGENTA [01;35m
#define NORMAL [0m
