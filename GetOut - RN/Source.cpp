#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <cstring>

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define HINT 104
#define vertical_line 179
#define horizontal_line 196
#define ene 233
#define SPECIAL 32
using namespace std;

int k;
int NumberOfEnemies;			//no. of enemies
int n, m, Night;
int a[401][401], b[401][401];
int dx[4] = { 1,0,-1,0 };
int dy[4] = { 0,1,0,-1 };
int NumberOfHints = 3;
int ok2;
int Line[200], coloana[200];		//line, column
char Characters[16000];			//characters
char Player = 224;
clock_t PowerTimer = clock() - 12000;
clock_t type3 = 0;
bool CanBeDestroyed;
bool IsSlow = false;
int SpeedOfEnemies = 6;
char BulletASCIICode = 248;

struct highscore	//structure that is used to save the highscores
{
	char name[3];
	clock_t time;
	bool Exists = false;
}easy[6], medium[6], hard[6];

struct man		//structure that helps for using the position of the Player
{
	int x, y;
}Road[4001];

man Human;

int GreaterValue(int a, int b)   //returns the maximum value between two numbers
{
	if (a > b)
		return a;
	return b;
}

int SmallerValue(int a, int b)      //returns the minimum value between two numbers
{
	if (a < b)
		return a;
	return b;
}

int RNG(int x)       //random number generator
{
	int RandomNumber;
	RandomNumber = rand() % x + 1;
	return RandomNumber;
}

void transform(int a[401][401])		//sub used to write the two dimensional array after every move
{
	int i, j, k;
	k = 0;
	if (Night == 0)	//if it’s not night
	{
		for (i = 1; i <= n; i++)	//nested for loop to go through the array
		{
			for (j = 1; j <= m; j++)
			{
				if (a[i][j] != 0)	//if the space in the array is not free
					Characters[k++] = (char)(a[i][j]);
				else  Characters[k++] = ' ';
			}
			Characters[k++] = '\n';
		}
	}
	else		//if it is night	
	{
		for (i = 1; i <= n; i++)		//nested for loop to go through the array
		{
			for (j = 1; j <= m; j++)
			{
				if (a[i][j] != 0 && i <= Human.x + 4 && i >= Human.x - 4 && j >= Human.y - 4 && j <= Human.y + 4)	//if the spaces are not empty and they are less than 4 squares away from the player positon
					Characters[k++] = (char)(a[i][j]);
				else  Characters[k++] = ' ';
			}
			Characters[k++] = '\n';
		}
	}
}

void ShowConsoleCursor(bool showFlag)		//the showFlag is set as False in the program
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = showFlag;		//the visibility of the cursor gets the value of showFlag, that being False
	SetConsoleCursorInfo(out, &cursorInfo);
}

void gotoxy(int x, int y)      //it moves one unit
{
	COORD coord;
	coord.X = x;
	coord.Y = y + 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void print(int a[401][401], clock_t timer, bool first)    //it outputs the info part of the game
{
	transform(a);
	int i, j;
	gotoxy(0, -1);
	if (timer != -1)
	{
		printf("You have %d hints left!  Time:%d\nUse the arrows to move.   For a hint press H.\n", NumberOfHints, timer / 1000);  //shows how many NumberOfHints are left
		int PowerCooldown;
		if (clock() - PowerTimer > 12000)
			PowerCooldown = 0;
		else PowerCooldown = 12 + ((PowerTimer - clock()) / 1000);
		printf("Press SPACE to use your superpower. You can use your hero power in %d  \n", PowerCooldown);			//shows the cooldown for the sspecial power
		if (first)
			printf("%s", Characters);
	}
	else
	{
		gotoxy(0, n + 3);
		printf("%s", Characters);
	}
	if (timer != -1)
	{
		gotoxy(0, n + 2);
		printf("Press Esc to exit application.\n");
	}
}

int canWalk(int x, int y, int Direction_x, int Direction_y)		//it veryfies if the move is possible (if there is not a wall in the way) //DE FAPT NUMAR CAT MA PO MISCA IN DIRECTIA RESPECTIVA
{
	int Counter = 0;
	int i = x;
	int j = y;
	while (a[i][j] == 0)
	{
		i += Direction_x;
		j += Direction_y;
		Counter++;
	}
	i = x;
	j = y;
	while (a[i][j] == 0)
	{
		i -= Direction_x;
		j -= Direction_y;
		Counter++;
	}
	return Counter - 1;
}


class bullet		//the class used for the bullet
{
public:
	struct punct	//public structure	that gives the position x and y 
	{
		int x, y;
	}Current;
	int direction;		//public int giving the direction in which it moves
	bool stun;		//public bool value that checks if it is stuned
	clock_t time;		//public timer
	bool IsDestroyed;		//public bool that checks if it is Destroyed
	bullet()
	{
		Current.x = Current.y = direction = 0;
		stun = false;
		time = 0;
		IsDestroyed = false;  //everything is set to 0, it's overloaded after
	}
	void build(int x, int y, int direction)
	{
		this->Current.x = x;			//IMI SETEAZA POZITIA IN CARE ESTE GENERAT
		this->Current.y = y;
		a[x][y] = (int)BulletASCIICode;		//PUN IN MATRICE
		gotoxy(y - 1, x + 1);					//MUT CURSORUL LA POZITIA RESPECTIVA (LA FEL CA LA INAMIC)
		if (!Night)
			printf("%c", BulletASCIICode);		//PRINTEZ GLONTUL DACA NU E NOAPTE
		else
		{
			if (this->Current.x <= Human.x + 4 && this->Current.x >= Human.x - 4 && this->Current.y <= Human.y + 4 && this->Current.y >= Human.y - 4)
				printf("%c", BulletASCIICode);			//DACA E NOAPTE SI GLONTUL SE AFLA IN RAZA DE VIZIBILITATE A PLAYER-ULUI ATUNCI IL PRINTEZ
			else printf(" ");							//ALTFEL NU
		}
		this->direction = direction;					//SETEZ DIRECTIA IN CARE SE MISCA GLONTUL
		this->IsDestroyed = false;						//SETEZ DACA A FOST DSITRU SAU NU - PENUTR PUTEREA SPECIALA
	}
	void destroy()								//FUNCTIA DE DISTRUGERE A GLONTULUI
	{
		a[this->Current.x][this->Current.y] = 0;			//IL MARHCEZ CU 9
		gotoxy(this->Current.y - 1, this->Current.x + 1);			//MA DUC LA POZITIA LUI
		printf(" ");			//IL STERG
		IsDestroyed = true;		//SI MARCHEZ CA DISTRUS
	}
	int walk(int count)			//FUNCTIA DE MERS AL FEL CA LA INAMIC - RETURNEAZA PATRATELE CAT MAI POATE SA MEARGA GLONUTUL
	{
		if (!IsDestroyed)
		{
			if (a[this->Current.x][this->Current.y + direction] == Player && CanBeDestroyed)
				ok2 = 1;
			else if (!CanBeDestroyed)
			{
				if (!(a[this->Current.x][this->Current.y + direction] == Player))
					if (a[this->Current.x][this->Current.y + direction])
					{
						return 0;
					}
			}
			else if (a[this->Current.x][this->Current.y + direction])			//DACA IMI GASESTE UN PERETE IMI RETURNEAZA 0
				return 0;
			if (!this->stun)
			{
				if (1)
				{
					if (!Night)
					{
						gotoxy(this->Current.y - 1, this->Current.x + 1);
						printf(" ");
						a[this->Current.x][this->Current.y] = 0;
						this->Current.y += this->direction;
						gotoxy(this->Current.y - 1, this->Current.x + 1);
						a[this->Current.x][this->Current.y] = (int)BulletASCIICode;
						printf("%c", BulletASCIICode);
					}
					if (Night)
					{
						if (this->Current.x <= Human.x + 4 && this->Current.x >= Human.x - 4 && this->Current.y <= Human.y + 4 && this->Current.y >= Human.y - 4)
						{
							gotoxy(this->Current.y - 1, this->Current.x + 1);
							printf(" ");
							a[this->Current.x][this->Current.y] = 0;
							this->Current.y += this->direction;
							gotoxy(this->Current.y - 1, this->Current.x + 1);
							a[this->Current.x][this->Current.y] = (int)BulletASCIICode;
							printf("%c", BulletASCIICode);
						}
						else
						{
							gotoxy(this->Current.y - 1, this->Current.x + 1);
							printf(" ");
							a[this->Current.x][this->Current.y] = 0;
							this->Current.y += this->direction;
							gotoxy(this->Current.y - 1, this->Current.x + 1);
							a[this->Current.x][this->Current.y] = (int)BulletASCIICode;
							printf(" ");
						}
					}
				}
			}
			if (this->stun && !this->time)		//CA LA INAMIC
			{
				this->time = clock();
			}
			if (this->stun&&this->time)
			{
				if (clock() - this->time > 5000)
				{
					this->time = 0;
					this->stun = false;
				}
			}
		}
		return 1;
	}
};

class enemy
{
	struct punct
	{
		int x, y;
	};

private:
	int ok = 0;
	int x, y;
public:
	bullet b;
	clock_t time;
	clock_t shoot_time;
	int count;
	bool stun;
	bool dest;
	punct Current;
	bool in_range;
	punct can_shoot;
	int speed;
	enemy* enemy1()				//O FUNCTIE CARE RETURNEAZA UN POINTER LA O STRUCTURA DE TIPUL ENEMY
	{
		enemy *t = new enemy;	//CREEZ UN NOU INAMIC
		t->time = 0;			//INITIALIZEZ TOATE VARIABILELE
		t->shoot_time = 0;
		t->count = -1;
		t->speed = 0;
		t->in_range = false;
		int x, y, Direction = 0;
		int ok = 0;
		while (!ok)
		{
			ok = 1;
			int count = 0;
			x = RNG(n - 2) + 1;
			y = RNG(m - 2) + 1;
			if (x == 2)
				ok = 0;
			if (y == 2)
				ok = 0;
			if (a[x][y] != 0)
				ok = 0;
			if (a[x + 1][y] != 0 && a[x - 1][y] != 0 && Direction == 1)
				ok = 0;
			if (a[x][y + 1] != 0 && a[x][y - 1] != 0 && Direction == 0)
				ok = 0;
			if (Direction && coloana[y])
				ok = 0;
			if (!Direction && Line[x])
				ok = 0;
			if (canWalk(x, y, Direction, !Direction) < 4)
				ok = 0;
		}
		t->Current.x = x;
		t->Current.y = y;
		if (Direction == 1)
		{
			t->x = 1;			//PASUL PE CARE IL FACE
			coloana[y] = 1;		//MARCHEZ CA PE COLOANA ASTA AM DEJA UN INAMIC
			t->y = 0;
		}
		else if (Direction == 0)
		{
			t->y = 1;
			Line[x] = 1;
			t->x = 0;
		}
		a[x][y] = ene;
		t->stun = false;
		t->dest = false;
		return t;
	}
	void walk()
	{
		ok = 0;
		if (!this->stun)
		{
			this->speed++;
			this->speed %= 2;
			if (this->shoot_time)
			{
				this->count++;
				int a;
				a = this->b.walk(this->count);
				if (!a)
				{
					this->b.destroy();
					this->count = -1;
					this->shoot_time = 0;
					this->in_range = false;
				}
			}
			if (clock() - this->shoot_time > 6000 && this->in_range)		//VERIFICA DACA AU TRECUT MAI MULT DE 6 SECUNDE DE CAND A TRAS ULTIMA OARA SI DACA PLAYER-UL ESTE IN RANGEUL SAU
			{
				this->b.destroy();											//DACA DA, ATUNCI DISTRUGE ULTIMUL GLONT
				this->in_range = false;
				this->count = -1;
				this->shoot_time = 0;
			}
			if (this->speed)
			{
				while (!ok)
				{
					if (a[this->Current.x + x][this->Current.y + y] == 0 || (this->Current.x + x == Human.x && this->Current.y + y == Human.y))			//VERIFICA DACA POT SA MERG IN DIRECTIA RESPECTIVA
					{
						a[this->Current.x][this->Current.y] = 0;				//RESTEZ DIRECTIA ACUTALA ADICA O MARCHEZ CU 0
						gotoxy(this->Current.y - 1, this->Current.x + 1);		//IMI MUTA CURSORUL IN LOCUL IN CARE ERA INAMICUL
						printf(" ");											//STERG INAMICUL DE PE POZITA LUI					
						this->Current.x = this->Current.x + x;					//MODIFIC POZITIA INAMICULUI
						this->Current.y = this->Current.y + y;
						gotoxy(this->Current.y - 1, this->Current.x + 1);		//IMI MUT CURSORUL AL POZITIA LA CARE TREBUIE SA PRINTEZ INAMICUL
						if (Night)
						{
							if (this->Current.x <= Human.x + 4 && this->Current.x >= Human.x - 4 && this->Current.y <= Human.y + 4 && this->Current.y >= Human.y - 4)
								printf("%c", ene);			//DACA E NOAPTE SI SE AFLA IN ARIA DE VIZIBILTATE A JUCATORULUI IL PRINTEZ
							else printf(" ");				//DACA NU, ATUNCI NU
						}
						else printf("%c", ene);					//PRINTEZ INAMICUL
						a[this->Current.x][this->Current.y] = ene;		//MARCHEZ IN MATRICE CA L-AM MUTAT
						this->can_shoot.x = this->Current.x;
						this->can_shoot.y = this->Current.y;
						if (!this->in_range)
						{
							for (int ii = 0; ii < 20; ii++)
							{
								this->can_shoot.x = this->can_shoot.x + x;
								this->can_shoot.y = this->can_shoot.y + y;
								if (a[this->can_shoot.x][this->can_shoot.y] == Player)
								{
									this->in_range = true;
									break;
								}
								else if (a[this->can_shoot.x][this->can_shoot.y] != 0)
									break;
							}
						}
						if (this->in_range && !this->shoot_time)			//DACA E IN RANGE SI NU ARE COOLDOWN LA IMPUSCAT
						{
							this->shoot_time = clock();
							this->b.build(this->Current.x, this->Current.y + y, y);		//IMI CONSTRUIESTE UN GLONT
						}
						ok = 1;
					}
					else
					{
						x = -x;
						y = -y;
					}
				}
			}
		}
		if (this->stun)			//INAMICUL ESTE STUNED SI IMI VERIFICA CAT TIMP MAI ARE PANA II TRECE
		{
			if (this->time == 0)
				this->time = clock();
			else if ((clock() - this->time) > 5000)
			{
				this->stun = false;
				this->time = 0;
			}
		}
	}

	void destroy()
	{
		delete this;
	}
};


enemy* EnemyVariable[35];

void special(int type)		//the function used for the special powers
{
	if (type == 1)		//if your class is Destroyer
	{
		int i, j;
		for (i = 0; i < NumberOfEnemies; i++)	//for each enemy
		{
			if (EnemyVariable[i]->b.Current.x <= Human.x + 1 && EnemyVariable[i]->b.Current.x >= Human.x - 1
				&& EnemyVariable[i]->b.Current.y <= Human.y + 2 && EnemyVariable[i]->b.Current.y >= Human.y - 2)	//if the bullet is a square or less away in the x direction and 2 squares or less in the y direction
			{
				EnemyVariable[i]->b.destroy();
				EnemyVariable[i]->b.IsDestroyed = true;	//the bullet is destroyed
			}
			if (EnemyVariable[i]->Current.x <= Human.x + 1 && EnemyVariable[i]->Current.x >= Human.x - 1
				&& EnemyVariable[i]->Current.y <= Human.y + 2 && EnemyVariable[i]->Current.y >= Human.y - 2)	//if the enemy is a square or less away in the x direction and 2 squares or less in the y direction
			{
				EnemyVariable[i]->dest = true;	//the enemy is destryed
				gotoxy(EnemyVariable[i]->Current.y - 1, EnemyVariable[i]->Current.x + 1);
				a[EnemyVariable[i]->Current.x][EnemyVariable[i]->Current.y] = 0;	//the space occupied by the enemy becomes free
				printf(" ");
				EnemyVariable[i]->Current.y = -10;
				EnemyVariable[i]->Current.x = -10;
			}
		}
	}
	if (type == 2)		//if your class is Chaos
	{
		int i;
		for (i = 0; i < NumberOfEnemies; i++)
		{

			if (EnemyVariable[i]->b.Current.x <= Human.x + 1 && EnemyVariable[i]->b.Current.x >= Human.x - 1
				&& EnemyVariable[i]->b.Current.y <= Human.y + 2 && EnemyVariable[i]->b.Current.y >= Human.y - 2)		//if the bullet is a square or less away in the x direction and 2 squares or less in the y direction
			{
				EnemyVariable[i]->b.stun = true;	//the bullet is stunned 
			}
			if (EnemyVariable[i]->Current.x <= Human.x + 4 && EnemyVariable[i]->Current.x >= Human.x - 4
				&& EnemyVariable[i]->Current.y <= Human.y + 4 && EnemyVariable[i]->Current.y >= Human.y - 4)	//if an enemy is 4 or less spaces close to the player
			{
				EnemyVariable[i]->stun = true;	//the enemy is stunned
			}
		}
	}
	if (type == 3)			//if your class is Ion
	{
		CanBeDestroyed = false;	//you can’t be Destroyed
	}
	if (type == 4)		//if your class is Slower
	{
		IsSlow = true;
		SpeedOfEnemies = 10;		//changes the speed of the enemies to a smaller one
	}
}

struct Value
{
	int x, y, info;
};

int Neighbor(int i, int j)		//sub return how many neighbor spaces in the grid are walls
{
	int v = 0;
	if (a[i + 1][j] != 0)
		v++;
	if (a[i - 1][j] != 0)
		v++;
	if (a[i][j + 1] != 0)
		v++;
	if (a[i][j - 1] != 0)
		v++;
	return v;
}

void Optimization()		//sub used to optimize the maze building
{
	int i, j, NeighborSpaces;
	for (i = 1; i <= n; i++)
		for (j = 1; j <= m; j++)
			if (a[i][j] != 0)
			{
				NeighborSpaces = Neighbor(i, j);		//get how many Neighbors of this space are walls
				if (NeighborSpaces == 4)	//if there are walls in all 4 directions, the value of a is going to be a cross in ASCII
					a[i][j] = 197;
				else if (NeighborSpaces == 3)	//if there are walls in only 3 directions
				{
					if (a[i + 1][j] == 0)	//if there is not a wall South
						a[i][j] = 193;
					else if (a[i][j - 1] == 0)	//if there is not a wall West
						a[i][j] = 195;
					else if (a[i - 1][j] == 0)	//if there is not a wall North
						a[i][j] = 194;
					else a[i][j] = 180;	//if there is not a wall East
				}
				else if (NeighborSpaces == 2)	//if there are only walls in only 2 directions
				{
					if (a[i + 1][j] != 0 && a[i][j - 1] != 0)		//if there is a wall in the West and in the South
						a[i][j] = 191;
					else if (a[i - 1][j] != 0 && a[i][j + 1] != 0)	//if there is a wall in the North and East
						a[i][j] = 192;
					else if (a[i + 1][j] != 0 && a[i][j + 1] != 0)	//if there is a wall in the East and South
						a[i][j] = 218;
					else if (a[i - 1][j] != 0 && a[i][j - 1] != 0)	//if there is a wall in the North and West
						a[i][j] = 217;
				}
			}
}

void Creation(
	int XDirectionStart, //DE UNDE POT GENERA X
	int XDirectionFinish, //PANA UNDE POT GENREA X
	int YDirectionStart, //SAME SHIT Y
	int YDirectionFinish, //SAME SHIT Y
	int Horizontal	//VERIFICA DACA ULTIMUL PERETE A FOST UNUL ORIZONTAL SAU UNUL VERTICAL
)	//sub used to create the proper maze
{
	if ((XDirectionStart + 1 < XDirectionFinish) && (YDirectionStart + 1 < YDirectionFinish))
	{
		if (Horizontal)
		{
			int Line = XDirectionStart;
			while (Line == XDirectionStart)
				Line = RNG(XDirectionFinish - XDirectionStart) + XDirectionStart - 1;
			int i;
			for (i = YDirectionStart; i <= YDirectionFinish; i++)
				a[Line][i] = horizontal_line;
			int Gate = RNG(YDirectionFinish - YDirectionStart) + YDirectionStart - 1;
			Creation(XDirectionStart, Line - 1, YDirectionStart, YDirectionFinish, 0);
			Creation(Line + 1, XDirectionFinish, YDirectionStart, YDirectionFinish, 0);
			while (a[Line + 1][Gate] != 0 || a[Line - 1][Gate] != 0)
				Gate = RNG(YDirectionFinish - YDirectionStart) + YDirectionStart - 1;
			a[Line][Gate] = 0;
		}
		else
		{
			int Line = YDirectionStart;
			while (Line == YDirectionStart)
				Line = RNG(YDirectionFinish - YDirectionStart) + YDirectionStart - 1;
			int i;
			for (i = XDirectionStart; i <= XDirectionFinish; i++)
				a[i][Line] = vertical_line;
			int Gate = RNG(XDirectionFinish - XDirectionStart) + XDirectionStart - 1;
			Creation(XDirectionStart, XDirectionFinish, YDirectionStart, Line - 1, 1);
			Creation(XDirectionStart, XDirectionFinish, Line + 1, YDirectionFinish, 1);
			while (a[Gate][Line - 1] != 0 || a[Gate][Line + 1] != 0)
				Gate = RNG(XDirectionFinish - XDirectionStart) + XDirectionStart - 1;
			a[Gate][Line] = 0;
		}
	}
}

void construiesteDrum(int x, int y, Value use[201][201])		//IMI CONTRUIESTE PE MATRICEA MEA DRUMUL AFLAT
{
	int k = 1;			//VA FI LUNGIMEA DRUMULUI
	while (x != -1 && y != -1)
	{
		Road[k].x = x;
		Road[k].y = y;
		x = use[x][y].x;
		y = use[Road[k].x][y].y;
		k++;
	}
	k--;
	Road[0].x = k;			//MEMOREZ LUNGIMEA DRUMUIL IN ROAD[0].X PENTRU CA E NEFOLOSITA
	int i;
	if (k > 20)
	{
		for (i = k; i >= k - 20; i--)
			b[Road[i].x][Road[i].y] = 1;		//MATRICEA IN CARE AM MARCAT DRUMUL
	}
	else
	{
		for (i = k; i >= 1; i--)
			b[Road[i].x][Road[i].y] = 1;
	}
}

void wayOut(man Human, man destination)	//lee
{
	int i, j, k;
	man coada[5001];
	Value use[201][201];		
	for (i = 1; i <= n; i++)
		for (j = 1; j <= m; j++)
			use[i][j].info = b[i][j] = a[i][j];
	use[Human.x][Human.y].info = 1;
	i = Human.x;
	j = Human.y;
	use[i][j].x = use[i][j].y = -1;
	int c, p, ok = 0;
	c = p = 1;
	coada[1].x = i;
	coada[1].y = j;
	while (c <= p&&ok == 0)
	{
		i = coada[c].x;
		j = coada[c].y;
		if ((i == destination.x) && (j == destination.y))
		{
			ok = 1;
			construiesteDrum(i, j, use);
		}
		else
		{
			for (k = 0; k < 4; k++)
				if (use[i + dx[k]][j + dy[k]].info == 0 || use[i + dx[k]][j + dy[k]].info == ene)			//VERIFIC DACA POT SA MERG INTR-O ANUMITA DIRECTIE
				{
					use[i + dx[k]][j + dy[k]].info = 1;						//DACA POT, ATUNCI MARCHEZ DIRECTIA IN CARE MERG CA VIZITATA
					use[i + dx[k]][j + dy[k]].x = i;						//AICI IMI MARCHEZ POZITIA DIN CARE AM VENIT
					use[i + dx[k]][j + dy[k]].y = j;
					p++;												//IMI CRESC COADA
					coada[p].x = i + dx[k];								//ADAUG IN COADA NOUA POZITIE IN CARE POT SA MERG
					coada[p].y = j + dy[k];
					if ((i + dx[k] == destination.x) && (j + dy[k] == destination.y))				//DACA AM AJUNS LA DESTINATIE
					{
						ok = 1;
						construiesteDrum(i, j, use);
					}
				}
			c++;			//TREC LA URMATOAREA VALOARE DIN COADA
		}
	}
}

void initialize(int dif)
{
	int i, j, k;
	CanBeDestroyed = true;
	if (dif == 1)
	{
		n = m = 25;			//DIMENSIUNILE MATRIXII
		NumberOfEnemies = 7;		//CATI INAMICI SUNT PE HARTA(DUPA PAREREA MEA, POTI SCHIMBA, DAR AI GRIJA SA NU SE AGLOMEREZE PENTRU CA ITI DA EEROARE LA CREAREA LABIRINTULUI
		SpeedOfEnemies = 5 - dif;	//VITEZA INAMICULUI - AMINTESTE-TI VITEZEW E IN FUNCTIE DE NUMARUL DE REFRESH-URI ALE PAGINII
	}
	else if (dif == 2)
	{
		NumberOfEnemies = 17;
		n = 30;
		m = 50;
		SpeedOfEnemies = 5 - dif;
	}
	else if (dif == 3)
	{
		NumberOfEnemies = 33;
		n = 43;
		m = 100;
		SpeedOfEnemies = 5 - dif;
	}
	for (i = 1; i <= 45; i++)
		for (j = 1; j <= 120; j++)
			b[i][j] = a[i][j] = 0, Characters[i*j] = NULL;			//IMI INITIALIZEZ MATRICILE CU 0
	for (i = 0; i < 200; i++)
		Line[i] = coloana[i] = 0;		//VECTORI CARE IMI VERTIFICA DACA PE LINIA SAU COLOANA RESPECTIVA AM GENERAT SAU NU VREUN INAMIC SAU NU
	for (i = 2; i <= n - 1; i++)
	{
		a[i][1] = a[i][m] = 179;		//IMI PUN PERETII(BORDEZ MATRICEA)
	}
	for (i = 2; i <= m - 1; i++)
		a[1][i] = a[n][i] = 196;		//IMI PUN PERETII(BORDEZ MATRICEA)
	a[1][1] = 218;					//COLURILE
	a[1][m] = 191;					//COLURILE
	a[n][1] = 192;					//COLURILE
	a[n][m] = 217;					//COLURILE
	int x = RNG(2) - 1;
	Creation(2, n - 1, 2, m - 1, x);
	a[n][m - 1] = 0;				//IMI SETEZT POARTA DE ISESIRE DIN LABIRINT
	Optimization();
	Human.x = Human.y = 2;			//SETEZ POZITIA DE START
	a[2][2] = Player;
}

int draw(clock_t timer, int dif, int type)
{
	man d;		//ASTA CRED CA E POZITIA IESIRII
	d.x = n;
	d.y = m - 1;
	int ok = 0;
	if (_kbhit())			//VERIFICA DA AM APAST VREO TASTA
	{
		switch (_getch())
		{
		case KEY_UP:
		{
			if (a[Human.x - 1][Human.y] == 0 || a[Human.x - 1][Human.y] == ene && !CanBeDestroyed)		//VERIFICA DACA POT SA MERG IN SUS
			{
				a[Human.x - 1][Human.y] = Player;					//FAC CE AM FACUT SI LA ENEMY	 - IL MUT PE NOUA POZITIE
				a[Human.x][Human.y] = 0;							//IL STERG DE PE VECHEA POZITIE
				if (!Night)											//DACA NU E NOATE STI CE FAC
				{
					gotoxy(Human.y - 1, Human.x + 1);
					printf(" ");
					gotoxy(Human.y - 1, Human.x - 1 + 1);
					printf("%c", Player);
				}
				else if (Night)								//DACA E NOAPTE SITU CE FAC
				{
					int i, j;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf(" ");
						}
					}
					Human.x -= 1;									//MODIFIC POZITIA JUCATORULUI
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)		//IN FOR-URILE ASTEA PRINTEZ CE ESTRE IN JURUL MEU
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf("%c", (char)a[i][j]);
						}
					}
					Human.x += 1;
				}
				Human.x--;
			}
			break;
		}
		case KEY_DOWN:
		{
			if (a[Human.x + 1][Human.y] == 0 || a[Human.x - 1][Human.y] == ene && !CanBeDestroyed)
			{
				a[Human.x + 1][Human.y] = Player;
				a[Human.x][Human.y] = 0;
				if (!Night)
				{
					gotoxy(Human.y - 1, Human.x + 1);
					printf(" ");
					gotoxy(Human.y - 1, Human.x + 1 + 1);
					printf("%c", Player);
				}
				else if (Night)
				{
					int i, j;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf(" ");
						}
					}
					Human.x += 1;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf("%c", (char)a[i][j]);
						}
					}
					Human.x -= 1;
				}
				Human.x++;
			}
			break;
		}
		case KEY_LEFT:
		{
			if (a[Human.x][Human.y - 1] == 0 || a[Human.x - 1][Human.y] == ene && !CanBeDestroyed)
			{
				a[Human.x][Human.y - 1] = Player;
				a[Human.x][Human.y] = 0;
				if (!Night)
				{
					gotoxy(Human.y - 1, Human.x + 1);
					printf(" ");
					gotoxy(Human.y - 1 - 1, Human.x + 1);
					printf("%c", Player);
				}
				else if (Night)
				{
					int i, j;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf(" ");
						}
					}
					Human.y -= 1;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf("%c", (char)a[i][j]);
						}
					}
					Human.y += 1;
				}
				Human.y--;
			}
			break;
		}
		case KEY_RIGHT:
		{
			if (a[Human.x][Human.y + 1] == 0 || a[Human.x - 1][Human.y] == ene && !CanBeDestroyed)
			{
				a[Human.x][Human.y + 1] = Player;
				a[Human.x][Human.y] = 0;
				if (!Night)
				{
					gotoxy(Human.y - 1, Human.x + 1);
					printf(" ");
					gotoxy(Human.y + 1 - 1, Human.x + 1);
					printf("%c", Player);
				}
				else if (Night)
				{
					int i, j;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf(" ");
						}
					}
					Human.y += 1;
					for (i = GreaterValue(Human.x - 4, 0); i <= SmallerValue(Human.x + 4, n); i++)
					{
						for (j = GreaterValue(Human.y - 4, 0); j <= SmallerValue(Human.y + 4, m); j++)
						{
							gotoxy(j - 1, i + 1);
							printf("%c", (char)a[i][j]);
						}
					}
					Human.y -= 1;
				}
				Human.y++;
			}
			break;
		}
		case HINT:					//HINT-UL 
		{
			if (NumberOfHints > 0)		//VERIFIC DACA MAI MA HINT-URI
			{
				NumberOfHints--;		//LE SCAD
				wayOut(Human, d);		//APELEZ LEE-UL
				print(b, -1, false);
				ok = 0;
				clock_t t;
				t = clock();
				while (clock() - t < 5000)
					if (_kbhit())
					{
						system("cls");
						print(a, timer, true);
						return 1;
					}
				system("cls");
				print(a, timer, true);
				return 1;
			}
			break;
		}
		case SPECIAL:					//PUTEREA SPECIALA - ARE COOLDOWN DE 12 SECUNDE
		{
			if (clock() - PowerTimer > 12000)
			{
				PowerTimer = clock();
				special(type);
			}
			break;
		}
		case VK_ESCAPE:			//APAS ESCAPE CA SA IES DIN JOC
		{
			exit(EXIT_SUCCESS);
		}
		case VK_BACK:			
		{
			Human.x = d.x;
			Human.y = d.y;
			break;
		}
		default:
		{
			ok = 0;
		}
		break;
		}
	}
	if (Human.x == d.x&&Human.y == d.y)
	{
		system("CLS");
		cout << "YOU WIN!" << endl;
		cout << "Enter your name: ";
		string name;
		cin >> name;
		if (dif == 1)
		{
			highscore aux;
			int i, j;
			for (i = 0; i < 5; i++)
				if (easy[i].Exists == false)
					break;
			for (int j = 0; j < 3; j++)
				easy[i].name[j] = ' ';
			for (int j = 0; j < name.size(); j++)
				easy[i].name[j] = name[j];
			easy[i].time = timer / 1000;
			easy[i].Exists = true;
			int n = i + 1;
			for (i = 0; i < n - 1; i++)				//O METODA DE SORTARE
				for (j = i + 1; j < n; j++)
					if (easy[i].time>easy[j].time)
					{
						aux = easy[i];
						easy[i] = easy[j];
						easy[j] = aux;
					}
			easy[6].Exists = false;
			ofstream fout1("highscore_easy.txt");
			for (i = 0; i < 5; i++)
			{
				if (easy[i].Exists == true)
				{
					fout1 << easy[i].name << "  " << easy[i].time << endl;
				}
				else break;
			}
			while (i < 5)
			{
				fout1 << "---  ----\n";
				i++;
			}
			fout1.close();
		}
		else if (dif == 2)
		{
			highscore aux;
			int i, j;
			for (i = 0; i < 5; i++)
				if (medium[i].Exists == false)
					break;
			for (int j = 0; j < 3; j++)
				medium[i].name[j] = ' ';
			for (int j = 0; j < name.size(); j++)
				medium[i].name[j] = name[j];
			medium[i].name[3] = NULL;
			medium[i].time = timer / 1000;
			medium[i].Exists = true;
			int n = i + 1;
			for (i = 0; i < n - 1; i++)
				for (j = i + 1; j < n; j++)
					if (medium[i].time>medium[j].time)
					{
						aux = medium[i];
						medium[i] = medium[j];
						medium[j] = aux;
					}
			medium[6].Exists = false;
			ofstream fout2("highscore_medium.txt");
			for (i = 0; i < 5; i++)
			{
				if (medium[i].Exists == true)
				{
					fout2 << medium[i].name << "  " << medium[i].time << endl;
				}
				else break;
			}
			while (i < 5)
			{
				fout2 << "---  ----\n";
				i++;
			}
			fout2.close();
		}
		else if (dif == 3)
		{
			highscore aux;
			int i, j;
			for (i = 0; i < 5; i++)
				if (hard[i].Exists == false)
					break;
			for (int j = 0; j < 3; j++)
				hard[i].name[j] = ' ';
			for (int j = 0; j < name.size(); j++)
				hard[i].name[j] = name[j];
			hard[i].name[3] = NULL;
			hard[i].time = timer / 1000;
			hard[i].Exists = true;
			int n = i + 1;
			for (i = 0; i < n - 1; i++)
				for (j = i + 1; j < n; j++)
					if (hard[i].time>hard[j].time)
					{
						aux = hard[i];
						hard[i] = hard[j];
						hard[j] = aux;
					}
			hard[6].Exists = false;
			ofstream fout3("highscore_hard.txt");
			for (i = 0; i < 5; i++)
			{
				if (hard[i].Exists == true)
				{
					fout3 << hard[i].name << "  " << hard[i].time << endl;
				}
				else break;
			}
			while (i < 5)
			{
				fout3 << "---  ----\n";
				i++;
			}
			fout3.close();
		}
		return 0;
	}
	return 1;
}

void play(int dif, int type)
{
	for (int i = 0; i < NumberOfEnemies; i++)
		EnemyVariable[i] = EnemyVariable[i]->enemy1();		//AM CREAT INAMICII CU POINTERII!!!!!  TINE MINTE STEFAN
	int cont = 0;
	clock_t startTime = clock();		//PORNESC TIMPUL
	clock_t timer = 0;
	print(a, timer, true);				//PRINTEZ TIMPUL
	while (draw(timer, dif, type) && !ok2)		//OK2 - DACA AM PIERDUT INCA SAU NU		
	{
		gotoxy(Human.y - 1, Human.x + 1);		//PRINTEAZA PLAYERUL
		printf("%c", Player);
		cont += 1;								//AICI VAD DACA PRINTEZ INAMICII SAU NU - ERA CHESTIA CU TICK-URILE DE LA CLOCK CA VEZIO COUNT-UL POATE SA FIE DOAR UN REST AL SPEEDOFENEMIES
		cont %= (SpeedOfEnemies / 2);
		if (cont % (SpeedOfEnemies / 2) == 0)
		{
			for (int i = 0; i < NumberOfEnemies; i++)
			{
				if (!EnemyVariable[i]->dest && !ok2)		//DACA INAMICUL NU A FOST  DISTRUS SI NU AI PIERDUTR INCA JOCUL
				{
					EnemyVariable[i]->walk();				//APELEZ FUNCTIA DE MERS CA SA FAC INAMICUL SA MEARGA
					if (CanBeDestroyed && EnemyVariable[i]->Current.x == Human.x&&EnemyVariable[i]->Current.y == Human.y)	//AICI VERIFIC DACA S-A UN INAMIC S-A INTALNIT CU PLAYER-UL
					{
						ok2 = 1;			//MARCHEZ CA JOC PIERDUT - AI MURIT
					}
				}
				if (ok2)				//JOC PIERDUT
				{
					system("CLS");
					cout << "YOU LOSE!" << endl;
					break;
				}
			}
		}
		if (!CanBeDestroyed)
		{
			if (!type3)					//TYPE3 ESTE TIMPUL CARE IMI VERIFICA DACA AM COOLDOWN LA POWER UP SAU NU
				type3 = clock();
			if (clock() - type3 > 2000)
			{
				CanBeDestroyed = true;
				type3 = 0;
			}
		}

		if (IsSlow)
		{
			if (!type3)
				type3 = clock();
			if (clock() - type3 > 5000)
			{
				IsSlow = false;
				SpeedOfEnemies = 6;
				type3 = 0;
			}
		}
		if (!ok2)
		{
			timer = clock() - startTime;
			print(a, timer, false);
			Sleep(25);		//0.025 SECUNDE - FOARTE MIC - I-AM DAT CA SA AIBA PUTIN PAUZA -  SANU RULEZE NON-STOP
		}
	}
	for (int i = 0; i < NumberOfEnemies; i++)			//DUPA CE AM TERMIANT JOCUL DISTRUG TOTI INAMICII
		EnemyVariable[i]->destroy();
}

int startGamePrint()
{
	string menu[4] = { "Easy","Medium","Hard","Back" };
	string day[2] = { "Day","Night" };
	string character[5] = { "Destroyer","Chaos","Ion","Slower" };
	int pointer = 0;
	int ok;
	int dif = 0;
	while (!dif)
	{
		system("cls");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "Start Game\n\n";
		for (int i = 0; i < 4; i++)
		{
			if (i == 3)
				cout << endl;
			if (i == pointer)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				cout << menu[i] << endl;
			}
			else
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				cout << menu[i] << endl;
			}
		}
		while (true)
		{
			if (_kbhit())
			{
				int c;
				c = _getch();
				if (c == KEY_UP)
				{
					pointer--;
					if (pointer <0)
						pointer = 3;
					break;
				}
				else if (c == KEY_DOWN)
				{
					pointer++;
					if (pointer >3)
						pointer = 0;
					break;
				}
				if (c == VK_RETURN)
				{
					if (pointer == 0)
						dif = 1;
					else if (pointer == 1)
						dif = 2;
					else if (pointer == 2)
						dif = 3;
					else if (pointer == 3)
						return 0;
					break;
				}
			}
		}
	}
	pointer = 0;
	ok = 1;
	Night = 0;
	while (ok)
	{
		system("cls");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "Day or Night?\n\n";
		for (int i = 0; i < 2; i++)
		{
			if (i == pointer)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				cout << day[i] << endl;
			}
			else
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				cout << day[i] << endl;
			}
		}
		while (true)
		{
			if (_kbhit())
			{
				int c;
				c = _getch();
				if (c == KEY_UP)
				{
					pointer--;
					if (pointer <0)
						pointer = 1;
					break;
				}
				else if (c == KEY_DOWN)
				{
					pointer++;
					if (pointer >1)
						pointer = 0;
					break;
				}
				if (c == VK_RETURN)
				{
					if (pointer == 1)
						Night = 1;
					ok = 0;
					break;
				}
			}
		}
	}
	int type;
	ok = 1;
	while (ok)
	{
		system("cls");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "Choose your character:\n\n";
		for (int i = 0; i < 4; i++)
		{
			if (i == pointer)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				cout << character[i];
				if (i == 0)
					cout << " - You can destroy a enemy." << endl;
				else if (i == 1)
					cout << " - You can stun some your enemys for 5 seconds." << endl;
				else if (i == 2)
					cout << " - You can become invulnerable for one second." << endl;
				else if (i == 3)
					cout << " - You can slow all your enemys for 5 seconds." << endl;
			}
			else
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				cout << character[i] << endl;
			}
		}
		while (true)
		{
			if (_kbhit())
			{
				int c;
				c = _getch();
				if (c == KEY_UP)
				{
					pointer--;
					if (pointer <0)
						pointer = 3;
					break;
				}
				else if (c == KEY_DOWN)
				{
					pointer++;
					if (pointer >3)
						pointer = 0;
					break;
				}
				if (c == VK_RETURN)
				{
					type = pointer + 1;
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					ok = 0;
					break;
				}
			}
		}
	}
	initialize(dif);
	ok2 = 0;
	NumberOfHints = 2;
	system("cls");
	if (Night)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	PowerTimer = clock() - 12000;
	CanBeDestroyed = true;
	IsSlow = false;
	type3 = 0;
	SpeedOfEnemies = 6;
	play(dif, type);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	cin.get();
	return 0;
}

void printEasy()
{
	ifstream fin("highscore_easy.txt");
	system("cls");
	cout << "Highscores - Easy\n\n";
	int i;
	char c[11];
	for (i = 0; i<11; i++)
		c[i] = NULL;
	for (int j = 0; j < 5; j++)
	{
		fin >> c;			//CITESC INTIAL NUMELE
		cout << c << "  ";	//AFISEZ NUMELE
		fin >> c;			//CITESC TIMPUL
		cout << c;			//AFISEZ TIMPUL
		cout << '\n';		//TREC LA URTMNATOARE LINIE
	}
	fin.close();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	cout << "\nBack";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	cin.get();
}

void printMedium()
{
	ifstream fin("highscore_medium.txt");
	system("cls");
	cout << "Highscores - Medium\n\n";
	int i;
	char c[11];
	for (i = 0; i<11; i++)
		c[i] = NULL;
	for (int j = 0; j < 5; j++)
	{
		fin >> c;
		cout << c << "  ";
		fin >> c;
		cout << c;
		cout << '\n';
	}
	fin.close();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	cout << "\nBack";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	cin.get();
}

void printHard()
{
	ifstream fin("highscore_hard.txt");
	system("cls");
	cout << "Highscores - Hard\n\n";
	int i;
	char c[11];
	for (i = 0; i<11; i++)
		c[i] = NULL;
	for (int j = 0; j < 5; j++)
	{
		fin >> c;
		cout << c << "  ";
		fin >> c;
		cout << c;
		cout << '\n';
	}
	fin.close();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	cout << "\nBack";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	cin.get();
}

void highScoresPrint()
{
	string menu[4] = { "Easy","Medium","Hard","Back" };
	system("CLS");
	int pointer = 0;
	while (true)
	{
		system("cls");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "Highscores\n\n";
		for (int i = 0; i < 4; i++)
		{
			if (i == 3)
				cout << endl;
			if (i == pointer)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				cout << menu[i] << endl;
			}
			else
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				cout << menu[i] << endl;
			}
		}
		while (true)
		{
			if (_kbhit())
			{
				int c;
				c = _getch();
				if (c == KEY_UP)
				{
					pointer--;
					if (pointer <0)
						pointer = 3;
					break;
				}
				else if (c == KEY_DOWN)
				{
					pointer++;
					if (pointer >3)
						pointer = 0;
					break;
				}
				if (c == VK_RETURN)
				{
					if (pointer == 0)
						printEasy();
					else if (pointer == 1)
						printMedium();
					else if (pointer == 2)
						printHard();
					else if (pointer == 3)
						return;
					break;
				}
			}
		}
	}
	return;
}

int printMenu()
{
	int ok;
	int pointer = 0;			//POINTER-UL ITI ARATA UNDE TE AFLII IN MENIU
	string menu[3] = { "Start Game","Highscores","Exit" };
	while (true)
	{
		system("cls");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		cout << "Main Menu\n\n";
		for (int i = 0; i < 3; i++)
		{
			if (i == pointer)
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);		//SETEZ CULOARE ROSU
				cout << menu[i] << endl;
			}
			else
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);	//SETEZ CULOARE ALB
				cout << menu[i] << endl;
			}
		}
		ok = 1;
		ok2 = 0;
		while (ok)
		{
			if (_kbhit())		//KEYBORD HIT - FUNCTIE DE LIBRARIE CARE VERIFICA DACA S-A ATINS VREO TASTA
			{
				int c;
				c = _getch();	//PREIA TASTA RESPECTIVA
				if (c == KEY_UP)
				{
					pointer--;
					if (pointer < 0)
						pointer = 2;
					ok = 0;
				}
				else if (c == KEY_DOWN)
				{
					pointer++;
					if (pointer > 2)
						pointer = 0;
					ok = 0;
				}
				if (c == VK_RETURN)			//VK_RETURN = TASTA ENTER
				{
					if (pointer == 0)
						startGamePrint();
					else if (pointer == 1)
						highScoresPrint();
					else if (pointer == 2)
						return 0;
					ok = 0;
				}
			}
		}
	}
}

bool ifLetter(int c)		//returns true if 'c' is a letter; returns false otherwise
{
	if (c >= 'A'&&c <= 'Z' || c >= 'a'&&c <= 'z')
		return true;
	return false;
}

void SetFont()		//sub that sets the font size
{
	HANDLE Hout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_FONT_INFOEX font = { sizeof(font) };
	::GetCurrentConsoleFontEx(Hout, FALSE, &font);
	font.dwFontSize.X = 10;
	font.dwFontSize.Y = 18;
	SetCurrentConsoleFontEx(Hout, FALSE, &font);
}

int main()
{
	ShowConsoleCursor(false);	//used to hide the Console Cursor
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, NULL);  //sets the game to FULLSCREEM mode
	SetFont();		//sets the font size
	srand(time(NULL));		//use of the current time in order to get differnt sequences of random numbers everytime you run the program
	ifstream fin1("highscore_easy.txt");		//allocate names for the highscore text files: fin1 for easy, fin2 for medium and fin3 for hard
	ifstream fin2("highscore_medium.txt");
	ifstream fin3("highscore_hard.txt");
	int i = 0, j;
	clock_t time;		//follow the clock ticks so you can measure time
	if (!fin1.good())
	{
		ofstream fout("highscore_easy.txt");		//uses the easy highscore text file as output file
		while (i < 5)		//i is set to 0 before, so there will be 5 lines written in the file
		{
			cout << "---  ----\n";		//each line is going to contain ---  ---- which will show in case there is no score written on that place
			i++;
		}
		fout.close();
	}
	i = 0;
	if (!fin2.good())
	{
		ofstream fout("highscore_medium.txt");		//uses the medium highscore text file as output file
		while (i < 5)
		{
			cout << "---  ----\n";		//each line is going to contain ---  ---- which will show in case there is no score written on that place
			i++;
		}
		fout.close();
	}
	i = 0;
	if (!fin3.good())
	{
		ofstream fout("highscore_hard.txt");		//uses the hard highscore text file as output file
		while (i < 5)
		{
			cout << "---  ----\n";		//each line is going to contain ---  ---- which will show in case there is no score written on that place
			i++;
		}
		fout.close();
	}
	for (i = 0; i < 5; i++)
	{
		char num[4];		//array of chars
		if (i != 0)			//CITESTE TOATE NEWLINE-URILE IN AFARA DE CEL DE PE PRIMA LINIE PENTRU CA ALKA NU EXSITA BALAN!!!
			fin1.get();
		fin1.get(num, 4);	//CITESTE PRIMELE 4 CARACTERE
		if (!ifLetter(num[0]))	//IMI VERIFICA DACA PRIMUL CARACTER ESTE O LITERA, DACA ESTE INSEAMNA CA EXISTA UN NUME SI UN HIGHSCORE PE LINIA RESPECTIVA, DACA NU ATUNCI VA CIITI -
		{
			for (j = i; j < 5; j++)		//AICI SETAM CA NU EXISTA UN NUME PE LINIILE URMATOARE
			{
				easy[j].Exists = false;
			}
			break;
		}
		else
		{
			easy[i].Exists = true;		//DACA EXISTA O LITERA PE LINIA RESPECTIVA
			for (int j = 0; j < 3; j++)
				easy[i].name[j] = num[j];	//SETAZA NUMELE JUCATORULUI
			fin1.get();
			fin1.get();
			fin1 >> time;
			easy[i].time = time;			//SETEAZA TIMPUL PENTRU JUCATORUL RESPECTIV
		}
	}
	for (i = 0; i < 5; i++)
	{
		char nume[4];		//array of chars
		if (i != 0)
			fin2.get();
		fin2.get(nume, 4);
		if (!ifLetter(nume[0]))
		{
			for (j = i; j < 5; j++)
			{
				medium[j].Exists = false;
			}
			break;
		}
		else
		{
			medium[i].Exists = true;
			for (int j = 0; j < 3; j++)
				medium[i].name[j] = nume[j];
			fin2.get();
			fin2.get();
			fin2 >> time;
			medium[i].time = time;
		}
	}
	for (i = 0; i < 5; i++)
	{
		char nume[4];
		if (i != 0)
			fin3.get();
		fin3.get(nume, 4);
		if (!ifLetter(nume[0]))
		{
			for (int j = i; j < 5; j++)
			{
				hard[j].Exists = false;
			}
		}
		else
		{
			hard[i].Exists = true;
			for (int j = 0; j < 3; j++)
				hard[i].name[j] = nume[j];
			fin3.get();
			fin3.get();
			fin3 >> time;
			hard[i].time = time;
		}
	}
	fin1.close();		//close all the text files
	fin2.close();
	fin3.close();
	printMenu();		//calls the printMenu sub and the game starts
	return 0;
}

