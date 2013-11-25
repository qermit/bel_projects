#ifndef _FTM_AUX_H_
#define _FTM_AUX_H_
extern volatile unsigned int* cpu_ID;
extern volatile unsigned int* cores;
extern volatile unsigned int* atomic;

inline unsigned int  getCpuId();
inline unsigned int  getCores();
inline unsigned int  atomic_get();
inline void          atomic_on();
inline void          atomic_off();

char* sprinthex(char* buffer, unsigned long val, unsigned char digits);
char* mat_sprinthex(char* buffer, unsigned long val);
#endif
