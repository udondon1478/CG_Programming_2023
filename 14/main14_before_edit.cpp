/* コンパイルコマンド
g++ moyac.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GLUT/glut.h>        //OpenGL/GLUT
#include <math.h>             //数学関数
#include <opencv2/opencv.hpp> //OpenCV

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

// ランダムに餌を描画し、パックマンが触れたら消えるようにする

typedef struct _Food
{
    double x, y, z;
} Food;

// 関数名の宣言
void initGL();
void display0();                                // ウィンドウ0用ディスプレイコールバック関数
void reshape0(int w, int h);                    // ウィンドウ0用リシェイプコールバック関数
void display1();                                // ウィンドウ1用ディスプレイコールバック関数
void reshape1(int w, int h);                    // ウィンドウ1用リシェイプコールバック関数
void timer(int value);                          // タイマーコールバック関数
void keyboard(unsigned char key, int x, int y); // キーボードコールバック関数
void scene();                                   // シーン描画関数
void drawString(char *str, int w, int h, int x0, int y0);
void drawFood();
void isCollided(Food food);

// グローバル変数
int winW[2], winH[2];            // ウィンドウサイズ
double fr = 30.0;                // フレームレート
Vec_3D objPos = {0.0, 0.0, 0.0}; // 物体座標
int winID[2];                    // ウィンドウID
int IMGNUM = 0;                  // 画像枚数
int direction = 0;               // 0:上 1:下 2:左 3:右

// 餌が食べられたかどうかのフラグ
int food_flag = 1;
int food_flag2 = 1;
// Food構造体を用いてbaitを定義
Food bait;
// パックマンと餌が接触したかどうかの判定
void isCollided(Food food)
{
    double distance = sqrt(pow(objPos.x - food.x, 2) + pow(objPos.y - food.y, 2));
    if (distance < 20)
    {
        food_flag = 1;
        food_flag2 = 1;
    }
}

// Catalina+retina
double rDisp = 1.0;

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL)); // 乱数初期化

    //----------OpenGL/GLUTの初期化----------
    glutInit(&argc, argv);

    //----------初期設定----------
    initGL();

    //----------イベント待ち無限ループ----------
    glutMainLoop();

    return 0;
}

// 初期設定関数
void initGL()
{
    // ウィンドウ0生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // ディスプレイ表示モード指定
    glutInitWindowSize(640, 480);                              // ウィンドウサイズの指定（640×480）
    glutInitWindowPosition(0, 0);                              // ウィンドウ位置の指定（0,0）
    winID[0] = glutCreateWindow("CG0");                        // ウィンドウ生成

    // コールバック関数指定
    glutDisplayFunc(display0);          // ディスプレイコールバック関数（"display0"）
    glutReshapeFunc(reshape0);          // リサイズコールバック関数（"reshape0"）
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバック関数（"timer"）
    glutKeyboardFunc(keyboard);         // キーボードコールバック関数

    // ウィンドウ 1 生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // ディスプレイ表示モード指定
    glutInitWindowSize(640, 480);                              // ウィンドウサイズの指定（640×480）
    glutInitWindowPosition(640, 0);                            // ウィンドウ位置の指定（640,0）
    winID[1] = glutCreateWindow("CG1");                        // ウィンドウ生成
    // コールバック関数指定
    glutDisplayFunc(display1);  // ディスプレイコールバック関数（"display1"）
    glutReshapeFunc(reshape1);  // リサイズコールバック関数（"reshape1"）
    glutKeyboardFunc(keyboard); // キーボードコールバック関数
    // 各ウィンドウ共通の設定
    for (int i = 0; i < 2; i++)
    {
        glutSetWindow(winID[i]); // 設定対象ウィンドウ選択
        // 各種設定
        glClearColor(0.0, 0.0, 0.2, 1.0);                  // ウィンドウクリア色の指定（RGBA 値）
        glEnable(GL_DEPTH_TEST);                           // デプスバッファ有効化
        glEnable(GL_NORMALIZE);                            // ベクトル正規化有効化
        glEnable(GL_BLEND);                                // アルファブレンディング有効化
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ブレンディング関数指定
        glEnable(GL_ALPHA_TEST);                           // アルファテスト有効化
        glAlphaFunc(GL_GREATER, 0.1);                      // アルファ値比較関数の指定
        // テクスチャ準備
        cv::Mat textureImage;
        // テクスチャ 0

        textureImage = cv::imread("pac.png", cv::IMREAD_UNCHANGED); // 画像読み込み
        glBindTexture(GL_TEXTURE_2D, 0);                            // テクスチャ 0 番呼び出し
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.cols, textureImage.rows, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, textureImage.data); // 画像をテクスチャに割り当て
    }
}

void scene()
{
    // 物体表示
    glEnable(GL_TEXTURE_2D);                    // テクスチャマッピング有効化
    glColor4d(1.0, 1.0, 1.0, 1.0);              // 物体色
    glBindTexture(GL_TEXTURE_2D, 0);            // テクスチャ 0 番呼び出し
    glPushMatrix();                             // 変換行列の一時保存
    glTranslated(objPos.x, objPos.y, objPos.z); // 平行移動
                                                // 角度を変更
    switch (direction)
    {
    case 1:
        glRotated(90, 0.0, 0.0, 1.0);
        break;
    case 2:
        glRotated(180, 0.0, 0.0, 1.0);
        break;
    case 3:
        glRotated(270, 0.0, 0.0, 1.0);
        break;
    case 4:
        glRotated(0, 0.0, 0.0, 1.0);
        break;
    }
    glScaled(40.0, 40.0, 1.0);   // 拡大縮小
    glBegin(GL_QUADS);           // 図形開始（四角形）
    glTexCoord2d(0.0, 0.0);      // テクスチャ座標
    glVertex3d(-0.5, 0.5, 0.0);  // 頂点座標
    glTexCoord2d(1.0, 0.0);      // テクスチャ座標
    glVertex3d(0.5, 0.5, 0.0);   // 頂点座標
    glTexCoord2d(1.0, 1.0);      // テクスチャ座標
    glVertex3d(0.5, -0.5, 0.0);  // 頂点座標
    glTexCoord2d(0.0, 1.0);      // テクスチャ座標
    glVertex3d(-0.5, -0.5, 0.0); // 頂点座標
    glEnd();                     // 図形終了
    glPopMatrix();               // 一時保存した変換行列の復帰
    glDisable(GL_TEXTURE_2D);    // テクスチャマッピング無効化
}

void drawFood()
{
    // Draw food at random positions
    // 餌が食べられたら、新しい餌をランダムな位置に描画する

    // 再度呼び出されるまで同じ位置に留まり続ける
    bait.x = rand() % 640;
    bait.y = rand() % 480;
    bait.z = 0.0;

    // Draw the food
    glDisable(GL_TEXTURE_2D);          // Disable texture mapping
    glColor4d(1.0, 0.0, 0.0, 1.0);     // Food color (red)
    glPushMatrix();                    // Save the transformation matrix
    glTranslated(bait.x, bait.y, 0.0); // Translate to the food position
    glScaled(20.0, 20.0, 1.0);         // Scale the food size
    glBegin(GL_QUADS);                 // Start drawing a quadrilateral
    glVertex3d(-0.5, 0.5, 0.0);        // Top-left vertex
    glVertex3d(0.5, 0.5, 0.0);         // Top-right vertex
    glVertex3d(0.5, -0.5, 0.0);        // Bottom-right vertex
    glVertex3d(-0.5, -0.5, 0.0);       // Bottom-left vertex
    glEnd();                           // End drawing
    glPopMatrix();                     // Restore the transformation matrix
    glEnable(GL_TEXTURE_2D);           // Enable texture mapping
    
}

// ディスプレイコールバック関数
void display0()
{
    glMatrixMode(GL_MODELVIEW);                         // カレント行列の指定（モデルビュー変換行列を指定）
    glLoadIdentity();                                   // 行列の初期化
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面消去
    // シーン構築
    scene();

    // 餌がパックマンと接触したら、新しい餌をランダムな位置に描画する
    isCollided(bait);
    if (food_flag == 1)
    {
        drawFood();
        food_flag = 0;
    }

    glutSwapBuffers(); // 描画実行
}

// ディスプレイコールバック関数
void display1()
{
    glMatrixMode(GL_MODELVIEW);                         // カレント行列の指定（モデルビュー変換行列を指定）
    glLoadIdentity();                                   // 行列の初期化
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面消去
    // シーン構築
    scene();

    isCollided(bait);
    if (food_flag2 == 1)
    {
        drawFood();
        food_flag2 = 0;
    }

    glutSwapBuffers(); // 描画実行
}

// リシェイプコールバック関数
void reshape0(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ビューポート設定
    glMatrixMode(GL_PROJECTION);            // カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();                       // 行列の初期化
    gluOrtho2D(-w, 0, -h / 2.0, h / 2.0);   // 二次元座標の設定
    winW[0] = w;
    winH[0] = h;
}

// リシェイプコールバック関数
void reshape1(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ビューポート設定
    glMatrixMode(GL_PROJECTION);            // カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();                       // 行列の初期化
    gluOrtho2D(0.0, w, -h / 2.0, h / 2.0);  // 二次元座標の設定
    winW[1] = w;
    winH[1] = h;
}

// キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
    // 十字キーでパックマンの移動方向を変更
    case 'w':
    case 'W':
        direction = 1;
        break;
    case 's':
    case 'S':
        direction = 3;
        break;
    case 'a':
    case 'A':
        direction = 2;
        break;
    case 'd':
    case 'D':
        direction = 4;
        break;
    default:
        break;
    }
}

// タイマーコールバック関数
void timer(int value)
{
    glutSetWindow(winID[1]);            // 設定対象ウィンドウ選択
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutSetWindow(winID[0]);            // 設定対象ウィンドウ選択
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定
    IMGNUM++;
    if (IMGNUM > 9)
    {
        IMGNUM = 0;
    }

    for (int i = 0; i < 2; i++)

    {
        glutSetWindow(winID[i]); // 設定対象ウィンドウ選択
        // 各種設定
        glClearColor(0.0, 0.0, 0.2, 1.0);                  // ウィンドウクリア色の指定（RGBA 値）
        glEnable(GL_DEPTH_TEST);                           // デプスバッファ有効化
        glEnable(GL_NORMALIZE);                            // ベクトル正規化有効化
        glEnable(GL_BLEND);                                // アルファブレンディング有効化
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ブレンディング関数指定
        glEnable(GL_ALPHA_TEST);                           // アルファテスト有効化
        glAlphaFunc(GL_GREATER, 0.1);                      // アルファ値比較関数の指定
        // テクスチャ準備
        cv::Mat textureImage;
        // テクスチャ 0
        char filename[256];
        sprintf(filename, "pac%d.png", IMGNUM);
        textureImage = cv::imread(filename, cv::IMREAD_UNCHANGED); // 画像読み込み
        glBindTexture(GL_TEXTURE_2D, 0);                           // テクスチャ 0 番呼び出し
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.cols, textureImage.rows, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, textureImage.data); // 画像をテクスチャに割り当て
    }

    // 初期状態を基準として、移動する方向に角度と座標を変更
    switch (direction)
    {
    case 1:
        objPos.y += 10;
        break;
    case 2:
        objPos.x -= 10;
        break;
    case 3:
        objPos.y -= 10;
        break;
    case 4:
        objPos.x += 10;
        break;
    }

    // 画面外に出たら反対側に移動
    if (objPos.x > winW[0])
    {
        objPos.x = -winW[1];
    }
    else if (objPos.x < -winW[1])
    {
        objPos.x = winW[0];
    }
    else if (objPos.y > winH[0] / 2)
    {
        objPos.y = -winH[0] / 2;
    }
    else if (objPos.y < -winH[0] / 2)
    {
        objPos.y = winH[0] / 2;
    }

    // food_flagをコンソールに表示
    printf("%d\n", food_flag);
}

void drawString(char *str, int w, int h, int x0, int y0)
{
    // 画面上に文字列を描画する関数
    // str: 文字列
    // w, h: ウィンドウサイズ
    // x0, y0: 文字列の左下の座標
    int i;
    glDisable(GL_LIGHTING);      // ライティングの無効化
    glDisable(GL_DEPTH_TEST);    // デプスバッファの無効化
    glMatrixMode(GL_PROJECTION); // 行列モードの設定
    glPushMatrix();              // 行列の保存
    glLoadIdentity();            // 単位行列の設定
    gluOrtho2D(0, w, 0, h);      // 正射影の設定
    glMatrixMode(GL_MODELVIEW);  // 行列モードの設定
    glPushMatrix();              // 行列の保存
    glLoadIdentity();            // 単位行列の設定
    glColor3d(1.0, 1.0, 1.0);    // 色の設定
    glRasterPos2i(x0, y0);       // ラスタ位置の設定
    for (i = 0; str[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]); // 文字の描画
    }
    glPopMatrix();               // 行列の復元
    glMatrixMode(GL_PROJECTION); // 行列モードの設定
    glPopMatrix();               // 行列の復元
    glMatrixMode(GL_MODELVIEW);  // 行列モードの設定
    glEnable(GL_DEPTH_TEST);     // デプスバッファの有効化
    glEnable(GL_LIGHTING);       // ライティングの有効化
}