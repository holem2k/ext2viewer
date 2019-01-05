#include "explorer.h"

const DWORD ANSWER_CANCEL                  = 0x01;
const DWORD ANSWER_OVERWRITE               = 0x02;
const DWORD ANSWER_OVERWRITE_ALL           = 0x04;
const DWORD ANSWER_SKIP                    = 0x08;
const DWORD ANSWER_SKIP_ALL                = 0x10;

int CreateOverwriteDlg(explorer *e, char *filename);