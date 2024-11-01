// g++ -O3 main12.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUTの使用

// マウスをクリックしている間、周囲のボールを引き寄せるプログラム
// ボールの数はBALLNUMで指定
// 引力の中心から離れれば離れるほど、引力が弱くなるようにする
// 引力の発生点はマウスカーソルがドラッグした位置にする

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
	double x, y, z;
} Vec_3D;

// 定数宣言
#define TILE 50	   // 床頂点格子分割数
#define G -980.0   // 重力加速度
#define BALLNUM 50 // 球の個数
// ボールを引き寄せる半径
#define ATTRACT_RADIUS 100.0
// ボールを引き寄せる力
#define ATTRACT_POWER 10

// 関数名の宣言
void init(void);
void display(void);
void idle(void);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void drawBall(void);
void drawFloor(void);
void drawString(char *str, int w, int h, int x0, int y0);
void timer(int value);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
double vectorNormalize(Vec_3D *vec);
Vec_3D vectorDiff(Vec_3D v1, Vec_3D v2);

// グローバル変数
double eDist, eDegX, eDegY;			 // 視点の位置
int mX, mY;							 // マウスの位置
int mState, mButton;				 // マウスの状態
int winW, winH;						 // ウィンドウの大きさ
int attract_x, attract_y, attract_z; // 引力の発生点
int attract_flag;					 // 引力の発生点を決めるフラグ
int attract_radius;					 // 引力の半径
int attract_power;					 // 引力の強さ
Vec_3D fPoint[TILE][TILE];			 // 床の頂点の位置
Vec_3D fNormal[TILE][TILE];			 // 床の頂点の法線ベクトル
double bottom = 0.0;				 // 床の高さ
double fWidth = 1000.0;				 // 床の幅
double framerate = 30.0;			 // フレームレート

float dt = 0.01; // 時間刻み幅

// Catalina+retina
double rDisplay = 1.0;

// ボールの構造体
typedef struct _Ball
{
	Vec_3D pos; // 位置
	Vec_3D vel; // 速度
	Vec_3D acc; // 加速度
	double r;	// 半径
	double m;	// 質量
} Ball;

// ボールの配列
Ball ball[BALLNUM];

// メイン関数
int main(int argc, char *argv[])
{
	// 初期化
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800 * rDisplay, 800 * rDisplay);
	glutCreateWindow("gravity");

	// コールバック関数の登録
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(100, timer, 0);

	// 初期化
	init();

	// メインループ
	glutMainLoop();

	return 0;
}

// 初期化
void init(void)
{
	int i, j;
	double x, y, z;

	// 乱数の初期化
	srand((unsigned int)time(NULL));

	// 視点の初期化
	eDist = 1000.0;
	eDegX = 0.0;
	eDegY = 0.0;

	// マウスの初期化
	mX = 0;
	mY = 0;
	mState = GLUT_UP;
	mButton = GLUT_LEFT_BUTTON;

	// ウィンドウの大きさの初期化
	winW = 800;
	winH = 800;

	// 引力の発生点の初期化
	attract_x = 0;
	attract_y = 0;
	attract_z = 0;
	attract_flag = 0;
	attract_radius = ATTRACT_RADIUS;
	attract_power = ATTRACT_POWER;

	// 床の頂点の位置の初期化
	for (i = 0; i < TILE; i++)
	{
		for (j = 0; j < TILE; j++)
		{
			x = fWidth * ((double)i / (double)(TILE - 1) - 0.5);
			z = fWidth * ((double)j / (double)(TILE - 1) - 0.5);
			y = bottom;
			fPoint[i][j].x = x;
			fPoint[i][j].y = y;
			fPoint[i][j].z = z;
		}
	}

	// 床の頂点の法線ベクトルの初期化
	for (i = 0; i < TILE; i++)
	{
		for (j = 0; j < TILE; j++)
		{
			fNormal[i][j].x = 0.0;
			fNormal[i][j].y = 1.0;
			fNormal[i][j].z = 0.0;
		}
	}

	// ボールの初期化
	initBall();
}

// ボールの初期化
void initBall(void)
{
	int i;
	for (i = 0; i < BALLNUM; i++)
	{
		ball[i].pos.x = 0.0;
		ball[i].pos.y = 0.0;
		ball[i].pos.z = 0.0;
		ball[i].vel.x = 0.0;
		ball[i].vel.y = 0.0;
		ball[i].vel.z = 0.0;
		ball[i].acc.x = 0.0;
		ball[i].acc.y = 0.0;
		ball[i].acc.z = 0.0;
		ball[i].r = 10.0;
		ball[i].m = 1.0;
	}
}

// ディスプレイ
void display(){
	int i;
	// バッファのクリア
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// モデルビュー変換行列の初期化
	glLoadIdentity();

	// 視点の移動
	glTranslated(0.0, 0.0, -eDist);
	glRotated(eDegX, 1.0, 0.0, 0.0);
	glRotated(eDegY, 0.0, 1.0, 0.0);

	// ボールの描画
	drawBall();

	// 床の描画
	drawFloor();

	// バッファの入れ替え
	glutSwapBuffers();
}

// アイドル
void idle(void){
	int i, j, k;
	double dist, force, force_x, force_y, force_z;
	Vec_3D vec;

	// ボールの位置の更新
	for (i = 0; i < BALLNUM; i++)
	{
		// 重力の計算
		ball[i].acc.x = 0.0;
		ball[i].acc.y = G;
		ball[i].acc.z = 0.0;

		// 床との衝突判定
		for (j = 0; j < TILE; j++)
		{
			for (k = 0; k < TILE; k++)
			{
				// ボールと床の距離
				dist = (ball[i].pos.x - fPoint[j][k].x) * fNormal[j][k].x + (ball[i].pos.y - fPoint[j][k].y) * fNormal[j][k].y + (ball[i].pos.z - fPoint[j][k].z) * fNormal[j][k].z;

				// ボールが床より上にある場合
				if (dist > ball[i].r)
				{
					continue;
				}

				// ボールが床より下にある場合
				if (dist < -ball[i].r)
				{
					continue;
				}

				// ボールが床と接触している場合
				if (dist >= -ball[i].r && dist <= ball[i].r)
				{
					// ボールの速度と床の法線ベクトルの内積
					double dot = ball[i].vel.x * fNormal[j][k].x + ball[i].vel.y * fNormal[j][k].y + ball[i].vel.z * fNormal[j][k].z;

					// ボールが床にめり込んでいる場合
					if (dot < 0.0)
					{
						// ボールの位置の修正
						ball[i].pos.x -= dist * fNormal[j][k].x;
						ball[i].
						
							pos.y -= dist * fNormal[j][k].y;
						ball[i].pos.z -= dist * fNormal[j][k].z;

						// ボールの速度の修正
						ball[i].vel.x -= dot * fNormal[j][k].x;
						ball[i].vel.y -= dot * fNormal[j][k].y;
						ball[i].vel.z -= dot * fNormal[j][k].z;
					}
				}
			}
		}

		// ボールの位置の更新
		ball[i].pos.x += ball[i].vel.x * dt;
		ball[i].pos.y += ball[i].vel.y * dt;
		ball[i].pos.z += ball[i].vel.z * dt;

		// ボールの速度の更新
		ball[i].vel.x += ball[i].acc.x * dt;
		ball[i].vel.y += ball[i].acc.y * dt;
		ball[i].vel.z += ball[i].acc.z * dt;

		// ボールの速度の減衰
		ball[i].vel.x *= 0.99;
		ball[i].vel.y *= 0.99;
		ball[i].vel.z *= 0.99;
	}

	// ボール同士の引力の計算
	for (i = 0; i < BALLNUM; i++)
	{
		for (j = 0; j < BALLNUM; j++)
		{
			if (i == j)
			{
				continue;
			}

			// ボール間の距離
			vec = vectorDiff(ball[i].pos, ball[j].pos);
			dist = vectorNormalize(&vec);

			// ボール間の距離が引力の半径より大きい場合
			if (dist > attract_radius)
			{
				continue;
			}

			// ボール間の距離が引力の半径より小さい場合
			if (dist < attract_radius)
			{
				// ボール間の距離が0の場合
				if (dist == 0.0)
				{
					continue;
				}

				// ボール間の距離が0より大きい場合
				if (dist > 0.0)
				{
					// ボール間の距離の2乗
					double dist2 = dist * dist;

					// ボール間の距離の3乗
					double dist3 = dist2 * dist;

					// ボール間の距離の4乗
					double dist4 = dist2 * dist2;

					// ボール間の距離の5乗
					double dist5 = dist3 * dist2;

					// ボール間の距離の6乗
					double dist6 = dist3 * dist3;

					// ボール間の距離の7乗
					double dist7 = dist3 * dist4;

					// ボール間の距離の8乗
					double dist8 = dist4 * dist4;

					// ボール間の距離の9乗
					double dist9 = dist4 * dist5;

					// ボール間の距離の10乗
					double dist10 = dist5 * dist5;

					// ボール間の距
					
