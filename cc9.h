#ifndef CC9_INCLUDE_GUARD
#define CC9_INCLUDE_GUARD
enum {
  TK_NUM = 256,
  TK_IDENT,
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
  ND_IDENT,
  ND_EQ,
  ND_NE,
  ND_LE,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char name;
} Node;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

extern char *user_input;
extern Vector *tokens;
extern int pos;
extern Node *code[100];

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void tokenize();
Node *parse_expr();
void parse_program();
void error(char *fmt, ...);
void error_at(char *loc, char *msg);
void gen(Node *node);
#endif
