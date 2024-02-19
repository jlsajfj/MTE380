#ifndef __COMPASS_H__
#define __COMPASS_H__

void compass_init(void);
void compass_run(void);

void compass_calibrate_start(void);
void compass_calibrate_end(void);

double compass_getHeading(void);

#endif
