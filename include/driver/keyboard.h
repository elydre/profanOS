#ifndef KEYBOARD_H
#define KEYBOARD_H

char kb_scancode_to_char(int scancode, int shift);
int kb_get_scancode();

void keyboard_init();

// define des inputs clavier
#define KB_A_pressed 16
#define KB_B_pressed 48
#define KB_C_pressed 46
#define KB_D_pressed 32
#define KB_E_pressed 18
#define KB_F_pressed 33
#define KB_G_pressed 34
#define KB_H_pressed 35
#define KB_I_pressed 23
#define KB_J_pressed 36
#define KB_K_pressed 37
#define KB_L_pressed 38
#define KB_M_pressed 39
#define KB_N_pressed 49
#define KB_O_pressed 24
#define KB_P_pressed 25
#define KB_Q_pressed 30
#define KB_R_pressed 19
#define KB_S_pressed 31
#define KB_T_pressed 20
#define KB_U_pressed 22
#define KB_V_pressed 47
#define KB_W_pressed 44
#define KB_X_pressed 45
#define KB_Y_pressed 21
#define KB_Z_pressed 17
#define KB_ESC_pressed 1
#define KB_ALT_pressed 56
#define KB_CTRL_pressed 29
#define KB_SHIFT_pressed 42
#define KB_MAJ_pressed 58

#define KB_A_released KB_A_pressed + 128 
#define KB_B_released KB_B_pressed + 128 
#define KB_C_released KB_C_pressed + 128 
#define KB_D_released KB_D_pressed + 128 
#define KB_E_released KB_E_pressed + 128 
#define KB_F_released KB_F_pressed + 128 
#define KB_G_released KB_G_pressed + 128 
#define KB_H_released KB_H_pressed + 128 
#define KB_I_released KB_I_pressed + 128 
#define KB_J_released KB_J_pressed + 128 
#define KB_K_released KB_K_pressed + 128 
#define KB_L_released KB_L_pressed + 128 
#define KB_M_released KB_M_pressed + 128 
#define KB_N_released KB_N_pressed + 128 
#define KB_O_released KB_O_pressed + 128 
#define KB_P_released KB_P_pressed + 128 
#define KB_Q_released KB_Q_pressed + 128 
#define KB_R_released KB_R_pressed + 128 
#define KB_S_released KB_S_pressed + 128 
#define KB_T_released KB_T_pressed + 128 
#define KB_U_released KB_U_pressed + 128 
#define KB_V_released KB_V_pressed + 128 
#define KB_W_released KB_W_pressed + 128 
#define KB_X_released KB_X_pressed + 128 
#define KB_Y_released KB_Y_pressed + 128 
#define KB_Z_released KB_Z_pressed + 128 
#define KB_ESCreleased KB_ESCpressed + 128 
#define KB_ALT_released KB_ALT_pressed + 128 
#define KB_CTRL_released KB_CTRL_pressed + 128 
#define KB_SHIFT_released KB_SHIFT_pressed + 128 
#define KB_MAJ_released KB_MAJ_pressed + 128 

#endif
