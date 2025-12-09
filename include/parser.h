#ifndef PARSER_H
#define PARSER_H

char **tokenize(const char *line);
void free_tokens(char **tokens);

#endif
