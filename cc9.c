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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (*p == '=' && *(p + 1) == '=') {
      tokens[i].ty = TK_EQ;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (*p == '!' && *(p + 1) == '=') {
      tokens[i].ty = TK_NE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (*p == '>') {
      if (*(p + 1) == '=') {
        tokens[i].ty = TK_GE;
        tokens[i].input = p;
        i++;
        p += 2;
      } else {
        tokens[i].ty = '>';
        tokens[i].input = p;
        i++;
        p++;
      }
      continue;
    }
    if (*p == '<') {
      if (*(p + 1) == '=') {
        tokens[i].ty = TK_LE;
        tokens[i].input = p;
        i++;
        p += 2;
      } else {
        tokens[i].ty = '<';
        tokens[i].input = p;
        i++;
        p++;
      }
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

int consume(int ty) {
  if (tokens[pos].ty == ty) {
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
      error_at(tokens[pos].input, "')' is expected");
    }
    return node;
  }
  if (tokens[pos].ty == TK_NUM) {
    Node *node = new_num_node(tokens[pos].val);
    pos++;
    return node;
  }
  error_at(tokens[pos].input, "number is expected");
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
