// g++ -O3 main15.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h>        //OpenGL/GLUT
#include <opencv2/opencv.hpp> //OpenCV

#define TILE 50 // 床頂点数

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
Vec_3D crossProduct(Vec_3D vec1, Vec_3D vec2);                              // 外積計算
double innerProduct(Vec_3D v1, Vec_3D v2);                                  // 内積計算
Vec_3D diffVec(Vec_3D vec1, Vec_3D vec2);                                   // ベクトルの差
Vec_3D addVec(Vec_3D vec1, Vec_3D vec2);                                    // ベクトルの和
Vec_3D multiVec(Vec_3D v, double c);                                        // ベクトルの定数倍
double vectorLen(Vec_3D vec);                                               // ベクトルの長さ
Vec_3D vectorNormalize(Vec_3D vec);                                         // ベクトルの正規化（単位ベクトル化）
void glMySolidCircle(double cx, double cy, double cz, double r, int p);     // 塗り潰し円
void glMyCylinder(double top, double bottom, double height, double slices); // 円柱
void drawTetsujin();
void drawArm();

// グローバル変数
Vec_3D fPoint[TILE][TILE];   // 床頂点
double fWidth = 5000.0;      // 床長
double eDist, eDegX, eDegY;  // 視点極座標
int winW, winH;              // ウィンドウサイズ
int mButton, mState, mX, mY; // マウス情報
double f = 30.0;             // フレームレート
double theta = 0.0;          // メリーゴーランドの回転角度

int pole_h[8];
int pole_h_flag[8];
int h_num = -100;

// Catalina+retina
double rDisp = 1.0;

// メイン関数
int main(int argc, char *argv[])
{
    glutInit(&argc, argv); // OpenGL,GLUTの初期化

    initGL(); // 初期設定

    glutMainLoop(); // イベント待ち無限ループ突入
    for (int i = 0; i < 8; i++)
    {
        h_num += 100;
        pole_h[i] = h_num;
    }

    for (int i = 0; i < 8; i++)
    {
        pole_h_flag[i] = 0;
    }

    return 0;
}

// 初期設定関数
void initGL()
{
    // ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // ディスプレイ表示モード指定
    glutInitWindowSize(1024, 640);                             // ウィンドウサイズの指定
    glutCreateWindow("CG Final");                              // ウィンドウ生成

    // コールバック関数指定
    glutDisplayFunc(display);          // ディスプレイコールバック関数（"display"）
    glutReshapeFunc(reshape);          // リサイズコールバック関数（"reshape"）
    glutTimerFunc(1000 / f, timer, 0); // タイマーコールバック関数（"timer", 1000/fミリ秒）
    glutMouseFunc(mouse);              // マウスクリックコールバック関数
    glutMotionFunc(motion);            // マウスドラッグコールバック関数
    glutKeyboardFunc(keyboard);        // キーボードコールバック関数
    drawTetsujin();
    drawArm();

    // 各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0);                  // ウィンドウクリア色の指定（RGBA値）
    glEnable(GL_DEPTH_TEST);                           // デプスバッファ有効化
    glEnable(GL_NORMALIZE);                            // ベクトル正規化有効化
    glEnable(GL_BLEND);                                // ブレンディング有効化
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ブレンディング方法指定（アルファブレンディング）

    // 陰影付け・光源
    glEnable(GL_LIGHTING); // 陰影付け有効化
    GLfloat col[4];        // 光源設定用配列
    // 光源0
    glEnable(GL_LIGHT0); // 光源0有効化
    col[0] = 0.8;
    col[1] = 0.8;
    col[2] = 0.8;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  // 拡散反射対象
    glLightfv(GL_LIGHT0, GL_SPECULAR, col); // 鏡面反射対象
    col[0] = 0.2;
    col[1] = 0.2;
    col[2] = 0.2;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);                    // 環境光対象
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0000001); // 減衰率
    // 光源1
    glEnable(GL_LIGHT1); // 光源1有効化
    col[0] = 0.8;
    col[1] = 0.8;
    col[2] = 0.8;
    col[3] = 1.0;
    glLightfv(GL_LIGHT1, GL_DIFFUSE, col);  // 拡散反射対象
    glLightfv(GL_LIGHT1, GL_SPECULAR, col); // 鏡面反射対象
    col[0] = 0.2;
    col[1] = 0.2;
    col[2] = 0.2;
    col[3] = 1.0;
    glLightfv(GL_LIGHT1, GL_AMBIENT, col);                    // 環境光対象
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0000001); // 減衰率

    // 視点極座標
    eDist = 5000.0; // 距離
    eDegX = 20.0;
    eDegY = 180.0; // x軸周り角度，y軸周り角度

    // 床頂点座標
    for (int j = 0; j < TILE; j++)
    {
        for (int i = 0; i < TILE; i++)
        {
            fPoint[i][j].x = -fWidth / 2.0 + i * fWidth / (TILE - 1);
            fPoint[i][j].y = 0.0;
            fPoint[i][j].z = -fWidth / 2.0 + j * fWidth / (TILE - 1);
        }
    }

    cv::Mat textureImage; // テクスチャ画像格納用
    char filename[256];   // ファイル名格納用
    sprintf(filename, "ait.jpg");
    textureImage = cv::imread(filename, cv::IMREAD_UNCHANGED);
    glBindTexture(GL_TEXTURE_2D, 0);                                                                                              // テクスチャオブジェクトのバインド
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                                                            // 拡大時の補間方法
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);                                                            // 縮小時の補間方法
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureImage.cols, textureImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, textureImage.data); // テクスチャ画像の指定
}

// リサイズコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ウィンドウ内の描画領域(ビューポート)の指定

    // 投影変換
    glMatrixMode(GL_PROJECTION);                               // カレント行列の設定
    glLoadIdentity();                                          // カレント行列初期化
    gluPerspective(40.0, (double)w / (double)h, 1.0, 10000.0); // 投影変換行列生成

    winW = w;
    winH = h; // ウィンドウサイズをグローバル変数に格納
}

// ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ウィンドウクリア

    // ビューイング変換準備
    glMatrixMode(GL_MODELVIEW); // カレント行列の設定
    glLoadIdentity();           // 行列初期化
    // 視点極座標から直交座標へ変換
    Vec_3D e;
    e.x = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    e.y = eDist * sin(eDegX * M_PI / 180.0);
    e.z = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);
    // 視点設定・ビューイング変換
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // ビューイング変換行列生成

    // 光源配置
    GLfloat lightPos0[] = {500.0, 2000.0, 2500.0, 1.0};  // 光源座標(点光源)
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);        // 光源配置
    GLfloat lightPos1[] = {-500.0, 2000.0, -500.0, 1.0}; // 光源座標(点光源)
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);        // 光源配置

    // 質感用配列
    GLfloat col[4], spe[4], shi[1];

    //----------床パネル----------
    col[0] = 0.2;
    col[1] = 1.0;
    col[2] = 0.2;
    col[3] = 1.0;
    spe[0] = 1.0;
    spe[1] = 1.0;
    spe[2] = 1.0;
    spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glBegin(GL_QUADS); // 図形開始
    for (int j = 0; j < TILE - 1; j++)
    {
        for (int i = 0; i < TILE - 1; i++)
        {
            // 法線ベクトル計算
            Vec_3D v1, v2, n;
            v1 = diffVec(fPoint[i][j + 1], fPoint[i][j]);
            v2 = diffVec(fPoint[i + 1][j], fPoint[i][j]);
            n = crossProduct(v1, v2);  // 法線ベクトル
            glNormal3d(n.x, n.y, n.z); // 法線ベクトル適用
            // 頂点座標
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z);                         // 頂点座標
            glVertex3d(fPoint[i][j + 1].x, fPoint[i][j + 1].y, fPoint[i][j + 1].z);             // 頂点座標
            glVertex3d(fPoint[i + 1][j + 1].x, fPoint[i + 1][j + 1].y, fPoint[i + 1][j + 1].z); // 頂点座標
            glVertex3d(fPoint[i + 1][j].x, fPoint[i + 1][j].y, fPoint[i + 1][j].z);             // 頂点座標
        }
    }
    glEnd(); // 図形終了

    glPushMatrix();
    glTranslated(0.0, 0.0, -1500.0);
    // 円柱
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glPushMatrix();
    glTranslated(1000.0, 100.0, 0.0);
    glScaled(100.0, 100.0, 100.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    glMyCylinder(0.5, 0.5, 25.0, 50);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-1000.0, 100.0, 0.0);
    glScaled(100.0, 100.0, 100.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    glMyCylinder(0.5, 0.5, 25.0, 50);
    glPopMatrix();

    // Quadに対してテクスチャ割り当て
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    // 中央に配置
    glTranslated(0.0, 850.0, 0.0);
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(-1000.0, 0.0, 0.0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(1000.0, 0.0, 0.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(1000.0, 500.0, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(-1000.0, 500.0, 0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPopMatrix();

    drawTetsujin();

    glPushMatrix();
    col[0] = 1.0;
    col[1] = 0.8;
    col[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glRotated(90, 1, 0, 0);
    glTranslated(1500, 0, 0);
    glMyCylinder(500, 500, 100.0, 8);
    // 茶色のシリンダー
    col[0] = 1.0;
    col[1] = 0.25;
    col[2] = 0.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glPushMatrix();
    glTranslated(0, 0, -500);
    glMyCylinder(50, 50, 1000.0, 8);

    // メリーゴーランドの乗る部分0
    glPushMatrix();
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glRotated(theta, 0, 0, 1);
    glTranslated(250, 0, 200);
    glTranslated(0, 0, pole_h[0]);
    glMyCylinder(30, 30, 400.0, 8);
    glPushMatrix();
    glScaled(0.2, 0.2, 0.2);
    glRotated(180, 0, 1, 0);
    glTranslated(0, -500, 650);
    drawTetsujin();
    glPopMatrix();
    glPopMatrix();

        // メリーゴーランドの乗る部分1
    glPushMatrix();
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glRotated(theta, 0, 0, 1);
    glTranslated(-250, 0, 200);
    glTranslated(0, 0, pole_h[1]);
    glMyCylinder(30, 30, 400.0, 8);
    glPushMatrix();
    glScaled(0.2, 0.2, 0.2);
    glRotated(180, 0, 1, 0);
    glRotated(180, 0, 0, 1);
    glTranslated(0, -500, 650);
    drawTetsujin();
    glPopMatrix();
    glPopMatrix();

            // メリーゴーランドの乗る部分2
    glPushMatrix();
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glRotated(theta, 0, 0, 1);
    glTranslated(-250, -250, 200);
    glTranslated(0, 0, pole_h[2]);
    glMyCylinder(30, 30, 400.0, 8);
    glPushMatrix();
    glScaled(0.2, 0.2, 0.2);
    glRotated(180, 0, 1, 0);
    glRotated(180-45, 0, 0, 1);
    glTranslated(0, -500, 650);
    drawTetsujin();
    glPopMatrix();
    glPopMatrix();

            // メリーゴーランドの乗る部分3
    glPushMatrix();
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glRotated(theta, 0, 0, 1);
    glTranslated(250, -250, 200);
    glTranslated(0, 0, pole_h[3]);
    glMyCylinder(30, 30, 400.0, 8);
    glPushMatrix();
    glScaled(0.2, 0.2, 0.2);
    glRotated(180, 0, 1, 0);
    glRotated(45, 0, 0, 1);
    glTranslated(0, -500, 650);
    drawTetsujin();
    glPopMatrix();
    glPopMatrix();

                // メリーゴーランドの乗る部分4
    glPushMatrix();
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glRotated(theta, 0, 0, 1);
    glTranslated(0, 250, 200);
    glTranslated(0, 0, pole_h[4]);
    glMyCylinder(30, 30, 400.0, 8);
    glPushMatrix();
    glScaled(0.2, 0.2, 0.2);
    glRotated(180, 0, 1, 0);
    glRotated(-70, 0, 0, 1);
    glTranslated(0, -500, 650);
    drawTetsujin();
    glPopMatrix();
    glPopMatrix();

                // メリーゴーランドの乗る部分5

    glPopMatrix();



    col[0] = 1.0;
    col[1] = 0.5;
    col[2] = 0.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glTranslated(0, 0, -1000);
    glMyCylinder(500, 0, 500, 8);
    glPopMatrix();

    // 描画バッファ切り替え
    glutSwapBuffers();
}

// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();               // ディスプレイイベント強制発生
    glutTimerFunc(1000 / f, timer, 0); // タイマー再設定
    theta += 1.0;

    // pole_h[i]が500に達するまで+1し続け、500に達したら-1し続けるコード
    for (int i = 0; i < 8; i++)
    {
        if (pole_h_flag[i] == 0)
        {
            pole_h[i] += 10;
            if (pole_h[i] >= 200)
            {
                pole_h_flag[i] = 1;
            }
        }
        else
        {
            pole_h[i] -= 10;
            if (pole_h[i] <= 0)
            {
                pole_h_flag[i] = 0;
            }
        }
    }
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    GLfloat winX, winY, winZ;     // ウィンドウ座標
    GLdouble objX, objY, objZ;    // ワールド座標
    GLdouble model[16], proj[16]; // モデルビュー変換行列，投影変換行列格納用
    GLint view[4];                // ビューポート設定格納用

    // マウス座標→ウィンドウ座標
    winX = x;
    winY = winH - y;
    glReadPixels(winX * rDisp, winY * rDisp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    // 変換行列取り出し
    glGetDoublev(GL_MODELVIEW_MATRIX, model); // モデルビュー変換行列格納
    glGetDoublev(GL_PROJECTION_MATRIX, proj); // 投影変換行列格納
    glGetIntegerv(GL_VIEWPORT, view);         // ビューポート設定格納

    // ウィンドウ座標→ワールド座標（objX, objY, objZ）
    gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);

    // マウス情報をグローバル変数に格納
    mButton = button;
    mState = state;
    mX = x;
    mY = y;
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    // 左ドラッグで変更
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        eDegY += (mX - x) * 0.2; // マウス横方向→水平角
        eDegX += (y - mY) * 0.2; // マウス縦方向→垂直角
        if (eDegX > 85)
            eDegX = 85.0;
        if (eDegX < -85)
            eDegX = -85.0;
    }

    // マウス座標をグローバル変数に保存
    mX = x;
    mY = y;
}

// キーボードコールバック関数
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:     //[ESC]キー
    case 'q':    //[q]キー
    case 'Q':    //[Q]キー
        exit(0); // プロセス終了
        break;

    default:
        break;
    }
}

// v1とv2の外積計算
Vec_3D crossProduct(Vec_3D vec1, Vec_3D vec2)
{
    Vec_3D out; // 戻り値用
    // 外積各成分の計算
    out.x = vec1.y * vec2.z - vec1.z * vec2.y;
    out.y = vec1.z * vec2.x - vec1.x * vec2.z;
    out.z = vec1.x * vec2.y - vec1.y * vec2.x;
    // 正規化
    out = vectorNormalize(out);
    // 戻り値
    return out;
}

// v1とv2の内積計算
double innerProduct(Vec_3D v1, Vec_3D v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// ベクトルの差の計算((v1-v2)を出力)
Vec_3D diffVec(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;
    out.x = v1.x - v2.x;
    out.y = v1.y - v2.y;
    out.z = v1.z - v2.z;
    return out;
}

// ベクトルの和の計算((v1+v2)を出力)
Vec_3D addVec(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;
    out.x = v1.x + v2.x;
    out.y = v1.y + v2.y;
    out.z = v1.z + v2.z;
    return out;
}

// ベクトルの定数c倍(c*vを出力)
Vec_3D multiVec(Vec_3D v, double c)
{
    Vec_3D out;
    out.x = c * v.x;
    out.y = c * v.y;
    out.z = c * v.z;
    return out;
}

// ベクトルの長さ
double vectorLen(Vec_3D vec)
{
    double len = sqrt(pow(vec.x, 2) + pow(vec.y, 2) + pow(vec.z, 2));
    return len;
}

// ベクトル正規化
Vec_3D vectorNormalize(Vec_3D vec)
{
    // ベクトル長
    double len = vectorLen(vec);
    // 各成分をベクトル長で割る
    if (len > 0)
    {
        vec.x /= len;
        vec.y /= len;
        vec.z /= len;
    }
    // 戻り値
    return vec;
}

/*
 塗り潰し円（多角形）描画関数
    (cx, cy, cz) : 中心座標
    r : 半径
    p : 分割数
    w : 線の太さ
 */
void glMySolidCircle(double cx, double cy, double cz, double r, int p)
{
    double x, y, z;  // 座標計算結果
    double t, delta; // パラメータt 及び変位量 delta

    delta = 2.0 * M_PI / p;

    // 折れ線近所の開始
    glBegin(GL_POLYGON);
    glNormal3d(0, 0, 1);
    for (int i = 0; i <= p; i++)
    {
        t = delta * i;
        x = r * cos(t) + cx;
        y = r * sin(t) + cy;
        z = cz;
        glVertex3d(x, y, z); // 円周上の頂点s
    }
    glEnd();
}

/*
 円柱（多角柱）描画関数（top, bottomのどちらかを0にすれば錐になる）
    top : 上面の半径
    bottom : 底面の半径
    height : 柱の高さ
    slices : 円周の分割数（何角柱にするか）
    ※滑らかな円柱にしたい場合は，何回か前に床面でやったように，隣接する４面の法線の平均を法線にすると良い（Phongシェーディングと言います）
 */
void glMyCylinder(double top, double bottom, double height, double slices)
{
    glPushMatrix();

    // 上面の円
    glPushMatrix();
    glTranslated(0, 0, height / 2);
    glMySolidCircle(0, 0, 0, top, slices);
    glPopMatrix();
    // 底面の円
    glPushMatrix();
    glTranslated(0, 0, -height / 2);
    glRotated(180, 0, 1, 0);
    glRotated(180, 0, 0, 1);
    glMySolidCircle(0, 0, 0, bottom, slices);
    glPopMatrix();

    // 胴体
    Vec_3D p1, p2, p3, p4; // ４頂点
    Vec_3D v1, v2, nv;
    double t1, t2, x, y, z;
    double delta = 2.0 * M_PI / slices;
    // 折れ線近似の開始
    glBegin(GL_QUADS);
    for (int i = 0; i < (int)slices; i++)
    { // 円周上のサンプリング
        t1 = delta * i;
        // p1の計算
        x = top * cos(t1);
        y = top * sin(t1);
        z = height / 2;
        p1 = (Vec_3D){x, y, z};
        // p2の計算
        x = bottom * cos(t1);
        y = bottom * sin(t1);
        z = -height / 2;
        p2 = (Vec_3D){x, y, z};
        t2 = delta * (i + 1);
        // p4の計算
        x = top * cos(t2);
        y = top * sin(t2);
        z = height / 2;
        p4 = (Vec_3D){x, y, z};
        // p3の計算
        x = bottom * cos(t2);
        y = bottom * sin(t2);
        z = -height / 2;
        p3 = (Vec_3D){x, y, z};
        // 法線の計算
        v1 = diffVec(p2, p1);
        v2 = diffVec(p3, p1);
        nv = crossProduct(v1, v2);
        glNormal3d(nv.x, nv.y, nv.z);
        // ４頂点に四角形ポリゴンを貼り付け
        glVertex3d(p1.x, p1.y, p1.z); // 円周上の頂点s
        glVertex3d(p2.x, p2.y, p2.z); // 円周上の頂点s
        glVertex3d(p3.x, p3.y, p3.z); // 円周上の頂点s
        glVertex3d(p4.x, p4.y, p4.z); // 円周上の頂点s
    }
    glEnd();

    glPopMatrix();
}

void drawTetsujin()
{
    glPushMatrix();

    // マテリアルの作成
    GLfloat bodyCol[4], bodySpe[4], bodyShi[1];
    bodyCol[0] = 0.0;
    bodyCol[1] = 0.0;
    bodyCol[2] = 1.0;
    bodyCol[3] = 1.0;
    bodySpe[0] = 1.0;
    bodySpe[1] = 1.0;
    bodySpe[2] = 1.0;
    bodySpe[3] = 1.0;
    bodyShi[0] = 50.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyCol);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, bodySpe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, bodyShi);

    glScaled(240.0, 250.0, 250.0);

    glPushMatrix();
    glScaled(1.0, 0.8, 1.0);

    // 腰の描画
    glPushMatrix();
    glTranslated(0.0, 4.0, 0.0);
    glutSolidSphere(1.0, 20, 20);

    // 胴体の描画
    glPushMatrix();
    glTranslated(0.0, 1.0, 0.0);
    glRotated(90, 1.0, 0.0, 0.0);
    glMyCylinder(1.0, 1.0, 2.0, 40);
    drawArm();

    // 反転させる
    glPushMatrix();
    glRotated(180, 0.0, 0.0, 1.0);
    drawArm();
    glPopMatrix();

    // 頭の描画
    glPushMatrix();

    glTranslated(0.0, 0.0, -2.7);
    glScaled(0.7, 0.7, 0.7);
    glutSolidSphere(1.0, 20, 20);
    // 鼻の描画
    glPushMatrix();
    glRotated(90, 1.0, 0.0, 0.0);
    glTranslated(0.0, 0.0, 0.7);
    glutSolidCone(0.5, 1.0, 20, 20);
    glPopMatrix();
    // 左目の描画
    glPushMatrix();
    bodyCol[0] = 1.0;
    bodyCol[1] = 1.0;
    bodyCol[2] = 0.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyCol);
    glScaled(1.0, 1.0, 0.6);
    glTranslated(-0.5, -0.6, -0.3);
    glRotated(90, 1.0, 0.0, 0.0);
    glutSolidSphere(0.5, 20, 20);
    glPopMatrix();
    // 右目の描画
    glPushMatrix();
    bodyCol[0] = 1.0;
    bodyCol[1] = 1.0;
    bodyCol[2] = 0.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyCol);
    glScaled(1.0, 1.0, 0.6);
    glTranslated(0.5, -0.6, -0.3);
    glRotated(90, 1.0, 0.0, 0.0);
    glutSolidSphere(0.5, 20, 20);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();

    glPushMatrix();
    bodyCol[0] = 1.0;
    bodyCol[1] = 0.0;
    bodyCol[2] = 0.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyCol);

    glTranslated(0.0, 1.0, 0.0);
    glRotated(90, 1.0, 0.0, 0.0);
    glMyCylinder(1.1, 1.1, 0.5, 40);
    glPopMatrix();

    bodyCol[0] = 0.0;
    bodyCol[1] = 0.0;
    bodyCol[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyCol);

    glPushMatrix();
    glTranslated(0.0, 2.0, 0.0);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    // 腰のPop
    glPopMatrix();

    // 左足の描画
    glPushMatrix();
    glTranslated(-0.5, 1.8, 0.0);
    glRotated(90, 1.0, 0.0, 0.0);
    glMyCylinder(0.3, 0.3, 3.5, 40);

    // 左足先の描画
    glPushMatrix();
    glTranslated(0.0, -0.5, 1.8);
    glScaled(0.5, 1.0, 0.4);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.0, 0.0, -0.5);
    // glMyCylinder(0.4, 0.4, 0.3, 40);
    glutSolidTorus(0.2, 0.2, 20, 20);
    glPopMatrix();
    glPopMatrix();

    // 右足の描画
    glPushMatrix();
    glTranslated(0.5, 1.8, 0.0);
    glRotated(90, 1.0, 0.0, 0.0);
    glMyCylinder(0.3, 0.3, 3.5, 40);

    // 右足先の描画
    glPushMatrix();
    glTranslated(0.0, -0.5, 1.8);
    glScaled(0.5, 1.0, 0.4);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.0, 0.0, -0.5);
    // glMyCylinder(0.4, 0.4, 0.3, 40);
    glutSolidTorus(0.2, 0.2, 20, 20);
    glPopMatrix();
    glPopMatrix();

    // 縮小のpop
    glPopMatrix();

    glPopMatrix();
}

void drawArm()
{
    // 左肩の描画
    glPushMatrix();
    glTranslated(-1.0, 0.0, -1.5);
    glRotated(90, 0.0, 1.0, 0.0);
    glMyCylinder(0.3, 0.3, 0.5, 40);
    // 左腕の描画
    glPushMatrix();
    glTranslated(0.0, 0.0, -0.7);
    glMyCylinder(0.25, 0.25, 1.3, 40);
    glPushMatrix();
    glTranslated(0.0, 0.0, -0.8);
    glutSolidSphere(0.25, 20, 20);
    glPushMatrix();
    glTranslated(0.6, 0.0, 0.0);
    glRotated(90, 0.0, 1.0, 0.0);
    glMyCylinder(0.25, 0.25, 0.8, 40);
    glPushMatrix();
    glTranslated(0.0, 0.0, 0.8);
    glutSolidSphere(0.4, 20, 20);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}