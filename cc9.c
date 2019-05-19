#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

enum {
  ND_NUM = 256,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

char *user_input;
Token tokens[100];
int pos;

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num_node(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *msg) {
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s", msg);
  exit(1);
}

void tokenize() {
  char *p = user_input;

  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error_at(p, "Failed to tokenize");
  }
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}
Node *parse_num() {
  if (tokens[pos].ty == TK_NUM) {
    Node *node = new_num_node(tokens[pos].val);
    pos++;
    return node;
  }
  error_at(tokens[pos].input, "number is expected");
}

Node *expr() {
  Node *node = parse_num();
  if (tokens[pos].ty == '+') {
    pos++;
    node = new_node('+', node, expr());
  } else if (tokens[pos].ty == '-') {
    pos++;
    node = new_node('-', node, expr());
  }
  return node;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);
  if (node->ty == '+') {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  add rax, rdi\n");
    printf("  push rax\n");
    return;
  } else if (node->ty == '-') {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  sub rax, rdi\n");
    printf("  push rax\n");
    return;
  }

  error("unknown node type: %d", node->ty);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  tokenize();

  pos = 0;
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}
