#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,
  TK_EQ,
  TK_NE,
  TK_LE,
  TK_GE,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

enum {
  ND_NUM = 256,
  ND_EQ,
  ND_NE,
  ND_LE,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

char *user_input;
Vector *tokens;
int pos;

Token *new_token(int ty, char *input) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->input = input;
  return token;
}
Token *new_num_token(int val, char *input) {
  Token *token = malloc(sizeof(Token));
  token->ty = TK_NUM;
  token->val = val;
  token->input = input;
  return token;
}
Token *get_token(int pos) { return (Token *)tokens->data[pos]; }

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

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}
void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

void expect(int line, int expected, int actual) {
  if (expected == actual) {
    return;
  }
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
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

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')') {

      vec_push(tokens, (void *)new_token(*p, p));
      p++;
      continue;
    }

    if (*p == '=' && *(p + 1) == '=') {
      vec_push(tokens, (void *)new_token(TK_EQ, p));
      p += 2;
      continue;
    }
    if (*p == '!' && *(p + 1) == '=') {
      vec_push(tokens, (void *)new_token(TK_NE, p));
      p += 2;
      continue;
    }
    if (*p == '>') {
      if (*(p + 1) == '=') {
        vec_push(tokens, (void *)new_token(TK_GE, p));
        p += 2;
      } else {
        vec_push(tokens, (void *)new_token('>', p));
        p++;
      }
      continue;
    }
    if (*p == '<') {
      if (*(p + 1) == '=') {
        vec_push(tokens, (void *)new_token(TK_LE, p));
        p += 2;
      } else {
        vec_push(tokens, (void *)new_token('<', p));
        p++;
      }
      continue;
    }

    if (isdigit(*p)) {
      char *cur_p = p;
      int val = strtol(p, &p, 10);
      vec_push(tokens, (void *)new_num_token(val, cur_p));
      continue;
    }

    error_at(p, "Failed to tokenize");
  }
  vec_push(tokens, (void *)new_token(TK_EOF, p));
}

int consume(int ty) {
  if (get_token(pos)->ty == ty) {
    pos++;
    return 1;
  }
  return 0;
}

Node *expr();

Node *parse_num() {
  if (consume('(')) {
    Node *node = expr();
    if (!consume(')')) {
      error_at(get_token(pos)->input, "')' is expected");
    }
    return node;
  }
  if (get_token(pos)->ty == TK_NUM) {
    Node *node = new_num_node(get_token(pos)->val);
    pos++;
    return node;
  }
  error_at(get_token(pos)->input, "number is expected");
}

Node *unary() {
  if (consume('+')) {
    return unary();
  }
  if (consume('-')) {
    return new_node('-', new_num_node(0), unary());
  }
  return parse_num();
}

Node *parse_term() {
  Node *node = unary();
  if (consume('*')) {
    node = new_node('*', node, parse_term());
  } else if (consume('/')) {
    node = new_node('/', node, parse_term());
  }
  return node;
}

Node *parse_factor() {
  Node *node = parse_term();
  if (consume('+')) {
    node = new_node('+', node, parse_factor());
  } else if (consume('-')) {
    node = new_node('-', node, parse_factor());
  }
  return node;
}

Node *parse_cmp() {
  Node *node = parse_factor();
  if (consume('>')) {
    node = new_node('<', parse_cmp(), node);
  } else if (consume('<')) {
    node = new_node('<', node, parse_cmp());
  } else if (consume(TK_LE)) {
    node = new_node(ND_LE, node, parse_cmp());
  } else if (consume(TK_GE)) {
    node = new_node(ND_LE, parse_cmp(), node);
  } else if (consume(TK_EQ)) {
    node = new_node(TK_EQ, node, parse_cmp());
  } else if (consume(TK_NE)) {
    node = new_node(TK_NE, node, parse_cmp());
  }

  return node;
}

Node *expr() { return parse_cmp(); }

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  imul rdi\n");
    break;
  case '/':
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case '<':
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}

void runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++) {
    vec_push(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (long)vec->data[0]);
  expect(__LINE__, 50, (long)vec->data[50]);
  expect(__LINE__, 99, (long)vec->data[99]);
  printf("OK\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }
  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  user_input = argv[1];
  tokens = new_vector();
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
