#ifndef LOG_H
#define LOG_H

#include "data/string_.h"
#include "time/pprint.h"

struct log_context {
  string_ (*formatter)(string_);
  void (*message_handler)(string_);
};
string_ std_format_fn(string_);
void stdout_handler(string_);

//void push_context(struct log_context);
//void pop_context(struct log_context);
//void with_context(struct log_context, void* (*fn)(void*));

//void (*file_output_handler)(char*);

void log_info_(string_);
void log_info(char*, ...);
void log_err(char*, ...);
void log_err_(string_);

#endif
