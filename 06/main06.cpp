// g++ main06.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUT

#include <vector> //動的配列確保

// 構造体の定義
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

typedef struct _GeoPoint // 都市の緯度・経度の構造体
{
    char location[256]; // 都市名
    double latitude;    // 緯度
    double longitude;   // 経度
} GeoPoint;

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
void draw_axes(double scale); // 軸の描画
void glEnable(GLenum cap);

// グローバル変数
int winW = 800, winH = 600;  // ウィンドウサイズを覚えておく変数
int mButton, mState, mX, mY; // マウス系の変数
double eDist = 120.0;        // 視点と原点との距離
double eDegX = 10.0;         // 緯度
double eDegY = 0.0;          // 経度
double T = 0.0;              // 時刻
double fps = 30.0;           // フレームレート
double dT = 1.0 / fps;       // 時間間隔
double theta = 0.0;          // 角度
std::vector<GeoPoint> GP;    // 都市の緯度・経度用動的配列
float zoomFactor = 1.0f;
int mouseX, mouseY;

double armAngle = -50.0;
int direction = -1;

#define WIDTH 320
#define HEIGHT 240

// 回転用
float anglex = 0.0f;
// 青
GLfloat Blue[] = {0.0, 0.0, 1.0, 1.0};
// ライトの位置
GLfloat lightpos[] = {200.0, 150.0, -500.0, 1.0};

// 円柱
void cylinder(float radius, float height, int sides)
{
    double pi = 3.1415;
    // 上面
    glNormal3d(0.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    for (double i = 0; i < sides; i++)
    {
        double t = pi * 2 / sides * (double)i;
        glVertex3d(radius * cos(t), height, radius * sin(t));
    }
    glEnd();
    // 側面
    glBegin(GL_QUAD_STRIP);
    for (double i = 0; i <= sides; i = i + 1)
    {
        double t = i * 2 * pi / sides;
        glNormal3f((GLfloat)cos(t), 0.0, (GLfloat)sin(t));
        glVertex3f((GLfloat)(radius * cos(t)), -height, (GLfloat)(radius * sin(t)));
        glVertex3f((GLfloat)(radius * cos(t)), height, (GLfloat)(radius * sin(t)));
    }
    glEnd();
    // 下面
    glNormal3d(0.0, -1.0, 0.0);
    glBegin(GL_POLYGON);
    for (double i = sides; i >= 0; --i)
    {
        double t = pi * 2 / sides * (double)i;
        glVertex3d(radius * cos(t), -height, radius * sin(t));
    }
    glEnd();
}

// メイン関数
int main(int argc, char *argv[])
{
    glutInit(&argc, argv); // OpenGL,GLUTの初期化

    initGL(); // 初期設定

    glutMainLoop(); // イベント待ち無限ループ突入

    return 0;
}

// 初期設定関数
void initGL()
{
    // ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // ディスプレイ表示モード指定
    glutInitWindowSize(winW, winH);                            // ウィンドウサイズの指定（800×600）
    glutCreateWindow("CG06");                                  // ウィンドウ生成
    // コールバック関数指定
    glutDisplayFunc(display);    // ディスプレイコールバック関数（"display"）
    glutReshapeFunc(reshape);    // リシェイプコールバック関数（"reshape"）
    glutMouseFunc(mouse);        // マウスクリックコールバック関数の指定（"mouse()"）
    glutMotionFunc(motion);      // マウスドラッグコールバック関数の指定（"motion()"）
    glutKeyboardFunc(keyboard);  // キーボードコールバック関数
    glutTimerFunc(33, timer, 0); // タイマーコールバック関数（"timer"）

    // 各種設定
    glClearColor(0.0, 0.0, 0.3, 1.0); // ウィンドウクリア色の指定（RGBA値）
    glEnable(GL_DEPTH_TEST);          // デプスバッファを使う

    // 緯度・経度のセット（とりあえず北から１０個分，サンプルとして記述しておく）
    // 参考 URL https://www.benricho.org/chimei/latlng_data.html
    GP.push_back((GeoPoint){"Hokkaido", 43.06417, 141.34694});
    GP.push_back((GeoPoint){"Aomori", 40.82444, 140.74});
    GP.push_back((GeoPoint){"Morioka", 39.70361, 141.1525});
    GP.push_back((GeoPoint){"Sendai", 38.26889, 140.87194});
    GP.push_back((GeoPoint){"Akita", 39.71861, 140.1025});
    GP.push_back((GeoPoint){"Yamagata", 38.24056, 140.36333});
    GP.push_back((GeoPoint){"Fukushima", 37.75, 140.46778});
    GP.push_back((GeoPoint){"Mito", 36.34139, 140.44667});
    GP.push_back((GeoPoint){"Utsunomiya", 36.56583, 139.88361});
    GP.push_back((GeoPoint){"Maebashi", 36.39111, 139.06083});
    GP.push_back((GeoPoint){"Saitama", 35.85694, 139.64889});
    GP.push_back((GeoPoint){"Chiba", 35.60472, 140.12333});
    GP.push_back((GeoPoint){"Tokyo", 35.68944, 139.69167});
    GP.push_back((GeoPoint){"Yokohama", 35.44778, 139.6425});
    GP.push_back((GeoPoint){"Niigata", 37.90222, 139.02361});
    GP.push_back((GeoPoint){"Toyama", 36.69528, 137.21139});
    GP.push_back((GeoPoint){"Kanazawa", 36.59444, 136.62556});
    GP.push_back((GeoPoint){"Fukui", 36.06528, 136.22194});
    GP.push_back((GeoPoint){"Nagano", 36.65139, 138.18111});
    GP.push_back((GeoPoint){"Gifu", 35.39111, 136.72222});
    GP.push_back((GeoPoint){"Shizuoka", 34.97694, 138.38306});
    GP.push_back((GeoPoint){"Nagoya", 35.18028, 136.90667});
    GP.push_back((GeoPoint){"Kyoto", 35.02139, 135.75556});
    GP.push_back((GeoPoint){"Osaka", 34.68639, 135.52});
    GP.push_back((GeoPoint){"Kobe", 34.69139, 135.18306});
    GP.push_back((GeoPoint){"Nara", 34.68528, 135.83278});
    GP.push_back((GeoPoint){"Wakayama", 34.22611, 135.1675});
    GP.push_back((GeoPoint){"Tottori", 35.50361, 134.23833});
    GP.push_back((GeoPoint){"Matsue", 35.47222, 133.05056});
    GP.push_back((GeoPoint){"Okayama", 34.66167, 133.935});
    GP.push_back((GeoPoint){"Hiroshima", 34.39639, 132.45944});
    GP.push_back((GeoPoint){"Yamaguchi", 34.18583, 131.47139});
    GP.push_back((GeoPoint){"Tokushima", 34.06583, 134.55944});
    GP.push_back((GeoPoint){"Takamatsu", 34.34028, 134.04333});
    GP.push_back((GeoPoint){"Matsuyama", 33.84167, 132.76611});
    GP.push_back((GeoPoint){"Kochi", 33.55972, 133.53111});
    GP.push_back((GeoPoint){"Fukuoka", 33.60639, 130.41806});
    GP.push_back((GeoPoint){"Saga", 33.24944, 130.29889});
    GP.push_back((GeoPoint){"Nagasaki", 32.74472, 129.87361});
    GP.push_back((GeoPoint){"Kumamoto", 32.78972, 130.74167});
    GP.push_back((GeoPoint){"Oita", 33.23806, 131.6125});
    GP.push_back((GeoPoint){"Miyazaki", 31.91111, 131.42389});
    GP.push_back((GeoPoint){"Kagoshima", 31.56028, 130.55806});
    GP.push_back((GeoPoint){"Naha", 26.2125, 127.68111});
}

// リシェイプコールバック関数(w：ウィンドウ幅，h：ウィンドウ高さ)
void reshape(int w, int h)
{
    glViewport(0, 0, w, h); // ビューポート設定（ウィンドウ全体を表示領域に設定）
    // 投影変換
    glMatrixMode(GL_PROJECTION); // カレント行列の設定（投影変換行列）
    glLoadIdentity();            // カレント行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 1000.0);

    winW = w;
    winH = h; // ウィンドウサイズの更新
}

// ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ウィンドウクリア

    // Apply zoom
    glScalef(zoomFactor, zoomFactor, 1.0f);

    // double ex, ey, ez;
    Vec_3D e;
    e.x = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    e.y = eDist * sin(eDegX * M_PI / 180.0);
    e.z = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);

    double ex, ey, ez;
    // 視点座標の計算
    ex = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    ey = eDist * sin(eDegX * M_PI / 180.0);
    ez = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);

    // ビューイング変換
    glMatrixMode(GL_MODELVIEW); // カレント行列の設定（モデルビュー変換行列）
    glLoadIdentity();           // カレント行列初期化
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // 床
    glColor4d(0.5, 0.5, 0.5, 1.0); // 色の設定（RGBA値）
    glBegin(GL_QUADS);             // 図形（複数の直線）の開始
    glVertex3d(-50.0, 0.0, -50.0); // 頂点座標0
    glVertex3d(-50.0, 0.0, 50.0);  // 頂点座標1
    glVertex3d(50.0, 0.0, 50.0);   // 頂点座標1
    glVertex3d(50.0, 0.0, -50.0);  // 頂点座標1
    glEnd();                       // 図形の終了

    //----------図形２ (直方体)----------
    glPushMatrix(); // 図形２の変換
    glTranslated(20.0, 5.0, 0.0);
    glScaled(10.0, 10.0, 10.0);
    // 図形定義
    glutWireCube(1.01);
    glColor4d(1.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glutSolidCube(1.0);
    draw_axes(1.5);
    glPopMatrix(); // 図形２の変換終わり

    //----------図形３ (地球儀)----------
    // 全体的に右に動かす
    glPushMatrix(); // 図形３の変換
    glTranslated(40.0, 5.0, 0.0);
    // 地軸を23.4度傾ける
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 5.0, 0.0);
    glRotated(23.4, 1.0, 0.0, 0.0);
    glRotated(theta, 0.0, 1.0, 0.0);
    glScaled(5.0, 5.0, 5.0);
    // 経線
    glColor4d(1.0, 1.0, 1.0, 1.0); // 色の設定（RGBA 値）
    glBegin(GL_LINE_STRIP);
    int degY = 0;
    for (int degY = -180; degY <= 180; degY += 30)
    {
        for (int degX = -90; degX <= 90; degX += 10)
        {
            double x = cos(degX * M_PI / 180.0) * sin(degY * M_PI / 180.0);
            double y = sin(degX * M_PI / 180.0);
            double z = cos(degX * M_PI / 180.0) * cos(degY * M_PI / 180.0);
            glVertex3d(x, y, z);
        }
    }
    glEnd();
    // 経線ここまで
    // 緯線
    glBegin(GL_LINE_STRIP);
    for (int degX = -90; degX <= 90; degX += 30)
    {
        for (int degY = -180; degY <= 180; degY += 10)
        {
            double x = cos(degX * M_PI / 180.0) * sin(degY * M_PI / 180.0);
            double y = sin(degX * M_PI / 180.0);
            double z = cos(degX * M_PI / 180.0) * cos(degY * M_PI / 180.0);
            glVertex3d(x, y, z);
        }
    }
    glEnd();
    // 緯線ここまで
    // 青い地球
    glColor4d(0.3, 0.3, 1.0, 1.0); // 色の設定（RGBA値）
    glutSolidSphere(1.0, 24, 12);
    // 青い地球ここまで
    // ピン
    glColor4d(1.0, 1.0, 0.0, 1.0); // 色の設定（RGBA 値）
    glPushMatrix();
    // 地球表面からちょっと離れた位置（1.2 倍の半径）
    double x = 1.2 * cos(35.18 * M_PI / 180.0) * sin(137.11 * M_PI / 180.0);
    double y = 1.2 * sin(35.18 * M_PI / 180.0);
    double z = 1.2 * cos(35.18 * M_PI / 180.0) * cos(137.11 * M_PI / 180.0);
    glTranslated(x, y, z);
    glutSolidSphere(0.05, 24, 12); // ピンの丸の部分
    glPopMatrix();
    glBegin(GL_LINES); // ピンの棒の部分
    glVertex3d(x, y, z);
    glVertex3d(0, 0, 0);
    glEnd();
    // ピンここまで
    // 日本
    glColor4d(0.0, 1.0, 0.0, 1.0); // 色の設定（RGBA 値）
    double dist = 1.0;
    for (int i = 0; i < GP.size(); i++)
    {
        double x = dist * cos(GP[i].latitude * M_PI / 180.0) * sin(GP[i].longitude * M_PI / 180.0);
        double y = dist * sin(GP[i].latitude * M_PI / 180.0);
        double z = dist * cos(GP[i].latitude * M_PI / 180.0) * cos(GP[i].longitude * M_PI / 180.0);
        glPushMatrix();
        glTranslated(x, y, z);
        glutSolidSphere(0.01, 24, 12);
        glPopMatrix();
    }
    // 日本ここまで
    draw_axes(1.5);
    glPopMatrix(); // 図形３の変換終わり
    glPopMatrix(); // 図形３の変換終わり

    // オリジナルオブジェクトのモデリング
    // 怪盗グルーのミニオンを描画
    // 全体を上にずらす
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 10.0, 0.0);
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 10.0, 0.0);
    glScaled(5.0, 5.0, 5.0);
    // 黄色の球
    glColor4d(1.0, 0.86, 0.0, 1.0); // 色の設定（RGBA値）
    glutSolidSphere(1.0, 24, 12);
    glPopMatrix();
    // 黄色の球ここまで

    // 黄色い円柱を描画
    glColor4d(1.0, 0.86, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 5.5, 0.0);
    cylinder(5.0, 4.5, 24);
    glPopMatrix();
    // 黄色い円柱ここまで

    // 眼帯全体を上に少し上げる
    glPushMatrix(); // 眼帯の変換
    glTranslated(0.0, 1.5, 0.0);

    // 黒い円柱を描画
    glColor4d(0.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 8.0, 0.0);
    cylinder(5.2, 1.0, 24);
    glPopMatrix();
    // 黒い円柱ここまで
    // 灰色の円柱
    glColor4d(0.5, 0.5, 0.5, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 8.0, 5.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    cylinder(2.0, 1.0, 24);
    glPopMatrix();
    // 灰色の円柱ここまで
    // 灰色の円柱の中に白の円柱を描画
    glColor4d(1.0, 1.0, 1.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 8.0, 5.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    cylinder(1.5, 1.1, 24);
    glPopMatrix();
    // 灰色の円柱の中に白の円柱ここまで
    // 白の円柱の中に円柱描画
    glColor4d(0.8, 0.64, 0.37, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 8.0, 5.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    cylinder(0.8, 1.2, 24);
    glPopMatrix();
    // 白の円柱の中に円柱描画ここまで
    glPopMatrix(); // 眼帯の変換終わり

    // 青い球(下半身)を描画
    glColor4d(0.23, 0.33, 1.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glScaled(5.0, 5.0, 5.0);
    glTranslated(0.0, 0.2, 0.0);
    glutSolidSphere(1.0, 24, 12);
    glPopMatrix();
    // 青い球を描画ここまで
    glPopMatrix(); // 図形３の変換終わり

    // 足の部分描画
    // 下半身と同じ色で足を2本描画
    glColor4d(0.23, 0.33, 1.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(1.5, 5.0, 0.0);
    cylinder(0.8, 3.0, 24);
    glPopMatrix();

    glColor4d(0.23, 0.33, 1.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(-1.5, 5.0, 0.0);
    cylinder(0.8, 3.0, 24);
    glPopMatrix();
    // 足の部分描画ここまで

    // 黒い靴を描画
    glColor4d(0.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(1.5, 2.0, 0.0);
    glScaled(1.0, 1.0, 2.0);
    cylinder(1.0, 1.0, 24);
    glPopMatrix();

    glColor4d(0.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(-1.5, 2.0, 0.0);
    glScaled(1.0, 1.0, 2.0);
    cylinder(1.0, 1.0, 24);
    glPopMatrix();
    // 黒い靴を描画ここまで

    // 腕の描画
    // 黄色い円柱を描画
    glColor4d(1.0, 0.86, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(5.0, 15.0, 0.0);
    // 回転の中心点をずらす
    glTranslated(0.0, 0.0, 0.0);
    glRotated(armAngle, 0.0, 0.0, 90.0);
    glTranslated(0.0, 2.5, 0.0);

    cylinder(0.8, 3.0, 24);

    // 手の部分
    glPushMatrix();
    glColor4d(0.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glTranslated(0.0, 4.0, 0.0);
    glScaled(1.0, 1.0, 2.0);
    glutSolidSphere(0.8, 24, 12);
    //指
    glPushMatrix();
    glTranslated(0.0, 0.0, 1.0);
    glutSolidSphere(0.2, 24, 12);

    glTranslated(0.0, 0.0, -2.0);
    glutSolidSphere(0.2, 24, 12);

    glTranslated(0.0, 0.0, 1.0);
    glRotated(90.0, 0.0, 1.0, 0.0);
    glutSolidSphere(0.2, 24, 12);

    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
    // 黄色い円柱を描画ここまで

    //反対の腕
    // 黄色い円柱を描画
    glColor4d(1.0, 0.86, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(-5.0, 15.0, 0.0);
    // 回転の中心点をずらす
    glTranslated(0.0, 0.0, 0.0);
    glRotated(-armAngle, 0.0, 0.0, 90.0);
    glTranslated(0.0, 2.5, 0.0);

    cylinder(0.8, 3.0, 24);

    // 手の部分
    glPushMatrix();
    glColor4d(0.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glTranslated(0.0, 4.0, 0.0);
    glScaled(1.0, 1.0, 2.0);
    glutSolidSphere(0.8, 24, 12);
    //指
    glPushMatrix();
    glTranslated(0.0, 0.0, 1.0);
    glutSolidSphere(0.2, 24, 12);

    glTranslated(0.0, 0.0, -2.0);
    glutSolidSphere(0.2, 24, 12);

    glTranslated(0.0, 0.0, 1.0);
    glRotated(90.0, 0.0, 1.0, 0.0);
    glutSolidSphere(0.2, 24, 12);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();

    glutSwapBuffers();

    // 更新
    eDegY += 1.0; // 旋回
    //    eDegY = 30.0*sin(theta);  // 振り子
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    // ズーム用の値の取得
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        mouseX = x;
        mouseY = y;
    }
    mX = x;
    mY = y;
    mButton = button;
    mState = state;
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton == GLUT_LEFT_BUTTON)
    {
        eDegY += (mX - x) * 0.5;
        eDegX += (y - mY) * 0.5;
        // マウス座標をグローバル変数に保存
    }
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        eDist += (mY - y) * 0.5;
        // マウス座標をグローバル変数に保存
        mX = x;
        mY = y;
    }
    int dy = y - mouseY;
    zoomFactor += dy * 0.01f;
    mouseY = y;
    glutPostRedisplay();
}

// タイマーコールバック関数
void timer(int value)
{
    T += dT;                     // 時刻更新
    theta += 360.0 * dT;         // 角度の更新．角速度３６０度
    glutPostRedisplay();         // ディスプレイイベント強制発生
    glutTimerFunc(33, timer, 0); // タイマー再設定
                                 // armAngleを-40まで-1ずつ減らし、-40になったら40まで1ずつ増やす

    if (armAngle > -100 && direction == -1)
    {
        armAngle -= 1;
    }
    else if (armAngle < -20 && direction == 1)
    {
        armAngle += 1;
    }

    if (armAngle == -100)
    {
        direction = 1;
    }
    else if (armAngle == -20)
    {
        direction = -1;
    }
}

// キーボード
void keyboard(unsigned char key, int x, int y)
{
    //    printf("key = %d at (%d, %d)\n", key, x, y);

    switch (key)
    {
    case 27:
    case 'q':
    case 'Q':
        exit(0); // プログラム終了
        break;
    default:
        break;
    }
}

// 軸の描画
void draw_axes(double scale)
{
    glPushMatrix();
    glScaled(scale, scale, scale);

    //----------図形0 (座標軸)----------
    // 図形定義
    glBegin(GL_LINES);             // 図形（複数の直線）の開始
    glColor4d(1.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(-1.0, 0.0, 0.0);    // 頂点座標0
    glVertex3d(1.0, 0.0, 0.0);     // 頂点座標1
    glColor4d(0.0, 1.0, 0.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(0.0, -1.0, 0.0);    // 頂点座標2
    glVertex3d(0.0, 1.0, 0.0);     // 頂点座標3
    glColor4d(0.0, 0.0, 1.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(0.0, 0.0, -1.0);    // 頂点座標2
    glVertex3d(0.0, 0.0, 1.0);     // 頂点座標3
    glEnd();                       // 図形の終了

    glPopMatrix();
}
