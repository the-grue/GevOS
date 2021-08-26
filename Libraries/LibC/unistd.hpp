#ifndef UNISTD_HPP
#define UNISTD_HPP

extern void _exit(int status);
extern void _shutdown();
extern void _reboot();
extern int close(int descriptor);
extern int read(int descriptor, void* buffer, int length);
extern int write(int descriptor, char* buffer, int length);
extern int open(char* file_name);
extern int spawn(char* file_name, char* args);

#endif
