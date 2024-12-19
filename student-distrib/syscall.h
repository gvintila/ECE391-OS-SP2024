//#if !defined(SYSCALL_H)
#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"
#include "x86_desc.h"
#include "idt.h"
#include "pid.h"
#include "rtc.h"
#include "filesystem.h"
#include "paging.h"
#include "lib.h"

#ifndef ASM

extern int32_t syscall_halt (uint8_t status);
extern int32_t syscall_execute (const uint8_t* command);
extern int32_t syscall_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t syscall_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t syscall_open (const uint8_t* filename);
extern int32_t syscall_close (int32_t fd); 
extern int32_t syscall_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t syscall_vidmap (uint8_t** screen_start);
extern int32_t syscall_set_handler (int32_t signum, void* handler_address);
extern int32_t syscall_sigreturn (void);
extern void* syscall_malloc (int32_t size);
extern void syscall_free (void* ptr);

extern int32_t halt (uint8_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd); 
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screen_start);
extern int32_t set_handler (int32_t signum, void* handler_address);
extern int32_t sigreturn (void);


#endif /* SYSCALL_H */

#endif /* ASM */
