#ifndef __OUTPUT_H__
#define __OUTPUT_H__

enum eOutputSubsystem {
   eOutputSubsystemADC        = 0,
   eOutputSubsystemBME280     = 1,
   eOutputSubsystemDS18B20    = 2,
   eOutputSubsystemSystem     = 3,
   eOutputSubsystemSwitch     = 4,
   eOutputSubsystemLast       = 5
};

enum eOutputLevel {
   eOutputLevelNone           = 0,
   eOutputLevelDebug          = 1,
   eOutputLevelNormal         = 2,
   eOutputLevelImportant      = 4
};

struct Output_Data {
   unsigned char mask[eOutputSubsystemLast];
};

void output(char*, enum eOutputSubsystem, enum eOutputLevel, int);

#endif
