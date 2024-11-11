#include "chess.h"
#include <stdlib.h>

// 비숍 방향 벡터
const Coord bishop_dir[] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
// 룩 방향 벡터
const Coord rook_dir[] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
// 나이트 방향 벡터
const Coord knight_dir[] = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2},
                            {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
// 퀸 방향 벡터(비숍 + 룩)
const Coord queen_dir[] = {
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {0, 1}, {0, -1}, {1, 0}, {-1, 0},
};

bool equal_coord(Coord r, Coord l) { return r.f == l.f && r.r == l.r; }

bool contain_type(PieceType sub, PieceType sup) {
  return sub == sup || sub == Bishop && sup == Queen ||
         sub == Rook && sup == Queen;
}

// 각 방향으로 공격이 있는지 확인
bool each_line_attacked(Piece *p, Coord dir, Coord target, Coord protect,
                        PieceType type, char limit) {
  int f, r;
  Piece *other;

  for (int i = 1; i < limit + 1; i++) {
    f = protect.f + i * dir.f;
    r = protect.r + i * dir.r;

    if (in_board(f, r)) {
      if (equal_coord(p->pos, (Coord){f, r})) {
        other = NULL;
      } else if (target.f == f && target.r == r) {
        other = p;
      } else {
        other = board[f][r];
      }

      if (other != NULL && other->color != p->color &&
          contain_type(type, other->type)) {
        return true;
      } else if (other != NULL) {
        break;
      }
    } else {
      break;
    }
  }
  return false;
}

// 해당 위치로 이동시 킹이 체크 당하는지 확인
bool checked_to(Piece *p, Coord target) {
  Coord protect = p->type == King     ? target
                  : p->color == White ? white_king->pos
                                      : black_king->pos;
  PieceColor color = p->color;

  // 비숍 방향 공격 확인
  for (int i = 0; i < 4; i++) {
    if (each_line_attacked(p, bishop_dir[i], target, protect, Bishop, BSIZE))
      return true;
  }
  // 룩
  for (int i = 0; i < 4; i++) {
    if (each_line_attacked(p, rook_dir[i], target, protect, Rook, BSIZE))
      return true;
  }
  // 나이트
  for (int i = 0; i < 8; i++) {
    if (each_line_attacked(p, knight_dir[i], target, protect, Knight, 1))
      return true;
  }
  // 킹
  for (int i = 0; i < 8; i++) {
    if (each_line_attacked(p, queen_dir[i], target, protect, King, 1))
      return true;
  }
  // 폰
  for (int i = 1; i > -2; i -= 2) {
    int f = protect.f + i;
    int r = protect.r + (color == White ? 1 : -1);
    Piece *other;

    if (in_board(f, r)) {
      if (equal_coord(p->pos, (Coord){f, r})) {
        other = NULL;
      } else if (target.f == f && target.r == r) {
        other = p;
      } else {
        other = board[f][r];
      }

      if (other != NULL && other->color != color && other->type == Pawn) {
        return true;
      }
    }
  }

  return false;
}

// 각 방향에 맞는 수 생성
void each_line(Piece *p, Coord dir, char limit) {
  Piece *other;
  int f, r;

  for (int i = 1; i < limit + 1; i++) {
    f = p->pos.f + i * dir.f;
    r = p->pos.r + i * dir.r;

    if (in_board(f, r)) {
      other = board[f][r];

      if (other == NULL) {
        if (!checked_to(p, (Coord){f, r})) {
          p->moves[f][r] = Move;
        }
      } else if (other->color == p->color) {
        break;
      } else {
        if (!checked_to(p, (Coord){f, r})) {
          p->moves[f][r] = Capture;
        }
        break;
      }
    } else {
      break;
    }
  }
}

// 수 비우기, 모든 수를 Unmove로 교체
void flush_moves(Piece *p) {
  for (int i = 0; i < BSIZE; i++) {
    for (int j = 0; j < BSIZE; j++) {
      p->moves[i][j] = Unmove;
    }
  }
}

// 수 생성
void make_moves(Piece *p) {
  flush_moves(p);

  Piece *other;
  int f, r;

  switch (p->type) {
  case Pawn:
    // 직선 방향 확인, 첫 수에서 2칸 움직일 수 있음
    for (int i = 0; i < (p->first_move ? 2 : 1); i++) {
      f = p->pos.f;
      r = p->pos.r + (p->color == White ? i + 1 : -i - 1);

      if (in_board(f, r) && board[f][r] == NULL) {
        if ((r == 0 || r == 7) && !checked_to(p, (Coord){f, r})) {
          p->moves[f][r] = Promotion;
        } else if (!checked_to(p, (Coord){f, r})) {
          p->moves[f][r] = Move;
        }
      } else {
        break;
      }
    }

    // 대각선 방향 확인
    for (int i = -1; i < 2; i += 2) {
      f = p->pos.f + i;
      r = p->pos.r + (p->color == White ? 1 : -1);

      if (in_board(f, r) && !checked_to(p, (Coord){f, r})) {
        other = board[f][r];

        if (other != NULL && other->color != p->color) {
          if (r == 0 || r == 7) {
            p->moves[f][r] = PromotionWithCapture;
          } else {
            p->moves[f][r] = Capture;
          }
        } else if (enpassant_piece != NULL &&
                   equal_coord(enpassant_pos, (Coord){f, r}) &&
                   enpassant_piece->color != p->color) {
          p->moves[f][r] = Enpassant;
        }
      }
    }
    break;
  case Knight:
    for (int i = 0; i < 8; i++)
      each_line(p, knight_dir[i], 1);
    break;
  case Bishop:
    for (int i = 0; i < 4; i++)
      each_line(p, bishop_dir[i], BSIZE);
    break;
  case Rook:
    for (int i = 0; i < 4; i++)
      each_line(p, rook_dir[i], BSIZE);
    break;
  case Queen:
    for (int i = 0; i < 8; i++)
      each_line(p, queen_dir[i], BSIZE);
    break;
  case King:
    for (int i = 0; i < 8; i++)
      each_line(p, queen_dir[i], 1);

    // 캐슬링 판정
    if (!checked_to(p, p->pos)) { // 체크 상태 x
      // 룩 찾기
      Piece *kingside_rook = find_rook(p, (Coord){p->pos.f + 2, p->pos.r});
      Piece *queenside_rook = find_rook(p, (Coord){p->pos.f - 2, p->pos.r});

      // 킹 사이드
      if (kingside_rook != NULL && p->first_move && kingside_rook->first_move) {
        bool passable = true;
        for (int i = 0; i < 2; i++) {
          if (board[p->pos.f + i + 1][p->pos.r] != NULL ||
              checked_to(p, (Coord){p->pos.f + i + 1, p->pos.r})) {
            passable = false;
            break;
          }
        }
        if (passable) {
          p->moves[p->pos.f + 2][p->pos.r] = Castle;
        }
      }

      // 퀸 사이드
      if (queenside_rook != NULL && p->first_move &&
          queenside_rook->first_move) {
        bool passable = true;
        for (int i = 0; i < 3; i++) {
          if (board[p->pos.f - i - 1][p->pos.r] != NULL ||
              checked_to(p, (Coord){p->pos.f - i - 1, p->pos.r})) {
            passable = false;
            break;
          }
        }
        if (passable) {
          p->moves[p->pos.f - 2][p->pos.r] = Castle;
        }
      }
    }
    break;
  }
}

bool valid_move(Piece *p, Coord coord) {
  if (in_board(coord.f, coord.r)) {
    return p->moves[coord.f][coord.r] != Unmove;
  }
  return false;
}

MoveKind move_kind(Piece *p, Coord coord) { return p->moves[coord.f][coord.r]; }

// 유효한 수가 하나도 없는지 확인
bool cannot_move(Piece *p) {
  for (int i = 0; i < BSIZE; i++)
    for (int j = 0; j < BSIZE; j++)
      if (p->moves[i][j] != Unmove)
        return false;

  return true;
}

// 기물 파괴, 메모리에서 해제
void break_piece(Piece *p) {
  board[p->pos.f][p->pos.r] = NULL;
  free(p);
}

// 기물을 원하는 위치에 두기
void put_piece_to(Piece *p, Coord target) {
  // 정상적인 수
  if (in_board(target.f, target.r) && valid_move(p, target)) {
    if (move_kind(p, target) == Capture) {
      break_piece(board[target.f][target.r]);
    }

    // 앙파상
    else if (move_kind(p, target) == Enpassant) {
      board[enpassant_piece->pos.f][enpassant_piece->pos.r] = NULL;
      break_piece(enpassant_piece);
      enpassant_piece = NULL;
    }
    // 프로모션 처리
    else if (move_kind(p, target) == Promotion ||
             move_kind(p, target) == PromotionWithCapture) {
      if (move_kind(p, target) == PromotionWithCapture) {
        break_piece(board[target.f][target.r]);
      }

      char piece_kind = '\0';
      getchar();

      while (true) {
        printf("프로모션, N B R Q 중 선택: ");
        scanf("%c", &piece_kind);

        if ('A' <= piece_kind && piece_kind <= 'Z') {
          piece_kind += 'a' - 'A';
        }

        if (piece_kind == 'n' || piece_kind == 'b' || piece_kind == 'r' ||
            piece_kind == 'q') {
          Piece *new_piece = new_piece_with_kind(piece_kind, target);
          new_piece->color = p->color;
          break_piece(p);
          board[target.f][target.r] = new_piece;
          goto ENDING;
        } else {
          continue;
        }
      }
    }
    // 캐슬링 처리
    else if (move_kind(p, target) == Castle) {
      Piece *rook = find_rook(p, target);

      board[rook->pos.f][rook->pos.r] = NULL; // 원래 자리 비우기
      // 새로운 자리 채우기
      board[target.f + (target.f > 3 ? -1 : 1)][target.r] = rook;
      update_piece(rook, (Coord){target.f + (target.f > 3 ? -1 : 1), target.r});
    }

    board[p->pos.f][p->pos.r] = NULL;
    board[target.f][target.r] = p;

  ENDING:
    update_piece(p, target);

    turn = turn == White ? Black : White;
    selected_piece = NULL;
  }
  // 제자리 & 이미 선택
  else if (equal_coord(target, p->pos)) {
  }
  // 유효하지 않은 수
  else {
  }
}

// 기물 정보 업데이트
void update_piece(Piece *p, Coord coord) {
  if (p->type == Pawn && p->first_move &&
      (p->pos.r - coord.r < 0 ? -(p->pos.r - coord.r) : p->pos.r - coord.r) ==
          2) {
    enpassant_piece = p;
    enpassant_pos = (Coord){
        .f = p->pos.f,
        .r = p->pos.r + (p->color == White ? 1 : -1),
    };
  } else {
    enpassant_piece = NULL;
  }

  p->first_move = false;
  p->pos = coord;
}

IconData piece_to_icon(PieceColor color, PieceType type) {
  char icon_size = ((sizeof PIECE_ICON) - 1) / 12;
  int index = color * 6 + type;

  return (IconData){
      .size = icon_size,
      .index = &PIECE_ICON[index * icon_size],
  };
}

// Ex) a4 -> {0, 3}
Coord notation_to_coord(char *note) {
  return (Coord){
      .f = note[0] - 'a',
      .r = note[1] - '1',
  };
}

// 새로운 기물 생성
Piece *new_piece(PieceColor color, PieceType type, Coord pos) {
  Piece *p = (Piece *)malloc(sizeof(Piece));
  p->color = color;
  p->type = type;
  p->pos = pos;
  p->first_move = true;

  if (type == King && color == White) {
    white_king = p;
  } else if (type == King && color == Black) {
    black_king = p;
  }

  return p;
}

// 문자를 통해서 새로운 기물 생성
Piece *new_piece_with_kind(char kind, Coord pos) {
  PieceColor color;

  if ('A' <= kind && kind <= 'Z') {
    color = White;
    kind += 'a' - 'A';
  } else {
    color = Black;
  }

  switch (kind) {
  case 'p':
    return new_piece(color, Pawn, pos);
  case 'b':
    return new_piece(color, Bishop, pos);
  case 'n':
    return new_piece(color, Knight, pos);
  case 'r':
    return new_piece(color, Rook, pos);
  case 'q':
    return new_piece(color, Queen, pos);
  case 'k':
    return new_piece(color, King, pos);
  default:
    return NULL;
  }
}

// target side에 있는 룩 찾기
Piece *find_rook(Piece *p, Coord target) {
  Piece *rook, *other;

  for (int i = 0; i < BSIZE; i++) {
    for (int j = 0; j < BSIZE; j++) {
      other = board[i][j];

      if (other != NULL && other->type == Rook && other->color == p->color &&
          (target.f > 3) == (other->pos.f > 3)) {
        rook = other;
        return rook;
      }
    }
  }
  return NULL;
}
