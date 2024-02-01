class mouse
{	
	typedef int BOOL;

	public:
		BOOL mouse_open(void);
		void mouse_close(void);
		void mouse_move(char button, char x, char y, char wheel);
};