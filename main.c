#include "chess.h"
#include <stdlib.h>

typedef enum input_state_t {
  Ok,
  OutOfBoard,
  CannotMove,
  NoPiece,
  NoTurn,
  InvalidMove,
} InputState;

char *input_state_message(InputState in) {
  switch (in) {
  case OutOfBoard:
    return "보드 밖의 범위, 다시 선택";
  case CannotMove:
    return "움직일 수 없음, 다시 선택";
  case NoPiece:
    return "기물 없음, 다시 선택";
  case NoTurn:
    return "현재 차례가 아닌 기물, 다시 선택";
  case InvalidMove:
    return "유효하지 않은 수, 다시 선택";
  case Ok:
    return "기물 위치 선택";
  }
}

int main() {
  char start[2], target[2];
  Coord start_pos, target_pos;
  char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w";
  InputState input_state = Ok;

  init_board(fen);    // 보드 시작 상태로 초기화
  render_board(NULL); // 선택된 기물 없이 체스판 출력

  while (true) {
    bool end = true; // 유효한 수가 하나도 남지 않으면 게임 종료
    Piece *piece;

    for (int i = 0; i < BSIZE; i++) {
      for (int j = 0; j < BSIZE; j++) {
        piece = board[i][j];

        // 현재 차례의 모든 기물의 유효한 수 생성
        if (piece != NULL && piece->color == turn) {
          make_moves(piece);
          end = !cannot_move(piece) ? false : end;
        }
      }
    }

    if (end) {
      break; // 게임 끝
    }

    printf("\n%s: ", input_state_message(input_state));
    scanf("%s", start);

    start_pos = notation_to_coord(start);

    if (!in_board(start_pos.f, start_pos.r)) {
      input_state = OutOfBoard;
      render_board(NULL);
      continue;
    }

    selected_piece = board[start_pos.f][start_pos.r];

    if (selected_piece == NULL) {
      input_state = NoPiece;
      render_board(NULL);
      continue;
    } else if (selected_piece->color != turn) {
      input_state = NoTurn;
      render_board(NULL);
      continue;
    } else if (cannot_move(selected_piece)) {
      input_state = CannotMove;
      render_board(NULL);
      continue;
    }

    input_state = Ok;
    render_board(selected_piece);

    printf("\n이동 위치 선택: ");
    scanf("%s", target);

    // 선택 취소
    if (target[0] == 'x' || target[0] == 'X') {
      render_board(NULL);
      continue;
    }

    target_pos = notation_to_coord(target);

    if (!in_board(target_pos.f, target_pos.r)) {
      input_state = OutOfBoard;
      render_board(NULL);
      continue;
    } else if (!valid_move(selected_piece, target_pos)) {
      input_state = InvalidMove;
      render_board(NULL);
      continue;
    } else {
      put_piece_to(selected_piece, target_pos);
      render_board(NULL);
    }
  }

  Piece *king = turn == White ? white_king : black_king;

  if (checked_to(king, king->pos)) {
    printf("\nCheckmate! %s's win\n", king->color == White ? "Black" : "White");
  } else {
    printf("\nStalemate! draw\n");
  }

  return 0;
}
