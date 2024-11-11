#include "chess.h"
#include <stdlib.h>
#include <string.h>

Piece *board[BSIZE][BSIZE];
Piece *white_king, *black_king;
Piece *selected_piece;
Piece *enpassant_piece = NULL;
Coord enpassant_pos;
PieceColor turn;

// 체스판 초기화, FEN에 따라 기물 생성 & 배치
void init_board(char *fen) {
  for (int i = 0; i < 8; i += 1)
    for (int j = 0; j < 8; j += 1)
      board[i][j] = NULL;

  char *fens[2];
  char *board_state[BSIZE];
  fens[0] = strtok(fen, " ");
  fens[1] = strtok(NULL, " ");

  board_state[0] = strtok(fens[0], "/");
  for (int i = 1; i < BSIZE; i++) {
    board_state[i] = strtok(NULL, "/");
  }

  if (strcmp(fens[1], "w") == 0) {
    turn = White;
  } else {
    turn = Black;
  }

  for (int i = 0, count; i < BSIZE; i++) {
    count = 0;
    for (int j = 0, space = 0; board_state[i][j] != '\0'; j += 1) {
      space = board_state[i][j] - '0';

      if (0 <= space && space <= 9) {
        count += space;
      } else {
        Coord coord = {count, 7 - i};
        board[count][7 - i] = new_piece_with_kind(board_state[i][j], coord);
        count += 1;
      }
    }
  }
}

// 체스판 출력해서 시각적으로 보여주는 함수
void render_board(Piece *selected_piece) {
  bool ind = true;

  if (selected_piece == NULL) {
    ind = false;
  }

  system("clear"); // 맥 & 리눅스 용; 윈도우 X

  printf("%s Turn\n\n", turn == White ? "White" : "Black");

  for (int i = BSIZE - 1; i >= 0; i--) {
    Piece *piece = board[0][i];
    Coord coord = (Coord){0, i};

    if (!ind) {
      printf("%d ", i + 1);
    } else {
      char next_char = ' ';
      MoveKind next_state = move_kind(selected_piece, coord);

      if (coord.f == selected_piece->pos.f &&
          coord.r == selected_piece->pos.r) {
        next_char = '[';
      } else if (next_state == Capture || next_state == PromotionWithCapture ||
                 next_state == Enpassant) {
        next_char = '(';
      }

      printf("%d%c", i + 1, next_char);
    }

    for (int j = 0; j < BSIZE; j++) {
      piece = board[j][i];
      coord = (Coord){j, i};

      if (!ind) {
        if (piece == NULL) {
          printf("- ");
          continue;
        } else {
          IconData icon = piece_to_icon(piece->color, piece->type);
          printf("%.*s ", icon.size, icon.index);
          continue;
        }
      } else {
        char next_char = ' ';
        MoveKind state = move_kind(selected_piece, coord);
        MoveKind next_state =
            move_kind(selected_piece, (Coord){coord.f + 1, coord.r});

        if (coord.f == selected_piece->pos.f &&
            coord.r == selected_piece->pos.r) {
          next_char = ']';
        } else if (coord.f + 1 == selected_piece->pos.f &&
                   coord.r == selected_piece->pos.r) {
          next_char = '[';
        } else if (state == Capture || state == PromotionWithCapture ||
                   state == Enpassant) {
          next_char = ')';
        } else if (next_state == Capture ||
                   next_state == PromotionWithCapture ||
                   next_state == Enpassant) {
          next_char = '(';
        }

        if (piece == NULL) {
          printf("%c%c", ind && valid_move(selected_piece, coord) ? '*' : '-',
                 next_char);
          continue;
        } else {
          IconData icon = piece_to_icon(piece->color, piece->type);

          printf("%.*s%c", icon.size, icon.index, next_char);
          continue;
        }
      }
    }
    puts(""); // 줄바꿈
  }
  puts("  a b c d e f g h");
}

// 좌표가 보드 내부의 좌표인지 알려줌
bool in_board(char f, char r) {
  return 0 <= f && f < BSIZE && 0 <= r && r < BSIZE;
}
