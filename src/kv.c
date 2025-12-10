#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kv.h"

typedef struct KV {
    char *key;
    char *value;
    struct KV *next;
} KV;

static KV *head = NULL;

static char *dupstr(const char *s) {
    if (!s) return NULL;
    char *p = malloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

void kv_init(void) { head = NULL; }

void kv_set(const char *key, const char *value) {
    if (!key) return;
    KV *cur = head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            free(cur->value);
            cur->value = dupstr(value);
            return;
        }
        cur = cur->next;
    }
    KV *n = malloc(sizeof(KV));
    n->key = dupstr(key);
    n->value = dupstr(value);
    n->next = head;
    head = n;
}

char *kv_get(const char *key) {
    if (!key) return NULL;
    KV *cur = head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) return dupstr(cur->value);
        cur = cur->next;
    }
    return NULL;
}

void kv_unset(const char *key) {
    KV *cur = head, *prev = NULL;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->next = cur->next;
            else head = cur->next;
            free(cur->key);
            free(cur->value);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void kv_print_all(void) {
    KV *cur = head;
    while (cur) {
        printf("%s=%s\n", cur->key, cur->value);
        cur = cur->next;
    }
}

void kv_free(void) {
    while (head) {
        KV *t = head;
        head = head->next;
        free(t->key);
        free(t->value);
        free(t);
    }
}
