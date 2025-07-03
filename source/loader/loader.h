#ifndef LOADER_H
#define LOADER_H


// 保护模式入口函数，在start.asm中定义
void protect_mode_entry (void);


extern boot_info_t boot_info;

#endif // LOADER_H
