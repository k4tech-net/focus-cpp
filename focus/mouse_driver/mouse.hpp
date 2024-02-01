typedef int BOOL;

BOOL mouse_open(void);
void mouse_close(void);
void mouse_move(char button, char x, char y, char wheel);
void moveR(int x, int y);
void press(char button);
void release();
void scroll(char wheel);
