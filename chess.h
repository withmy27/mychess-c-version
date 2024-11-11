#include <stdbool.h>
#include <stdio.h>
#define PIECE_ICON "♙♘♗♖♕♔♟♞♝♜♛♚"
#define BSIZE 8 // 보드 사이즈

typedef struct coord_t {
  char f; // file 열
  char r; // rank 행
} Coord;

typedef enum piece_color_t {
  White = 0,
  Black = 1,
} PieceColor;

typedef enum piece_type_t {
  Pawn = 0,
  Knight = 1,
  Bishop = 2,
  Rook = 3,
  Queen = 4,
  King = 5,
} PieceType;

typedef enum move_kind_t {
  Move,                 // 갈 수 있는 칸
  Capture,              // 기물을 잡을 수 있는 칸
  Enpassant,            // 앙파상(특수 행마법)
  Promotion,            // 프로모션(특수 행마법)
  PromotionWithCapture, // 프로모션과 동시에 포획
  Castle,               // 캐슬링(특수 행마법)
  Unmove,               // 갈 수 없는 칸
} MoveKind;

typedef struct piece_t {
  PieceColor color;             // 색
  PieceType type;               // 종류
  Coord pos;                    // 위치
  bool first_move;              // 처음 행마인지 여부
  MoveKind moves[BSIZE][BSIZE]; // 가능한 행마 목록
} Piece;

typedef struct icon_data_t {
  char size;   // 유니코드 사이즈, UTF-8에서 3B
  char *index; // 문자열에서 위치
} IconData;

extern Piece *board[BSIZE][BSIZE]; // 기물에 대한 정보를 담고 있음
extern Piece *white_king, *black_king; // 백 킹과 흑 킹
extern Piece *enpassant_piece;         // 앙파상 대상의 폰
extern Coord enpassant_pos;            // 앙파상 대항의 폰의 위치
extern PieceColor turn;                // 현재 게임의 차례
extern Piece *selected_piece;          // 선택된 기물

IconData piece_to_icon(PieceColor color, PieceType type);
void init_board(char *fen);
void render_board(Piece *selected_piece);
bool in_board(char f, char r);
Coord notation_to_coord(char *note);
Piece *new_piece(PieceColor color, PieceType type, Coord pos);
Piece *new_piece_with_kind(char kind, Coord pos);
void break_piece(Piece *p);
bool valid_move(Piece *p, Coord coord);
bool checked_to(Piece *p, Coord target);
void update_piece(Piece *p, Coord coord);
void put_piece_to(Piece *p, Coord target);
Piece *find_rook(Piece *p, Coord target);
MoveKind move_kind(Piece *p, Coord coord);
void make_moves(Piece *p);
bool cannot_move(Piece *p);
