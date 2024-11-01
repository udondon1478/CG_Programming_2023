// g++ -O3 50balls.cpp -framework OpenGL -framework GLUT  -Wno-deprecated
// （テクスチャを使う時）g++ -O3 main11.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
//  　　OpenCVもインクルードすること #include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUTの使用

// 定数宣言
#define TILE 100   // 床頂点格子分割数
#define G -980.0   // 重力加速度
#define BALLNUM 25 // 球の個数

// 三次元ベクトル構造体
typedef struct _Vec_3D
{
   double x, y, z;
} Vec_3D;

// 関数名の宣言
void initGL();
void display();
void resize(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
double vectorNormalize(Vec_3D *vec);
Vec_3D diffVec(Vec_3D v1, Vec_3D v2);

// グローバル変数
double eDist, eDegX, eDegY;  // 視点極座標
int mX, mY, mState, mButton; // マウス座標
int winW, winH;              // ウィンドウサイズ
Vec_3D fPoint[TILE][TILE];   // 床頂点
double bottom = 0.0;         // 床面高さ
double fWidth = 1000.0;      // 床長
double fr = 30.0;            // フレームレート

double r[BALLNUM]; // 球の半径
Vec_3D p[BALLNUM]; // 球の中心座標
Vec_3D v[BALLNUM]; // 球の速度
Vec_3D c[BALLNUM]; // 球の色

// ボールの速度ベクトル

//----------ボールの速度ベクトルを矢印で可視化----------
void drawVelocityArrow(int k)
{
   // 矢印の長さ
   GLfloat arrowLength = 0.1 * r[k];
   // 矢印の幅
   GLfloat arrowWidth = 0.01 * r[k];
   // 矢印の頭の半径
   GLfloat headRadius = 0.02 * r[k];

   // 矢印の始点
   Vec_3D start = p[k];
   // 矢印の終点
   Vec_3D end;
   end.x = p[k].x + v[k].x;
   end.y = p[k].y + v[k].y;
   end.z = p[k].z + v[k].z;

   // 矢印の軸
   Vec_3D axis;
   axis.x = end.x - start.x;
   axis.y = end.y - start.y;
   axis.z = end.z - start.z;

   // 矢印の頭
   Vec_3D head;
   head.x = end.x + arrowLength * axis.x / 2.0;
   head.y = end.y + arrowLength * axis.y / 2.0;
   head.z = end.z + arrowLength * axis.z / 2.0;

   // 矢印の始点から頭までの線を描画
   glLineWidth(2.0);
   glBegin(GL_LINES);
   glVertex3d(start.x, start.y, start.z);
   glVertex3d(head.x, head.y, head.z);
   //矢印に追従する四角錐を描画
   glPushMatrix();
   glTranslated(head.x, head.y, head.z);
   glRotated(atan2(axis.z, axis.x) * 180.0 / M_PI, 0.0, 1.0, 0.0);
   glRotated(atan2(sqrt(axis.x * axis.x + axis.z * axis.z), axis.y) * 180.0 / M_PI, 0.0, 0.0, 1.0);
   glutWireCone(arrowWidth, arrowLength, 10, 10);
   glPopMatrix();
   glEnd();

   // 矢印の頭の円を描画
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(head.x, head.y, head.z);
   for (int i = 0; i < 360; i += 10)
   {
      glVertex3d(head.x + headRadius * cos(i * M_PI / 180.0), head.y + headRadius * sin(i * M_PI / 180.0), head.z);
   }
   glEnd();
}

// メイン関数
int main(int argc, char *argv[])
{
   glutInit(&argc, argv); // OpenGL/GLUTの初期化
   initGL();              // 初期設定

   glutMainLoop(); // イベント待ち無限ループ

   return 0;
}

// 初期化関数
void initGL()
{
   // 描画ウィンドウ生成
   glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // ディスプレイモードの指定
   glutInitWindowSize(800, 600);                              // ウィンドウサイズの指定
   glutCreateWindow("CG_11");                                 // ウィンドウの生成

   // コールバック関数の指定
   glutDisplayFunc(display);           // ディスプレイコールバック関数の指定
   glutReshapeFunc(resize);            // リサイズコールバック関数の指定
   glutMouseFunc(mouse);               // マウスクリックコールバック関数の指定
   glutMotionFunc(motion);             // マウスドラッグコールバック関数の指定
   glutKeyboardFunc(keyboard);         // キーボードコールバック関数の指定
   glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバック関数の指定

   // 各種設定
   glClearColor(0.0, 0.0, 0.2, 1.0); // ウィンドウクリア色の指定（RGBA）
   glEnable(GL_DEPTH_TEST);          // デプスバッファの有効化

   // 光源設定
   GLfloat col[4]; // パラメータ(RGBA)
   col[0] = 0.8;
   col[1] = 0.8;
   col[2] = 0.8;
   col[3] = 1.0;
   glEnable(GL_LIGHTING);                  // 陰影付けの有効化
   glEnable(GL_LIGHT0);                    // 光源0の有効化
   glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  // 光源0の拡散反射の強度
   glLightfv(GL_LIGHT0, GL_SPECULAR, col); // 光源0の鏡面反射の強度
   col[0] = 0.2;
   col[1] = 0.2;
   col[2] = 0.2;
   col[3] = 1.0;
   glLightfv(GL_LIGHT0, GL_AMBIENT, col); // 光源0の環境光の強度
   col[0] = 0.5;
   col[1] = 0.5;
   col[2] = 0.5;
   col[3] = 1.0;
   glEnable(GL_LIGHT1);                    // 光源1の有効化
   glLightfv(GL_LIGHT1, GL_DIFFUSE, col);  // 光源1の拡散反射の強度
   glLightfv(GL_LIGHT1, GL_SPECULAR, col); // 光源1の鏡面反射の強度
   col[0] = 0.1;
   col[1] = 0.1;
   col[2] = 0.1;
   col[3] = 1.0;
   glLightfv(GL_LIGHT1, GL_AMBIENT, col); // 光源0の環境光の強度

   // 視点関係
   eDist = 1600.0;
   eDegX = 10.0;
   eDegY = 0.0; // 視点極座標

   // 床頂点座標
   for (int j = 0; j < TILE; j++)
   {
      for (int i = 0; i < TILE; i++)
      {
         fPoint[i][j].x = -fWidth / 2.0 + i * fWidth / (TILE - 1);
         fPoint[i][j].y = bottom;
         fPoint[i][j].z = -fWidth / 2.0 + j * fWidth / (TILE - 1);
      }
   }

   // 球初期値
   srand((unsigned)time(NULL)); // 乱数初期化
   for (int k = 0; k < BALLNUM; k++)
   {
      r[k] = 25.0 * rand() / RAND_MAX + 25.0; // 半径(25.0~50.0の乱数)
      p[k].x = 0.0;
      p[k].y = bottom + r[k];
      p[k].z = 0.0;                                // 初期座標
      v[k].x = 500.0 * rand() / RAND_MAX - 250.0;  // 初期速度x成分(-250.0~255.0の乱数)
      v[k].y = 1000.0 * rand() / RAND_MAX + 500.0; // 初期速度y成分(500.0~1500.0の乱数)
      v[k].z = 500.0 * rand() / RAND_MAX - 250.0;  // 初期速度z成分(-250.0~255.0の乱数)
      c[k].x = 1.0 * rand() / RAND_MAX;            // 色R成分(0.0~1.0の乱数)
      c[k].y = 1.0 * rand() / RAND_MAX;            // 色G成分(0.0~1.0の乱数)
      c[k].z = 1.0 * rand() / RAND_MAX;            // 色B成分(0.0~1.0の乱数)
   }
}

// ディスプレイコールバック関数
void display()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ウィンドウクリア

   // ビューイング変換準備
   glMatrixMode(GL_MODELVIEW); // カレント行列の設定（モデルビュー変換行列）
   glLoadIdentity();           // 行列初期化
   // 視点極座標から直交座標へ変換
   Vec_3D eye;
   eye.x = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
   eye.y = eDist * sin(eDegX * M_PI / 180.0);
   eye.z = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);
   // 視点設定・ビューイング変換
   gluLookAt(eye.x, eye.y, eye.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点・注視点に基づくビューイング変換行列の生成
   // 光源
   GLfloat lightPos0[] = {-100.0, 200.0, 150.0, 1.0};
   glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
   GLfloat lightPos1[] = {100.0, 200.0, -150.0, 1.0};
   glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

   GLfloat col[4], spe[4], shi[1];

   //----------床パネル----------
   col[0] = 0.0;
   col[1] = 0.5;
   col[2] = 0.0;
   col[3] = 1.0; // 拡散反射係数
   spe[0] = 1.0;
   spe[1] = 1.0;
   spe[2] = 1.0;
   spe[3] = 1.0; // 鏡面反射係数
   shi[0] = 50;
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);   // 拡散反射
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);   // 環境光
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  // 鏡面反射
   glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi); // ハイライト
   glBegin(GL_QUADS);                                  // 図形開始
   for (int j = 0; j < TILE - 1; j++)
   {
      for (int i = 0; i < TILE - 1; i++)
      {
         Vec_3D v1, v2, n;
         v1 = diffVec(fPoint[i][j + 1], fPoint[i][j]);
         v2 = diffVec(fPoint[i + 1][j], fPoint[i][j]);
         n = normcrossprod(v1, v2);
         glNormal3d(n.x, n.y, n.z);
         glVertex3d(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z);                         // 頂点座標
         glVertex3d(fPoint[i][j + 1].x, fPoint[i][j + 1].y, fPoint[i][j + 1].z);             // 頂点座標
         glVertex3d(fPoint[i + 1][j + 1].x, fPoint[i + 1][j + 1].y, fPoint[i + 1][j + 1].z); // 頂点座標
         glVertex3d(fPoint[i + 1][j].x, fPoint[i + 1][j].y, fPoint[i + 1][j].z);             // 頂点座標
      }
   }
   glEnd(); // 図形終了

   //----------球----------
   for (int k = 0; k < BALLNUM; k++)
   {
      col[0] = c[k].x;
      col[1] = c[k].y;
      col[2] = c[k].z;
      col[3] = 1.0; // 拡散反射係数
      spe[0] = 1.0;
      spe[1] = 1.0;
      spe[2] = 1.0;
      spe[3] = 1.0;                                                 // 鏡面反射係数
      shi[0] = 100.0;                                               // ハイライト係数
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト
      // 配置
      glPushMatrix();                       // 変換行列の一時保存
      glTranslated(p[k].x, p[k].y, p[k].z); // 変換行列に平行移動行列を乗算
      glutSolidSphere(r[k], 36, 18);        // 球
      glPopMatrix();                        // 一時保存した行列の復帰

      // 球の位置更新
      p[k].x += v[k].x / fr;
      p[k].y += v[k].y / fr;
      p[k].z += v[k].z / fr;

      // 速度更新
      double at1 = 0.9;  // はね返り係数
      double at2 = 0.99; // 転がり抵抗
      // x成分
      if (p[k].x < -fWidth / 2.0 + r[k])
      {
         v[k].x = -v[k].x * at1;        // 速度のx成分を反転
         p[k].x = -fWidth / 2.0 + r[k]; // めり込みを解消
      }
      if (p[k].x > fWidth / 2.0 - r[k])
      {
         v[k].x = -v[k].x * at2;       // 速度のx成分を反転
         p[k].x = fWidth / 2.0 - r[k]; // めり込みを解消
      }
      // z 成分
      if (p[k].z < -fWidth / 2.0 + r[k])
      {
         v[k].z = -v[k].z * at1;        // 速度のz成分を反転
         p[k].z = -fWidth / 2.0 + r[k]; // めり込みを解消
      }
      if (p[k].z > fWidth / 2.0 - r[k])
      {
         v[k].z = -v[k].z * at1;       // 速度のz成分を反転
         p[k].z = fWidth / 2.0 - r[k]; // めり込みを解消
      }
      // y 成分
      if (p[k].y < bottom + r[k])
      {
         v[k].y = -v[k].y * at1; // 速度のx成分を反転
         p[k].y = bottom + r[k]; // めり込みを解消
         v[k].x *= at2;
         v[k].z *= at2; // 転がりによる減衰
      }

      // 天井
      /*if(p.y>300-r){
          v.y=-v.y;
          p.y=300-r;
      }*/

      // 重力加速度による速度更新
      v[k].y += G / fr;

      //----------球(影)----------
      // 色の設定
      col[0] = 0.0;
      col[1] = 0.0;
      col[2] = 0.0;
      col[3] = 0.0; // 拡散反射係数
      spe[0] = 0.0;
      spe[1] = 0;
      spe[2] = 0;
      spe[3] = 0;                                                   // 鏡面反射係数
      shi[0] = 0.0;                                                 // ハイライト係数
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト
      // 配置
      glPushMatrix();                  // 変換行列の一時保存
      glTranslated(p[k].x, 0, p[k].z); // 変換行列に平行移動行列を乗算
      glScaled(1.0, 0.01, 1.0);
      glutSolidSphere(r[k], 36, 18); // 球
      glPopMatrix();                 // 一時保存した行列の復帰
   }

   //----------ボールの速度ベクトルを矢印で可視化----------
   for (int k = 0; k < BALLNUM; k++)
   {
      drawVelocityArrow(k);
   }

   glutSwapBuffers(); // 描画実行
}

// リサイズコールバック関数
void resize(int w, int h)
{
   glViewport(0, 0, w, h); // ビューポート設定
   // 投影変換の設定
   glMatrixMode(GL_PROJECTION);                               // 変換行列の指定（設定対象は投影変換行列）
   glLoadIdentity();                                          // 行列初期化
   gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); // 透視投影ビューボリューム設定

   winW = w;
   winH = h; // ウィンドウサイズ格納
}

// タイマーコールバック関数
void timer(int value)
{
   glutPostRedisplay();                // ディスプレイイベント強制発生
   glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
   GLdouble model[16], proj[16]; // 変換行列格納用
   GLint view[4];                // ビューポート設定格納用
   GLfloat winX, winY, winZ;     // ウィンドウ座標
   GLdouble objX, objY, objZ;    // ワールド座標

   // マウスボタンが押された
   if (state == GLUT_DOWN)
   {
      mX = x;
      mY = y;
      mState = state;
      mButton = button; // マウス情報保持
      // 左ボタンのとき
      if (button == GLUT_LEFT_BUTTON)
      {
         // マウス座標からウィンドウ座標を取得
         winX = mX;
         winY = winH - mY;                                                    // x座標，y座標
         glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); // z座標

         // 変換行列，ビューポート設定取り出し
         glGetDoublev(GL_MODELVIEW_MATRIX, model); // モデルビュー変換行列
         glGetDoublev(GL_PROJECTION_MATRIX, proj); // 投影変換行列
         glGetIntegerv(GL_VIEWPORT, view);         // ビューポート設定

         // ウィンドウ座標をワールド座標に変換
         gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);

         // クリック位置が球の内部にあるかどうかを判定
         srand((unsigned)time(NULL)); // 乱数初期化
         for (int k = 0; k < BALLNUM; k++)
         {
            double distance = sqrt((objX - p[k].x) * (objX - p[k].x) + (objY - p[k].y) * (objY - p[k].y) + (objZ - p[k].z) * (objZ - p[k].z));

            if (distance <= r[k] + 500)
            {
               v[k].x = 500.0 * rand() / RAND_MAX - 250.0; // 初期速度x成分(-250.0~255.0の乱数)
               v[k].y = 300.0 * rand() / RAND_MAX + 500.0; // 初期速度y成分(500.0~1500.0の乱数)
               v[k].z = 500.0 * rand() / RAND_MAX - 250.0; // 初期速度z成分(-250.0~255.0の乱数)
            }
         }
      }
   }
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
   if (mButton == GLUT_RIGHT_BUTTON)
   {
      // マウスの移動量を角度変化量に変換
      eDegY = eDegY + (mX - x) * 0.5; // マウス横方向→水平角
      eDegX = eDegX + (y - mY) * 0.5; // マウス縦方向→垂直角
   }

   // マウス座標をグローバル変数に保存
   mX = x;
   mY = y;
}

// キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
   switch (key)
   {
   case 'q':
   case 'Q':
   case 27:
      exit(0);
   }
}

// ベクトルの外積計算
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2)
{
   Vec_3D out;

   out.x = v1.y * v2.z - v1.z * v2.y;
   out.y = v1.z * v2.x - v1.x * v2.z;
   out.z = v1.x * v2.y - v1.y * v2.x;
   vectorNormalize(&out);

   return out;
}

// ベクトルの差
Vec_3D diffVec(Vec_3D v1, Vec_3D v2)
{
   Vec_3D out;

   out.x = v1.x - v2.x;
   out.y = v1.y - v2.y;
   out.z = v1.z - v2.z;

   return out;
}

// ベクトル正規化
double vectorNormalize(Vec_3D *vec)
{
   double length;

   // ベクトル長
   length = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);

   if (length > 0)
   {
      // 正規化
      vec->x = vec->x / length;
      vec->y = vec->y / length;
      vec->z = vec->z / length;
   }
   // 戻り値はベクトル長
   return length;
}

