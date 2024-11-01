// g++ main04.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUT用

#include "myPrimitives2D.h" // 基本図形の簡易ライブラリ（第３回で作ったやつ）

#define RADIUS (300)

// 関数名の宣言
void initGL();
void display();
void resize(int w, int h);

void mouse(int button, int state, int x, int y);

void motion(int x, int y);

int mButton, mState, mX, mY; // ボタン情報記憶用

void keyboard(unsigned char key, int x, int y);
void timer(int value);
void triangle(double x, double y, double scale);
void astral(double x, double y, double scale);

// グローバル変数
int px = 0, py = 0;
int winW = 800, winH = 600;
int x2 = 0, y2 = 0;
double r = 50.0;
int N = 24;

double T = 0.0;         // 時刻
double fps = 30.0;      // フレームレート(Frame Per Second)
double dT = 1.0 / fps;  // 時間間隔（コマ送りのスピード）
double theta = 0.0;     // 角度
int scale_flag = 0;     // 拡大・縮小ON/OFF用フラグ．デフォルトはOFF
int rotate_flag = 0;    // 回転ON/OFF用フラグ．デフォルトはOFF
int translate_flag = 0; // 横揺れON/OFF用フラグ．デフォルトはOFF
int object_type = 0;    // オブジェクトの種類記憶用変数

double circle_x(int r, double theta)
{
    return r * cos(theta);
}

double circle_y(int r, double theta)
{
    return r * sin(theta);
}

// メイン関数
int main(int argc, char *argv[])
{
    glutInit(&argc, argv); // OpenGL/GLUT初期化

    initGL(); // 初期設定

    glutMainLoop(); // イベント待ち無限ループ突入

    return 0;
}

// 初期化・初期設定関数
void initGL()
{
    // 描画ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE); // ディスプレイモードの指定
    glutInitWindowSize(800, 600);                 // ウィンドウサイズの指定
    glutCreateWindow("CG_04");                    // ウィンドウの生成

    // コールバック関数の指定
    glutDisplayFunc(display);    // ディスプレイコールバック関数の指定("display")
    glutReshapeFunc(resize);     // リサイズコールバック関数の指定("resize")
    glutMouseFunc(mouse);        // マウスコールバック関数の指定("mouse")
    glutMotionFunc(motion);      // マウスモーションコールバック関数の指定("mouse")
    glutKeyboardFunc(keyboard);  // キーボードコールバック関数の指定("keyboard")
    glutTimerFunc(33, timer, 0); // タイマーコールバック関数の指定("timer")

    // 各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0); // ウィンドウクリア色の指定(RGBA)
}

// タイマーコルバック関数
void timer(int value)
{
    T += dT;                           // 時刻の更新
    theta = (30.0 / 180.0 * M_PI) * T; // 角度の更新．角速度30.0度．ラジアンでセット
    glutPostRedisplay();               // ディスプレイイベントの強制発生（即座に再描画）
    glutTimerFunc(33, timer, 0);       // タイマーの再設定(33msecごとにタイマーイベント発生)
}

// ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // ウィンドウクリア

    glMatrixMode(GL_MODELVIEW); // カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();           // 行列の初期化

        //----------背景----------
    // 画面いっぱいに四角形を描画
    glBegin(GL_QUAD_STRIP); // 図形開始
    // 下辺
    glColor3d(0.85, 0.83, 0.77);
    glVertex2d(-512, -512);
    glColor3d(0.85, 0.83, 0.77);
    glVertex2d(512, -512);
    // 中点
    glColor3d(0.13, 0.31, 0.4);
    glVertex2d(-512, 0);
    glColor3d(0.13, 0.31, 0.4);
    glVertex2d(512, 0);
    // 上辺
    glColor3d(0.02, 0.01, 0.15);
    glVertex2d(-512, 512);
    glColor3d(0.02, 0.01, 0.15);
    glVertex2d(512, 512);
    glEnd(); // 図形終了

    //----------円1----------
    glColor3d(0.0, 1.0, 0.0); // 色の指定
    glPushMatrix();           // 動き制御
    glTranslated(px, py, 0);
    if (translate_flag)
        glTranslated(10.0 * sin(theta * 10.0), 0, 0);
    if (rotate_flag)
        glRotated(30.0 * sin(theta * 2.0), 0, 0, 1);
    if (scale_flag)
        glScaled(1.0 + 0.5 * sin(theta), 1.0 + 0.5 * sin(theta), 1.0);
    glTranslated(-px, -py, 0);
    switch (object_type)
    {
    case 1:
        triangle(px, py, r); // オリジナルオブジェクト描画関数○○○の呼び出し
        break;
    case 2:
        astral(px, py, r);
        break;
    default:
        glMyWireCircle2(px, py, r, 3.0, N);
        break;
    }
    glPopMatrix(); // 動き制御終わり
    // glFlush(); //OpenGL描画命令実行
    glutSwapBuffers(); // OpenGL描画命令実行

    glFlush(); // OpenGL描画命令実行
}

// リサイズコールバック関数
void resize(int w, int h)
{
    glViewport(0, 0, w, h);                           // ビューポート設定
    glMatrixMode(GL_PROJECTION);                      // カレント行列の設定→投影変換行列
    glLoadIdentity();                                 // 行列初期化
    gluOrtho2D(-w / 2.0, w / 2.0, -h / 2.0, h / 2.0); // 二次元投影変換行列設定
    winW = w;
    winH = h;
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    // printf("mouse: button = %d, state = %d at (%d, %d)\n", button, state, x, y);
    switch (button)
    {
    case GLUT_LEFT_BUTTON: // マウス左ボタンが操作された
        printf("left");
        break;
    case GLUT_MIDDLE_BUTTON: // マウス中ボタンが操作された
        printf("middle");
        break;
    case GLUT_RIGHT_BUTTON: // マウス右ボタンが操作された
        printf("right");
        break;
    default:
        break;
    }
    printf(" button is ");
    switch (state)
    {
    case GLUT_UP: // ボタンが離された
        printf("released");
        x2 = x;
        y2 = y;
        break;
    case GLUT_DOWN: // ボタンが押された
        printf("pressed");
        break;
    default:
        break;
    }
    printf(" at (%d, %d), (%d, %d)\n", x, y, x - winW / 3, -(y - winH / 2));
    px = x - winW / 2;
    py = winH / 2 - y;
    glutPostRedisplay(); // 再描画命令

    mButton = button;
    mState = state;
    mX = x;
    mY = y;
}

// マウスドラッグコールバック関数(x,y:座標)
void motion(int x, int y)
{
    if (mButton == GLUT_LEFT_BUTTON)
    {
        printf("Dragging at (%d, %d)\n", x, y);
        r = sqrt((x - mX) * (x - mX) + (y - mY) * (y - mY));
        glutPostRedisplay(); // 再描画命令
    }
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        printf("Dragging at (%d, %d)\n", x, y);
    }

    glutPostRedisplay(); // 再描画命令
}

// キーボードコールバック関数
void keyboard(unsigned char key, int x, int y)
{
    printf("key = %d at (%d, %d)\n", key, x, y);
    switch (key)
    {
    case 27:     // [ESC]キー
    case 'q':    // [q]キー
    case 'Q':    // [Q]キー
        exit(0); // プログラム終了
    case '3':
        N = 3;
        break;
    case '4':
        N = 4;
        break;
    case '5':
        N = 5;
        break;
    case '6':
        N = 6;
        break;
    case '7':
        N = 12;
        break;
    case '8':
        N = 24;
        break;
    case '9':
        N = 48;
        break;
    case '0':
        scale_flag = (scale_flag + 1) % 2; // トグルスイッチ
        break;
    case '1':
        rotate_flag = (rotate_flag + 1) % 2; // トグルスイッチ
        break;
    case '2':
        translate_flag = (translate_flag + 1) % 2; // トグルスイッチ
        break;
    case 't':
        object_type = 1;
        break;
    case 'T':
        object_type = 1;
        break;
    case 'a':
        object_type = 2;
        break;
    case 'A':
        object_type = 2;
        break;
    case 'c':
        object_type = 0;
        break;
    case 'C': 
        object_type = 0;
        break;
    default:
        break;
    }
}

void triangle(double x, double y, double scale)
{
    glPushMatrix(); // 指定した位置(x,y)へ移動
    glTranslated(x, y, 0);
    glScaled(scale, scale, 1.0);
    glBegin(GL_TRIANGLES); // 図形開始
    // 幅1高さ0.866の三角形
    glColor3d(1.0, 1.0, 0.0); // 色の指定
    glVertex2d(0, 0.6);       // 頂点 0
    glColor3d(1.0, 0.0, 0.0); // 色の指定
    glVertex2d(-0.5, -0.25);  // 頂点 1
    glColor3d(0.0, 1.0, 0.0); // 色の指定
    glVertex2d(0.5, -0.25);   // 頂点 2

    // 幅1高さ0.866の三角形
    glColor3d(0.0, 0.0, 1.0); // 色の指定
    glVertex2d(-0.5, 0.3);    // 頂点 3
    glColor3d(1.0, 0.0, 1.0); // 色の指定
    glVertex2d(0, -0.5666);   // 頂点 4
    glColor3d(0.0, 1.0, 1.0); // 色の指定
    glVertex2d(0.5, 0.3);     // 頂点 5
    glEnd();                  // 図形終了
    glPopMatrix();
}

void astral(double x, double y, double scale){
    glPushMatrix();
    glTranslated(x, y, 0);
    glScaled(scale/100.0, scale/100.0, 1.0);
    double vertices[48][2];
    double theta_a = 0.0;
    for (int i = 0; i < 48; i++) {
        vertices[i][0] = circle_x(RADIUS, theta_a);
        vertices[i][1] = circle_y(RADIUS, theta_a);
        theta_a += M_PI / 24.0;
    }

    //第一層    glBegin(GL_TRIANGLES);
    glColor3d(0.82, 0.28, 0.40);
    for (int i = 11; i < 14; i++) {
        glVertex2d(vertices[i][0], vertices[i][1]);
    }
    glEnd();

    //第二層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.86, 0.33, 0.44);
    glVertex2d(vertices[13][0], vertices[13][1]);
    glVertex2d(vertices[14][0], vertices[14][1]);
    glVertex2d(vertices[11][0], vertices[11][1]);
    glVertex2d(vertices[10][0], vertices[10][1]);
    glEnd();

    //第三層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.90, 0.43, 0.47);
    glVertex2d(vertices[14][0], vertices[14][1]);
    glVertex2d(vertices[15][0], vertices[15][1]);
    glVertex2d(vertices[10][0], vertices[10][1]);
    glVertex2d(vertices[9][0], vertices[9][1]);
    glEnd();

    //第四層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.91, 0.53, 0.48);
    glVertex2d(vertices[15][0], vertices[15][1]);
    glVertex2d(vertices[16][0], vertices[16][1]);
    glVertex2d(vertices[9][0], vertices[9][1]);
    glVertex2d(vertices[8][0], vertices[8][1]);
    glEnd();

    //第五層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.92, 0.60, 0.50);
    glVertex2d(vertices[16][0], vertices[16][1]);
    glVertex2d(vertices[17][0], vertices[17][1]);
    glVertex2d(vertices[8][0], vertices[8][1]);
    glVertex2d(vertices[7][0], vertices[7][1]);
    glEnd();

    //第六層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.95, 0.68, 0.55);
    glVertex2d(vertices[17][0], vertices[17][1]);
    glVertex2d(vertices[18][0], vertices[18][1]);
    glVertex2d(vertices[7][0], vertices[7][1]);
    glVertex2d(vertices[6][0], vertices[6][1]);
    glEnd();

    //第七層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.96, 0.73, 0.60);
    glVertex2d(vertices[18][0], vertices[18][1]);
    glVertex2d(vertices[19][0], vertices[19][1]);
    glVertex2d(vertices[6][0], vertices[6][1]);
    glVertex2d(vertices[5][0], vertices[5][1]);
    glEnd();

    //第八層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.96, 0.78, 0.66);
    glVertex2d(vertices[19][0], vertices[19][1]);
    glVertex2d(vertices[20][0], vertices[20][1]);
    glVertex2d(vertices[5][0], vertices[5][1]);
    glVertex2d(vertices[4][0], vertices[4][1]);
    glEnd();

    //第九層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.97, 0.83, 0.72);
    glVertex2d(vertices[20][0], vertices[20][1]);
    glVertex2d(vertices[21][0], vertices[21][1]);
    glVertex2d(vertices[4][0], vertices[4][1]);
    glVertex2d(vertices[3][0], vertices[3][1]);
    glEnd();

    //第十層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.97, 0.87, 0.78);
    glVertex2d(vertices[21][0], vertices[21][1]);
    glVertex2d(vertices[22][0], vertices[22][1]);
    glVertex2d(vertices[3][0], vertices[3][1]);
    glVertex2d(vertices[2][0], vertices[2][1]);
    glEnd();

    //第十一層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.96, 0.88, 0.79);
    glVertex2d(vertices[22][0], vertices[22][1]);
    glVertex2d(vertices[23][0], vertices[23][1]);
    glVertex2d(vertices[2][0], vertices[2][1]);
    glVertex2d(vertices[1][0], vertices[1][1]);
    glEnd();

    //第十二層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.91, 0.88, 0.81);
    glVertex2d(vertices[23][0], vertices[23][1]);
    glVertex2d(vertices[24][0], vertices[24][1]);
    glVertex2d(vertices[1][0], vertices[1][1]);
    glVertex2d(vertices[0][0], vertices[0][1]);
    glEnd();

    //第十三層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.78, 0.83, 0.79);
    glVertex2d(vertices[24][0], vertices[24][1]);
    glVertex2d(vertices[25][0], vertices[25][1]);
    glVertex2d(vertices[0][0], vertices[0][1]);
    glVertex2d(vertices[47][0], vertices[47][1]);
    glEnd();

    //第十四層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.62, 0.79, 0.77);
    glVertex2d(vertices[25][0], vertices[25][1]);
    glVertex2d(vertices[26][0], vertices[26][1]);
    glVertex2d(vertices[47][0], vertices[47][1]);
    glVertex2d(vertices[46][0], vertices[46][1]);
    glEnd();

    //第十五層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.47, 0.75, 0.74);
    glVertex2d(vertices[26][0], vertices[26][1]);
    glVertex2d(vertices[27][0], vertices[27][1]);
    glVertex2d(vertices[46][0], vertices[46][1]);
    glVertex2d(vertices[45][0], vertices[45][1]);
    glEnd();

    //第十六層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.30, 0.64, 0.7);
    glVertex2d(vertices[27][0], vertices[27][1]);
    glVertex2d(vertices[28][0], vertices[28][1]);
    glVertex2d(vertices[45][0], vertices[45][1]);
    glVertex2d(vertices[44][0], vertices[44][1]);
    glEnd();

    //第十七層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.23, 0.52, 0.62);
    glVertex2d(vertices[28][0], vertices[28][1]);
    glVertex2d(vertices[29][0], vertices[29][1]);
    glVertex2d(vertices[44][0], vertices[44][1]);
    glVertex2d(vertices[43][0], vertices[43][1]);
    glEnd();

    //第十八層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.14, 0.33, 0.45);
    glVertex2d(vertices[29][0], vertices[29][1]);
    glVertex2d(vertices[30][0], vertices[30][1]);
    glVertex2d(vertices[43][0], vertices[43][1]);
    glVertex2d(vertices[42][0], vertices[42][1]);
    glEnd();

    //第十九層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.07, 0.17, 0.29);
    glVertex2d(vertices[30][0], vertices[30][1]);
    glVertex2d(vertices[31][0], vertices[31][1]);
    glVertex2d(vertices[42][0], vertices[42][1]);
    glVertex2d(vertices[41][0], vertices[41][1]);
    glEnd();

    //第二十層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.02, 0.03, 0.14);
    glVertex2d(vertices[31][0], vertices[31][1]);
    glVertex2d(vertices[32][0], vertices[32][1]);
    glVertex2d(vertices[41][0], vertices[41][1]);
    glVertex2d(vertices[40][0], vertices[40][1]);
    glEnd();

    //第二十一層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.02, 0.03, 0.14);
    glVertex2d(vertices[32][0], vertices[32][1]);
    glVertex2d(vertices[33][0], vertices[33][1]);
    glVertex2d(vertices[40][0], vertices[40][1]);
    glVertex2d(vertices[39][0], vertices[39][1]);
    glEnd();

    //第二十二層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.007, 0.003, 0.12);
    glVertex2d(vertices[33][0], vertices[33][1]);
    glVertex2d(vertices[34][0], vertices[34][1]);
    glVertex2d(vertices[39][0], vertices[39][1]);
    glVertex2d(vertices[38][0], vertices[38][1]);
    glEnd();

    //第二十三層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.003, 0.003, 0.11);
    glVertex2d(vertices[34][0], vertices[34][1]);
    glVertex2d(vertices[35][0], vertices[35][1]);
    glVertex2d(vertices[38][0], vertices[38][1]);
    glVertex2d(vertices[37][0], vertices[37][1]);
    glEnd();

    //第二十四層
    glBegin(GL_TRIANGLES);
    glColor3d(0.0, 0.0, 0.09);
    glVertex2d(vertices[35][0], vertices[35][1]);
    glVertex2d(vertices[36][0], vertices[36][1]);
    glVertex2d(vertices[37][0], vertices[37][1]);
    glEnd();


    //白い円を描画、半径はRADIUS/2
    glBegin(GL_POLYGON);
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 0; i < 48; i++)
    {
        glVertex2d(vertices[i][0] / 2, vertices[i][1] / 2);
    }
    glEnd();
    glPopMatrix();
}