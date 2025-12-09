#ifndef KV_H
#define KV_H

void kv_init();
void kv_set(const char *key, const char *value);
char *kv_get(const char *key);
void kv_unset(const char *key);
void kv_print_all();
void kv_free();

#endif
