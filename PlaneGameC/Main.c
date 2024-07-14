				
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define true 1
#define false 0

typedef char bool;

struct Node {
	void* data;
	struct Node* next;
};

typedef struct {
	struct Node* firstNode;
	struct Node* prevNode;
	struct Node* currNode;
} List;

List* ListCreate(void) {
	List* ll = malloc(sizeof(List));
	ll->currNode = NULL;
	ll->prevNode = NULL;
	ll->firstNode = NULL;
	return ll;
}

int ListGetSize(List* list) {
	int iter = 0;
	list->currNode = list->firstNode;
	while (list->currNode != NULL) {
		list->currNode = list->currNode->next;
		iter++;
	}
	return iter;
}

void ListInsert(List* list, void* data) {	
	if (list->firstNode == NULL) {
		list->firstNode = malloc(sizeof(struct Node));
		list->firstNode->data = data;
		list->firstNode->next = NULL;
	} else if (list->firstNode->next == NULL) {
		list->firstNode->next = malloc(sizeof(struct Node));
		list->firstNode->next->data = data;
		list->firstNode->next->next = NULL;
	} else {
		list->currNode = list->firstNode;
		while (list->currNode != NULL) {
			if (list->currNode->next == NULL) {
				list->currNode->next = malloc(sizeof(struct Node));
				list->currNode->next->data = data;
				list->currNode->next->next = NULL;
				break;
			}
			list->currNode = list->currNode->next;
		}
	}
}

void* ListReturn(List* list, int index) {
	int iter = 0;
	list->currNode = list->firstNode;
	while (list->currNode != NULL) {
		if (iter == index) {
			return list->currNode->data;
		}
		list->currNode = list->currNode->next;
		iter++;
	}
}

void ListDelete(List* list, int index) {
	int iter = 0;
	if (index == 0) {
		if (ListGetSize(list) > 1) {
			struct Node* ff = list->firstNode->next;
			free(list->firstNode);
			list->firstNode = ff;
		} else {
			free(list->firstNode);
			list->firstNode = NULL;
		}
	} else{
		list->currNode = list->firstNode;
		while (list->currNode != NULL) {
			if (iter == index) {
				struct Node* pp = list->currNode;
				list->prevNode->next = pp->next;
				free(pp);
				break;
			}
			list->prevNode = list->currNode;
			list->currNode = list->currNode->next;
			iter++;
		}
	}
}

void ListClear(List* list) {
	while (ListGetSize(list) > 0) {
		ListDelete(list, ListGetSize(list) - 1);
	}
}

void ListDestroy(List* list) {
	while (ListGetSize(list) > 0) {
		ListDelete(list, ListGetSize(list) - 1);
	}
	free(list);
}

bool IsColliding(SDL_Rect one, SDL_Rect two, float rad) {
	float xx = one.x + (one.w / 2.0f) - two.x + (two.w / 2.0f);
	float yy = one.y + (one.h / 2.0f) - two.y + (two.h / 2.0f);
	float dis = sqrt(xx * xx + yy * yy);
	if (dis < rad) {
		return true;
	}
	return false;
}

float DotProduct(float dxone, float dyone, float dxtwo, float dytwo) {
	return (dxone * dxtwo) + (dyone * dytwo);
}

void Normalize(float dx, float dy, float* rx, float* ry) {
	float len = sqrt(dx * dx + dy * dy);
	*rx = dx / len;
	*ry = dy / len;
}

bool FacingPlayer(float fone, float ftwo, float dxone, float dyone, float dxtwo, float dytwo) {
	float xxone = 0;
	float yyone = 0;
	float xxtwo = 0;
	float yytwo = 0;
	Normalize(dxtwo - dxone, dytwo - dyone, &xxone, &yyone);
	Normalize(fone, ftwo, &fone, &ftwo);
	float dd = DotProduct(xxone, yyone, fone, ftwo);
	if (dd > 0.8f) {
		return true;
	}
	return false;
}

SDL_Texture* LoadTexture(SDL_Renderer* rend, const char* path) {
	SDL_Surface* ss = IMG_Load(path);
	SDL_Texture* tt = SDL_CreateTextureFromSurface(rend, ss);
	free(ss);
	if (!tt) {
		printf("Could Not Load: %s\n", path);
	}
	return tt;
}

SDL_Texture* LoadText(SDL_Renderer* rend, TTF_Font* font, const char* str) {
	SDL_Surface* ss = TTF_RenderText_Solid(font, str, (SDL_Color){255, 255, 255, 255});
	SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, ss);
	if (!tex) {
		printf("Could Not Load Text...\n");
	}	
	return tex;
}

typedef struct {
	SDL_Window* window;
	SDL_Renderer* rend;
	SDL_Texture* rendTex;
} CoreGame;

typedef struct {
	SDL_Rect bounds;
	SDL_Texture* tex;
	float angle;
	int dx, dy;
	float speed;
	bool dead;
	int alpha;
} Player;

Player* PlayerCreate(SDL_Renderer* rend) {
	Player* pp = malloc(sizeof(Player));
	pp->bounds = (SDL_Rect){450, 250, 100, 30};
	pp->angle = 0.0f;
	pp->dx = 0;
	pp->dy = 0;
	pp->speed = 5.0f;
	pp->dead = false;
	pp->tex = LoadTexture(rend, "Data/playerSprite.png");
	pp->alpha = 255;
	return pp;
}

void PlayerDestroy(Player* pp) {
	SDL_DestroyTexture(pp->tex);
	free(pp);
}

void PlayerUpdate(Player* pp, bool uu, bool dd) {
	if (uu) {
		pp->angle -= 5.0f;
	}
	if (dd) {
		pp->angle += 5.0f;
	}
	pp->dx = -(cos(pp->angle * (3.14f / 180.0f)) * pp->speed);
	pp->dy = -(sin(pp->angle * (3.14f / 180.0f)) * pp->speed);
	if (pp->dead) {
		if (pp->angle < 80) {
			pp->angle += 4;
		}
		if (pp->angle > 100) {
			pp->angle -= 4;
		}
		pp->alpha -= 4;
	}
}

void PlayerRender(SDL_Renderer* rend, Player* player) {
	SDL_SetTextureAlphaMod(player->tex, player->alpha);
	SDL_RenderCopyEx(rend, player->tex, NULL, &player->bounds, player->angle, NULL, 0);
}

void PlayerReset(Player* player) {
	player->bounds = (SDL_Rect){450, 250, 100, 30};
	player->angle = 0.0f;
	player->alpha = 255;
	player->dead = false; 
}

typedef struct {
	SDL_Rect bounds;
	float angle;
	float speed;
	int time;
	int iter;
} Projectile;

Projectile* ProjCreate(float x, float y, float angle) {
	Projectile* pp = malloc(sizeof(Projectile));
	pp->bounds = (SDL_Rect){x, y, 50.0f, 20.0f};
	pp->speed =10.0f;
	pp->angle = angle;
	pp->time = 90;
	pp->iter = 0;
	return pp;
}

void ProjDestroy(Projectile* pp) {
	free(pp);
}

void ProjUpdate(Projectile* pp, float dx, float dy) {
	pp->iter++;
	pp->bounds.x += dx;
	pp->bounds.y += dy;
	pp->bounds.x += cos(pp->angle * (3.14f / 180.0f)) * pp->speed;
	pp->bounds.y += sin(pp->angle * (3.14f / 180.0f)) * pp->speed;
}

void ProjRender(SDL_Renderer* rend, SDL_Texture* tex, Projectile* pp) {
	SDL_RenderCopyEx(rend, tex, NULL, &pp->bounds, pp->angle, NULL, 0);
}

typedef struct {
	SDL_Rect bounds;
} Cloud;

Cloud* CloudCreate(float x, float y) {
	Cloud* cc = malloc(sizeof(Cloud));
	cc->bounds = (SDL_Rect){x, y, 100, 100};
	return cc;
}

void CloudDestroy(Cloud* cc) {
	free(cc);
}

void CloudUpdate(Cloud* cc, float dx, float dy) {
	cc->bounds.x += dx;
	cc->bounds.y += dy;
	if (cc->bounds.x > 1000) {
		cc->bounds.x = -100;
	}
	if (cc->bounds.x < -100) {
		cc->bounds.x = 1000;
	}
	if (cc->bounds.y > 600) {
		cc->bounds.y = -100;
	}
	if (cc->bounds.y < -100) {
		cc->bounds.y = 600;
	}
}

void CloudRender(SDL_Renderer* rend, SDL_Texture* tex, Cloud* cc) {
	SDL_RenderCopy(rend, tex, NULL, &cc->bounds);
}

typedef struct {
	SDL_Rect bounds;
	float angle;
	float speed;
	bool dead;
	int alpha;
	int siter, stime;
	bool shoot;
} Enemy;

Enemy* EnemyCreate() {
	Enemy* en = malloc(sizeof(Enemy));
	int tt = rand() % 100;
	if (tt < 25) {
		en->bounds = (SDL_Rect){-100, rand() % 600, 100, 30};
	} else if (tt < 50) {
		en->bounds = (SDL_Rect){1000, rand() % 600, 100, 30};
	} else if (tt < 75) {
		en->bounds = (SDL_Rect){rand() % 1000, -100, 100, 30};
	} else {
		en->bounds = (SDL_Rect){rand() % 1000, 600, 100, 30};
	}
	en->angle = rand() % 360;
	en->speed = 5.0f;
	en->dead = false;
	en->alpha = 255;
	en->stime = 20 + rand() % 90;
	en->siter = 0;
	en->shoot = false;
	return en;
}

void EnemyDestroy(Enemy* en) {
	free(en);
}

void EnemyUpdate(Enemy* en, float dx, float dy) {
	en->bounds.x += dx;
	en->bounds.y += dy;
	en->bounds.x += cos(en->angle * (3.14f / 180.0f)) * en->speed;
	en->bounds.y += sin(en->angle * (3.14f / 180.0f)) * en->speed;
	if (en->bounds.x > 1500) {
		en->bounds.x = -1500;
	}
	if (en->bounds.x < -1500) {
		en->bounds.x = 1500;
	}
	if (en->bounds.y > 750) {
		en->bounds.y = -750;
	}
	if (en->bounds.y < -750) {
		en->bounds.y = 750;
	}
	if (en->dead) {
		if (en->angle < 100) {
			en->angle += 2;
		}
		if (en->angle > 100) {
			en->angle -= 2;
		}
		en->alpha -= 5;
	} else {
		en->siter++;
		if (en->siter > en->stime) {
			en->shoot = true;
			en->siter = 0;
			en->stime = 20 + rand() % 90;
		}
	}
}

void EnemyRender(SDL_Renderer* rend, SDL_Texture* tex, Enemy* en) {
	SDL_SetTextureAlphaMod(tex, en->alpha);
	SDL_RenderCopyEx(rend, tex, NULL, &en->bounds, en->angle, NULL, 0);
}
typedef struct {
	float angle;
	float speed;
	float alpha; 
	SDL_Rect bounds;
} Part;

Part* PartCreate(float x, float y, float angle) {
	Part* pp = malloc(sizeof(Part));
	pp->bounds = (SDL_Rect){x, y, 20, 20};
	pp->angle = angle;
	pp->speed = 5 + rand() % 10;
	pp->alpha = 255;
	return pp;
}

void PartDestroy(Part* pp) {
	free(pp);
}

void PartUpdate(Part* pp, float xx, float yy) {
	pp->bounds.x += xx;
	pp->bounds.y += yy;
	pp->bounds.x += cos(pp->angle * (3.14f / 180.0f)) * pp->speed;
	pp->bounds.y += sin(pp->angle * (3.14f / 180.0f)) * pp->speed;
	pp->alpha -= 5;
}

void PartRender(SDL_Renderer* rend, SDL_Texture* tex, Part* pp) {
	SDL_SetTextureAlphaMod(tex, pp->alpha);
	SDL_RenderCopy(rend, tex, NULL, &pp->bounds);
}

static CoreGame* game;
static Player* player;
static bool up = false, down = false, space = false;
static SDL_Texture* btex;
static List* blist;
static int biter = 0, btime = 20;
static SDL_Texture* cloudTex;
static List* clist;
static SDL_Texture* enTex;
static List* enList;
static int spawnTime = 90, spawnIter = 0;
const static int ENMAX = 20;
static SDL_Texture* partTex;
static List* partList;
static TTF_Font* font;
static SDL_Texture* score;
static SDL_Texture* best;
static int bestScore = 0, currScore = 0;
static int iter = 0;
static SDL_Texture* ptext;
static SDL_Texture* ptextTwo;
static bool paused = true;

void RendererSetup(void) {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();;
	game = malloc(sizeof(CoreGame));
	game->window = SDL_CreateWindow("HelloFriend", 0, 0, 1000, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	game->rend = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
	game->rendTex = SDL_CreateTexture(game->rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1000, 600);
	SDL_SetRenderDrawColor(game->rend, 100, 0, 100, 255);
}

void RendererCleanup(void) {
	SDL_DestroyTexture(game->rendTex);
	SDL_DestroyRenderer(game->rend);
	SDL_DestroyWindow(game->window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Init(void) {
	RendererSetup();

	font = TTF_OpenFont("Data/Font.ttf", 40);
	
	ptext = LoadText(game->rend, font, "Press Space To Start/Shoot");
	ptextTwo = LoadText(game->rend, font, "Up/Down Arrows To Stear");
	
	player = PlayerCreate(game->rend);
	btex = LoadTexture(game->rend, "Data/fireBall.png");
	blist = ListCreate();
	cloudTex = LoadTexture(game->rend, "Data/Cloud.png");
	clist = ListCreate();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Cloud* cc = CloudCreate(j * (1000.0f / 4.0f) + rand() % 100, i * (600.0f / 4.0f) + rand() % 100);
			ListInsert(clist, cc);
		}
	}
	enTex = LoadTexture(game->rend, "Data/plane2.png");
	enList = ListCreate();
	partTex = LoadTexture(game->rend, "Data/smoke.png");
	partList = ListCreate();

	score = LoadText(game->rend, font, "Score: 0");
	best;
	FILE* ff = fopen("Data/Data.txt", "r");
	if (!ff) {
		best = LoadText(game->rend, font, "Best: 0");
	} else {
		char buffer[32] = {'0'};
		fscanf(ff, "%s", buffer);
		sscanf(buffer, "%d", &bestScore);
		char str[128] = {'\0'};
		sprintf(str, "%s %s", "BestScore:", buffer);
		best = LoadText(game->rend, font, str);
		fclose(ff);
	}
}

void Cleanup(void) {
	for (unsigned int i = 0; i < ListGetSize(clist); i++) {
		free((Cloud*)ListReturn(clist, i));	
	}
	ListDestroy(clist);
	for (unsigned int i = 0; i < ListGetSize(blist); i++) {
		free((Projectile*)ListReturn(blist, i));	
	}
	ListDestroy(blist);
	for (unsigned int i = 0; i < ListGetSize(enList); i++) {
		free((Enemy*)ListReturn(enList, i));	
	}
	ListDestroy(enList);
	for (unsigned int i = 0; i < ListGetSize(partList); i++) {
		free((Part*)ListReturn(partList, i));	
	}
	ListDestroy(partList);
	PlayerDestroy(player);
	SDL_DestroyTexture(score);
	SDL_DestroyTexture(best);
	SDL_DestroyTexture(ptext);
	SDL_DestroyTexture(ptextTwo);
	SDL_DestroyTexture(enTex);
	SDL_DestroyTexture(cloudTex);
	SDL_DestroyTexture(partTex);
	RendererCleanup();
}

void Reset(void) {
	for (unsigned int i = 0; i < ListGetSize(blist); i++) {
		free((Projectile*)ListReturn(blist, i));	
	}
	for (unsigned int i = 0; i < ListGetSize(enList); i++) {
		free((Enemy*)ListReturn(enList, i));	
	}
	for (unsigned int i = 0; i < ListGetSize(partList); i++) {
		free((Part*)ListReturn(partList, i));
	}
	ListClear(blist);
	ListClear(enList);
	ListClear(partList);
	PlayerReset(player);
	currScore = 0;
	SDL_DestroyTexture(score);
	score = LoadText(game->rend, font, "Score: 0");
	paused = true;
}

void Update(void) {
	if (paused) {
		for (unsigned int i = 0; i < ListGetSize(clist); i++) {
			CloudUpdate((Cloud*)ListReturn(clist, i), -5.0f, 0.0f);
		}
		if (space) {
			paused = false;
		}
	} else {
		PlayerUpdate(player, up, down);
		for (unsigned int i = 0; i < ListGetSize(blist); i++) {
			if (IsColliding(((Projectile*)ListReturn(blist, i))->bounds, player->bounds, 40.0f)) {
				player->dead = true;
				break;
			}
		}
		for (unsigned int i = 0; i < ListGetSize(enList); i++) {
			if (IsColliding(((Enemy*)ListReturn(enList, i))->bounds, player->bounds, 75.0f)) {
				((Enemy*)ListReturn(enList, i))->dead = true;
				player->dead = true;
			}
		}
		if (player->dead) {
			Part* pp = PartCreate(player->bounds.x, player->bounds.y, 250 + rand() % 40);
			ListInsert(partList, pp); 
			if (player->alpha < 10) {
				Reset();
			}
		}
		for (unsigned int i = 0; i < ListGetSize(blist); i++) {
			Projectile* pp = (Projectile*)ListReturn(blist, i);
			ProjUpdate(pp, player->dx, player->dy);
			if (pp->iter > pp->time) {
				ProjDestroy(pp);
				ListDelete(blist, i);
			}
		}
		biter++;
		if (space) {
			if (biter > btime) {
				float xoff = player->bounds.x;// + player->bounds.w / 2.0f;
				float yoff = player->bounds.y;// + player->bounds.h / 2.0f;
				xoff += cos(player->angle * (3.14f / 180.0f)) * 150.0f;
				yoff += sin(player->angle * (3.14f / 180.0f)) * 150.0f;
				Projectile* pp = ProjCreate(xoff, yoff, player->angle);
				ListInsert(blist, pp);
				biter = 0;
			}
		}
		for (unsigned int i = 0; i < ListGetSize(clist); i++) {
			CloudUpdate((Cloud*)ListReturn(clist, i), player->dx, player->dy);
		}
		for (unsigned int i = 0; i < ListGetSize(enList); i++) {
			EnemyUpdate((Enemy*)ListReturn(enList, i), player->dx, player->dy);
			for (unsigned j = 0; j < ListGetSize(blist); j++) {
				if (IsColliding(((Enemy*)ListReturn(enList, i))->bounds, ((Projectile*)ListReturn(blist, j))->bounds, 100.0f)) {
					((Enemy*)ListReturn(enList, i))->dead = true;
					ProjDestroy((Projectile*)ListReturn(blist, j));
					ListDelete(blist, j);
					break;	
				}
			}
			if (((Enemy*)ListReturn(enList, i))->dead) {
				Part* pp = PartCreate(((Enemy*)ListReturn(enList, i))->bounds.x + 50, ((Enemy*)ListReturn(enList, i))->bounds.y + 10, 300 + rand() % 150);
				ListInsert(partList, pp);
				if (((Enemy*)ListReturn(enList, i))->alpha <= 10) {
					EnemyDestroy(((Enemy*)ListReturn(enList, i)));
					ListDelete(enList, i);
				}
			}		
		}
		if (ListGetSize(enList) < ENMAX) {
			spawnIter++;
			if (spawnIter > spawnTime) {
				Enemy* en = EnemyCreate();
				ListInsert(enList, en);
				spawnIter = 0;
			}
		}
		for (unsigned int i = 0; i < ListGetSize(enList); i++) {
			Enemy* en = (Enemy*)ListReturn(enList, i);
			float dirx = cos(en->angle * (3.14f / 180.0f));
			float diry = sin(en->angle * (3.14f / 180.0f));
			if (FacingPlayer(dirx, diry, en->bounds.x, en->bounds.y, player->bounds.x, player->bounds.y)) {
				if (en->shoot) {
					float xoff = en->bounds.x;
					float yoff = en->bounds.y;
					xoff += cos(en->angle * (3.14f / 180.0f)) * 150.0f;
					yoff += sin(en->angle * (3.14f / 180.0f)) * 150.0f;
					Projectile* pp = ProjCreate(xoff, yoff, en->angle);
					ListInsert(blist, pp);
					en->shoot = false;
				}
			}
		}
		for (unsigned int i = 0; i < ListGetSize(partList); i++) {
			PartUpdate((Part*)ListReturn(partList, i), player->dx, player->dy);
		}
		
		iter++;
		if (iter > 60) {
			char buffer[128] = {'\0'};
			currScore++;
			sprintf(buffer, "%s %d", "Score: ", currScore);
			SDL_DestroyTexture(score);
			score = LoadText(game->rend, font, buffer);
			if (currScore > bestScore) {
				bestScore = currScore;
				memset(buffer, '\0', 128);
				sprintf(buffer, "%s %d", "BestScore: ", bestScore);
				SDL_DestroyTexture(best);
				best = LoadText(game->rend, font, buffer);
			}
			iter = 0;	
		}

	}
}

void Render(void) {
	PlayerRender(game->rend, player);
	for (unsigned int i = 0; i < ListGetSize(enList); i++) {
		EnemyRender(game->rend, enTex, (Enemy*)ListReturn(enList, i));
	}
	for (unsigned int i = 0; i < ListGetSize(blist); i++) {
		ProjRender(game->rend, btex, (Projectile*)ListReturn(blist, i));
	}
	for (unsigned int i = 0; i < ListGetSize(clist); i++) {
		CloudRender(game->rend, cloudTex, (Cloud*)ListReturn(clist, i));
	}
	for (unsigned int i = 0; i < ListGetSize(partList); i++) {
		PartRender(game->rend, partTex, (Part*)ListReturn(partList, i));
	}
	SDL_Rect tone = (SDL_Rect){50, 50, 300, 50};
	SDL_RenderCopy(game->rend, score, NULL, &tone);
	tone = (SDL_Rect){50, 100, 300, 50};
	SDL_RenderCopy(game->rend, best, NULL, &tone);
	if (paused) {
		SDL_Rect pone = (SDL_Rect){100, 200, 800, 100};
		SDL_RenderCopy(game->rend, ptext, NULL, &pone);
		pone.y += 100;
		SDL_RenderCopy(game->rend, ptextTwo, NULL, &pone);
	}
}

int main(void) {
	Init();
	
	bool running = true;
	
	const unsigned long TFPS = 1000 / 30;
	unsigned long tbegin = SDL_GetTicks();
	unsigned long ttime = SDL_GetTicks();

	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
			}
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_UP) {
					up = true;
				}
				if (e.key.keysym.sym == SDLK_DOWN) {
					down = true;
				}
				if (e.key.keysym.sym == SDLK_SPACE) {
					space = true;
				}
			}
			if (e.type == SDL_KEYUP) {
				if (e.key.keysym.sym == SDLK_UP) {
					up = false;
				}
				if (e.key.keysym.sym == SDLK_DOWN) {
					down = false;
				}
				if (e.key.keysym.sym == SDLK_SPACE) {
					space = false;
				}
			}
		}
		Update();
		SDL_SetRenderTarget(game->rend, game->rendTex);
		SDL_RenderClear(game->rend);
		Render();
		SDL_SetRenderTarget(game->rend, NULL);
		SDL_RenderCopy(game->rend, game->rendTex, NULL, NULL);
		SDL_RenderPresent(game->rend);
		tbegin = SDL_GetTicks();
		unsigned long tdelta = tbegin - ttime;
		if (tdelta < TFPS) {
			SDL_Delay(TFPS - tdelta);
		}
		ttime = SDL_GetTicks();	
	}
	
	Cleanup();
	
	FILE* sdata = fopen("Data/Data.txt", "w");

	fprintf(sdata, "%d", bestScore);

	fclose(sdata);

	return 0;
}

