#pragma once

extern bool GetExeCRC(DWORD &r_dwProcCRC, DWORD &r_dwFileCRC);

extern void BuildProcessCRC();
extern BYTE GetProcessCRCMagicCubePiece();
