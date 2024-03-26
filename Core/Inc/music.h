#ifndef __MUSIC_H__
#define __MUSIC_H__

void music_init(void);
void music_run(void);

void music_play(const char *name);
void music_pause(void);
void music_stop(void);

#endif
