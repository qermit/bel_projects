#ifndef _FTM_AUX_H_
#define _FTM_AUX_H_
extern volatile unsigned int* cpu_ID;
extern volatile unsigned int* atomic;

inline unsigned int  getCpuId();
inline unsigned int  atomic_get();
inline void          atomic_on();
inline void          atomic_off();

extern volatile unsigned int* cores;
extern volatile unsigned int* time_sys;

inline unsigned int  getCores();
inline unsigned long long get_sys_time();

inline void cycSleep(unsigned int cycs);
inline void uSleep(unsigned long long uSecs);
char* sprinthex(char* buffer, unsigned long val, unsigned char digits);
char* mat_sprinthex(char* buffer, unsigned long val);
#endif
