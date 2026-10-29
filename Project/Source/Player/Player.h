#pragma pnce

#define PLAYER_WIDTH	(70.0f)
#define PLAYER_HEIGHT	(150.0f)
#define PLAYER_RADIUS	(35.0f)

struct PlayerData
{
	bool active;
	int handle;
	//int bulletIntervalTimer;
	int hp;
	float posX;
	float posY;
	float moveX;
	float moveY;
};

void InitPlayer();
void LoadPlayer();
void StartPlayer();
void StepPlayer();
void UpdatePlayer();
void DrawPlayer();
void FinPlayer();

PlayerData* GetPlayer();

void PlayerHitEnemy();
void PlayerHitEnemyBullet();