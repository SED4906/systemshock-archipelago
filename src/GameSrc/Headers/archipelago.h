#ifndef ARCHIPELAGO_H
#define ARCHIPELAGO_H

#ifdef __cplusplus
extern "C" {
#endif

void archipelago_begin();
void archipelago_update();
void archipelago_reset();

bool archipelago_intercept_questbit_on(short qnum);
bool archipelago_intercept_questbit_off(short qnum);
short archipelago_intercept_questvar_set(short qnum, short x, const char* func_name);

#ifdef __cplusplus
}
#endif

#endif
