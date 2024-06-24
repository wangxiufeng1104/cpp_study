#ifndef __SYS_UNTIL_H            
#define __SYS_UNTIL_H
#ifdef __cplusplus
extern "C"
{ 
#endif


int becomeDaemon(int flags);
void becomeSingle(const char *process_name);

#ifdef __cplusplus
}
#endif
#endif