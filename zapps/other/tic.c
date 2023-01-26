// a tic tac toe game

#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <i_vgui.h>
#include <i_mouse.h>

int **board;
vgui_t init();
void end(vgui_t vgui);
void draw_board(int **board, vgui_t vgui);
int game_over;
int turn;

int main(int argc, char **argv) {
    printf("Tic Tac Toe !\n");
    vgui_t vgui = init();

    while (!game_over) {
        // we need to draw the board
        draw_board(board, vgui);
        vgui_set_pixel(&vgui, mouse_get_x(), mouse_get_y(), 0xFFFFFF);

        // we need to get the mouse input
        if (mouse_get_button(0)) {
            int x = mouse_get_x();
            int y = mouse_get_y();
            // we need to check if the mouse is in a square
            if (x < 303 && y < 303) {
                int i = x / 101;
                int j = y / 101;
                if (board[i][j] == 0) {
                    board[i][j] = turn;
                    turn = (turn == 1) ? 2 : 1;
                    board[i][j] = 1;
                }
            }
        }
    }

    end(vgui);
    return 0;
}

void draw_board(int **board, vgui_t vgui) {
    // draw the grid

    // we draw the vertical lines
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vgui_draw_line(&vgui, i*101, j*101, i*101+100, j*101, 0x00FF00);
        }
    }
    // we draw the horizontal lines
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vgui_draw_line(&vgui, i*101, j*101, i*101, j*101+100, 0xFF0000);
        }
    }

    // we draw the last lines
    vgui_draw_line(&vgui, 0, 303, 303, 303, 0x00FF00);
    vgui_draw_line(&vgui, 303, 0, 303, 303, 0xFF0000);

    // we draw the squares
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == 0) {
                // draw a blank square (do nothing)
            } else if (board[i][j] == 1) {
                vgui_draw_line(&vgui, i*101, j*101, i*101+100, j*101+100, 0x0000FF);
                vgui_draw_line(&vgui, i*101, j*101+100, i*101+100, j*101, 0x0000FF);
            } else if (board[i][j] == 2) {
                // we cant draw a circle so we draw a small square
                vgui_draw_line(&vgui, i*101+10, j*101+10, i*101+90, j*101+10, 0x0000FF);
                vgui_draw_line(&vgui, i*101+10, j*101+10, i*101+10, j*101+90, 0x0000FF);
                vgui_draw_line(&vgui, i*101+90, j*101+10, i*101+90, j*101+90, 0x0000FF);
                vgui_draw_line(&vgui, i*101+10, j*101+90, i*101+90, j*101+90, 0x0000FF);
            }
        }
    }

    vgui_render(&vgui, 1);
}

vgui_t init() {
    board = malloc(3 * sizeof(int*));
    for (int i = 0; i < 3; i++) {
        board[i] = malloc(3 * sizeof(int));
        for (int j = 0; j < 3; j++) {
            board[i][j] = 0;
        }
    }
    game_over = 0;
    turn = 1;
    vgui_t vgui = vgui_setup(1024, 768);
    return vgui;
}

void end(vgui_t vgui) {
    for (int i = 0; i < 3; i++) {
        free(board[i]);
    }
    free(board);
    vgui_exit(&vgui);
}
