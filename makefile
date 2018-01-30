main: main.c shell.c redirections.c pipe.c
		 gcc -o main main.c shell.c redirections.c pipe.c -I.
