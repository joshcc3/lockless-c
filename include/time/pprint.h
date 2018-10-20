#ifndef TIME_PPRINT_H
#define TIME_PPRINT_H

void time_of_day(struct timeval *);
void useconds_elapsed(struct timeval *, struct timeval *);
unsigned long long usec_since_epoch(struct timeval *);
void pprint_time1(struct timeval *);
unsigned long long now();
#endif
