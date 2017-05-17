#ifndef readstream_h
#define readstream_h 
#include <cstdint>
#include "buffer.h"
#include "ReadoutProcessor.h"
int readstream(int32_t _fdIn,ReadoutProcessor &ro_processor);
#endif
