#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#define OUTPUT_FILE_NAME "output.log"
#define OUTPUT_FILE_BUFFER_SIZE 512

enum eOutputChannel {
   eOutputChannelUART         = 0,
   eOutputChannelFile         = 1,
   eOutputChannelLast         = 2
};

enum eOutputSubsystem {
   eOutputSubsystemADC        = 0,
   eOutputSubsystemBME280     = 1,
   eOutputSubsystemDS18B20    = 2,
   eOutputSubsystemDS3231     = 3,
   eOutputSubsystemSystem     = 4,
   eOutputSubsystemSwitch     = 5,
   eOutputSubsystemLast       = 6
};

enum eOutputLevel {
   eOutputLevelDebug          = 0,
   eOutputLevelNormal         = 1,
   eOutputLevelImportant      = 2
};

struct Output_Data {
   unsigned char channel_mask;
   unsigned char subsystem_mask[eOutputSubsystemLast];
   char file_data[OUTPUT_FILE_BUFFER_SIZE];
   int file_data_len;
};

void output(char*, enum eOutputSubsystem, enum eOutputLevel);

#endif
