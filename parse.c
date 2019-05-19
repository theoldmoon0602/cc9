#include "cc9.h"
#include <ctype.h>
#include <stdlib.h>

char *user_input;
Vector *tokens;
int pos;
Node *code[100];

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

Node *new_ident_node(char name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

void tokenize() {
  char *p = user_input;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == ';') {

      vec_push(tokens, (void *)new_token(*p, p));
      p++;
      continue;
    }
    if ('a' <= *p && *p <= 'z') {
      vec_push(tokens, (void *)new_token(TK_IDENT, p));
      p++;
      continue;
    }
    if (*p == '=') {
      if (*(p + 1) == '=') {
        vec_push(tokens, (void *)new_token(TK_EQ, p));
        p += 2;
      } else {
        vec_push(tokens, (void *)new_token('=', p));
        p++;
      }
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

Token *get_token(int pos) { return (Token *)tokens->data[pos]; }

int consume(int ty) {
  if (get_token(pos)->ty == ty) {
    pos++;
    return 1;
  }
  return 0;
}

Node *parse_num() {
  if (consume('(')) {
    Node *node = parse_expr();
    if (!consume(')')) {
      error_at(get_token(pos)->input, "')' is expected");
    }
    return node;
  }
  if (get_token(pos)->ty == TK_IDENT) {
    Node *node = new_ident_node(*(get_token(pos)->input));
    pos++;
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

Node *parse_addsub() {
  Node *node = parse_term();
  if (consume('+')) {
    node = new_node('+', node, parse_addsub());
  } else if (consume('-')) {
    node = new_node('-', node, parse_addsub());
  }
  return node;
}

Node *parse_relational() {
  Node *node = parse_addsub();
  if (consume('>')) {
    node = new_node('<', parse_relational(), node);
  } else if (consume('<')) {
    node = new_node('<', node, parse_relational());
  } else if (consume(TK_LE)) {
    node = new_node(ND_LE, node, parse_relational());
  } else if (consume(TK_GE)) {
    node = new_node(ND_LE, parse_relational(), node);
  }
  return node;
}

Node *parse_equality() {
  Node *node = parse_relational();
  if (consume(TK_EQ)) {
    node = new_node(TK_EQ, node, parse_equality());
  } else if (consume(TK_NE)) {
    node = new_node(TK_NE, node, parse_equality());
  }

  return node;
}

Node *parse_assign() {
  Node *node = parse_equality();
  if (consume('=')) {
    node = new_node('=', node, parse_assign());
  }
  return node;
}

Node *parse_expr() { return parse_assign(); }

Node *parse_stmt() {
  Node *node = parse_expr();
  if (!consume(';')) {
    error_at(get_token(pos)->input, "; is expected");
  }
  return node;
}

void parse_program() {
  int i = 0;
  while (get_token(pos)->ty != TK_EOF) {
    code[i++] = parse_stmt();
  }
  code[i] = NULL;
}
