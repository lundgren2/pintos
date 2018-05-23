#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init(void);
void sys_plist_(void);
void sys_seek_(int FD, unsigned pos);
unsigned sys_filesize_(int FD);
unsigned sys_tell_(int FD);
bool verify_fix_length(void *start, int length); // lab19/lab5
bool verify_variable_length(char *start);        // lab19/lab5

#endif /* userprog/syscall.h */
