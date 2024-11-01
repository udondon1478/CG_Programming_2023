// g++ main07 copy.cpp -framework OpenGL -framework GLUT -Wno-deprecated
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

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void timer(int value);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2); // ベクトル外積計算関数
double vectorNormalize(Vec_3D *vec);        // ベクトル正規化用関数

// グローバル変数
double eDist, eDegX, eDegY;
double mX, mY, mState, mButton;
int winW, winH;
float zoomFactor = 1.0f;
int mouseX, mouseY;

double armAngle = -50.0;
int direction = -1;

double T = 0.0;        // 時刻
double fps = 30.0;     // フレームレート
double dT = 1.0 / fps; // 時間間隔
double theta = 0.0;    // 角度

#define WIDTH 320
#define HEIGHT 240

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
    // 描画ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // ディスプレイモードの指定
    glutInitWindowSize(800, 600);                              // ウィンドウサイズの指定
    glutCreateWindow("CG_07");                                 // ウィンドウの生成
    // コールバック関数の指定
    glutDisplayFunc(display);    // ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape);    // リシェイプコールバック関数の指定（"reshape()"）
    glutMouseFunc(mouse);        // マウスクリックコールバック関数の指定（"mouse()"）
    glutMotionFunc(motion);      // マウスドラッグコールバック関数の指定（"motion()"）
    glutTimerFunc(33, timer, 0); // タイマーコールバック関数（"timer"）
    // 各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0); // ウィンドウクリア色の指定（RGBA）
    glEnable(GL_DEPTH_TEST);          // デプスバッファの有効化
    glEnable(GL_NORMALIZE);           // 法線ベクトル正規化の有効化
    // 光源設定
    glEnable(GL_LIGHTING);                               // 陰影付けの有効化
    glEnable(GL_LIGHT0);                                 // 光源 0 の有効化
    GLfloat col[] = {1.0, 1.0, 1.0, 1.0};                // パラメータ設定
    glLightfv(GL_LIGHT1, GL_DIFFUSE, col);               // 光源 1 の拡散反射に関する強度
    glLightfv(GL_LIGHT1, GL_SPECULAR, col);              // 光源 1 の鏡面反射に関する強度
    glLightfv(GL_LIGHT1, GL_AMBIENT, col);               // 光源 1 の環境光反射に関する強度
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.0);   // 光源 1 の一定減衰率の設定
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05); // 光源 1 の二次減衰率の設定

    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);               // 光源 1 の拡散反射に関する強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);              // 光源 1 の鏡面反射に関する強度
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);               // 光源 1 の環境光反射に関する強度
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0);   // 光源 1 の一定減衰率の設定
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05); // 光源 1 の二次減衰率の設定
    // 視点関係
    eDist = 15.0;
    eDegX = 0.0;
    eDegY = 0.0;
}

// ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面消去
    // 視点座標の計算
    Vec_3D e;
    e.x = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    e.y = eDist * sin(eDegX * M_PI / 180.0);
    e.z = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);
    // モデルビュー変換の設定
    glMatrixMode(GL_MODELVIEW);                             // 変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();                                       // 行列初期化
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点視線設定（視野変換行列を乗算）
    // 光源 0 の位置指定
    GLfloat lightpos0[] = {1.0, 3.0, 5.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos0);
    GLfloat col[4], spe[4], shi[1]; // 材質設定用変数
    // 物体 1（球）
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.0;
    col[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe[0] = 1.0;
    spe[1] = 1.0;
    spe[2] = 1.0;
    spe[3] = 1.0;                                                 // 鏡面反射係数
    shi[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト係数を設定

    // 床
    glPushMatrix();              // 行列一時保存
    glBegin(GL_QUADS);           // 物体（四角形）頂点配置開始
    glVertex3d(-4.0, 0.0, -4.0); // 頂点
    glVertex3d(4.0, 0.0, -4.0);  // 頂点
    glVertex3d(4.0, 0.0, 4.0);   // 頂点
    glVertex3d(-4.0, 0.0, 4.0);  // 頂点
    glEnd();                     // 物体頂点配置終了
    glPopMatrix();               // 行列復帰

    // 三角形4つからなるピラミッド
    Vec_3D p0, p1, p2; // 頂点
    p0.x = 0.0;
    p0.y = 2.0;
    p0.z = 0.3; // p0
    p1.x = -2.0;
    p1.y = 0.0;
    p1.z = -2.0; // p1
    p2.x = 2.0;
    p2.y = 0.0;
    p2.z = -2.0;       // p2
    Vec_3D v1, v2, nv; // 辺ベクトル，法線ベクトル
    v1.x = p1.x - p0.x;
    v1.y = p1.y - p0.y;
    v1.z = p1.z - p0.z; // 辺ベクトル v1 の計算
    v2.x = p2.x - p0.x;
    v2.y = p2.y - p0.y;
    v2.z = p2.z - p0.z;           // 辺ベクトル v2 の計算
    nv = normcrossprod(v1, v2);   // v1 と v2 の外積の計算
    glPushMatrix();               // 行列一時保存
    glNormal3d(nv.x, nv.y, nv.z); // 法線ベクトル設定
    glBegin(GL_TRIANGLES);        // 物体（三角形）頂点配置開始
    glVertex3d(p0.x, p0.y, p0.z); // 頂点 p0
    glVertex3d(p1.x, p1.y, p1.z); // 頂点 p1
    glVertex3d(p2.x, p2.y, p2.z); // 頂点 p2
    glEnd();                      // 物体頂点配置終了
    glPopMatrix();                // 行列復帰

    p0.x = 0.0;
    p0.y = 2.0;
    p0.z = 0.3; // p0
    p1.x = 2.0;
    p1.y = 0.0;
    p1.z = 2.0; // p1
    p2.x = 2.0;
    p2.y = 0.0;
    p2.z = -2.0; // p2

    v1.x = p1.x - p0.x;
    v1.y = p1.y - p0.y;
    v1.z = p1.z - p0.z; // 辺ベクトル v1 の計算
    v2.x = p2.x - p0.x;
    v2.y = p2.y - p0.y;
    v2.z = p2.z - p0.z;           // 辺ベクトル v2 の計算
    nv = normcrossprod(v1, v2);   // v1 と v2 の外積の計算
    glPushMatrix();               // 行列一時保存
    glNormal3d(nv.x, nv.y, nv.z); // 法線ベクトル設定
    glBegin(GL_TRIANGLES);        // 物体（三角形）頂点配置開始
    glVertex3d(p0.x, p0.y, p0.z); // 頂点 p0
    glVertex3d(p1.x, p1.y, p1.z); // 頂点 p1
    glVertex3d(p2.x, p2.y, p2.z); // 頂点 p2
    glEnd();                      // 物体頂点配置終了
    glPopMatrix();                // 行列復帰

    p0.x = 0.0;
    p0.y = 2.0;
    p0.z = 0.3; // p0
    p1.x = -2.0;
    p1.y = 0.0;
    p1.z = 2.0; // p1
    p2.x = 2.0;
    p2.y = 0.0;
    p2.z = 2.0; // p2

    v1.x = p1.x - p0.x;
    v1.y = p1.y - p0.y;
    v1.z = p1.z - p0.z; // 辺ベクトル v1 の計算
    v2.x = p2.x - p0.x;
    v2.y = p2.y - p0.y;
    v2.z = p2.z - p0.z;           // 辺ベクトル v2 の計算
    nv = normcrossprod(v1, v2);   // v1 と v2 の外積の計算
    glPushMatrix();               // 行列一時保存
    glNormal3d(nv.x, nv.y, nv.z); // 法線ベクトル設定
    glBegin(GL_TRIANGLES);        // 物体（三角形）頂点配置開始
    glVertex3d(p0.x, p0.y, p0.z); // 頂点 p0
    glVertex3d(p1.x, p1.y, p1.z); // 頂点 p1
    glVertex3d(p2.x, p2.y, p2.z); // 頂点 p2
    glEnd();                      // 物体頂点配置終了
    glPopMatrix();                // 行列復帰

    p0.x = 0.0;
    p0.y = 2.0;
    p0.z = 0.3; // p0
    p1.x = -2.0;
    p1.y = 0.0;
    p1.z = -2.0; // p1
    p2.x = -2.0;
    p2.y = 0.0;
    p2.z = 2.0; // p2

    v1.x = p1.x - p0.x;
    v1.y = p1.y - p0.y;
    v1.z = p1.z - p0.z; // 辺ベクトル v1 の計算
    v2.x = p2.x - p0.x;
    v2.y = p2.y - p0.y;
    v2.z = p2.z - p0.z;           // 辺ベクトル v2 の計算
    nv = normcrossprod(v1, v2);   // v1 と v2 の外積の計算
    glPushMatrix();               // 行列一時保存
    glNormal3d(nv.x, nv.y, nv.z); // 法線ベクトル設定
    glBegin(GL_TRIANGLES);        // 物体（三角形）頂点配置開始
    glVertex3d(p0.x, p0.y, p0.z); // 頂点 p0
    glVertex3d(p1.x, p1.y, p1.z); // 頂点 p1
    glVertex3d(p2.x, p2.y, p2.z); // 頂点 p2
    glEnd();                      // 物体頂点配置終了
    glPopMatrix();                // 行列復帰

    // トールビヨン用のライトの設定

    GLfloat lightpos1[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT1, GL_POSITION, lightpos1);
    GLfloat col1[4], spe1[4], shi1[1]; // 材質設定用変数
    // 物体 1（球）
    col1[0] = 1.0;
    col1[1] = 0.0;
    col1[2] = 0.0;
    col1[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe1[0] = 1.0;
    spe1[1] = 1.0;
    spe1[2] = 1.0;
    spe1[3] = 1.0;                                                 // 鏡面反射係数
    shi1[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col1); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe1);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi1);           // ハイライト係数を設定

    // トールビヨン
    glPushMatrix(); // 現在のモデルビュー変換行列をスタックに保存
    glScaled(0.05, 0.05, 0.05);
    glTranslated(0.0, 60.0, 0.0);
    // glTranslated(20.0, -10.0, 0.0);  //平行移動
    // glScaled(0.5, 1.0, 1.0);  //拡大縮小
    // glRotated(30.0, 0.0, 0.0, 1.0);  //回転

    //----------図形0 (座標軸)----------
    // ＊＊＊モデルビュー変換行列は視点情報のみ（ビューイング変換そのもの）＊＊＊
    // 図形定義

    glPushMatrix();
    glTranslated(0.0, 10.0, 0.0);
    glScaled(2.0, 2.0, 2.0);
    glColor4d(1.0, 1.0, 0.0, 1.0);
    glutSolidOctahedron();
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.0, 10.0, 0.0);
    glScaled(5.0, 5.0, 5.0);
    glColor4d(0.59, 0.318, 0.929, 1.0);
    glutSolidIcosahedron();
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.0, 10.0, 0.0);
    glScaled(5.0, 5.0, 5.0);
    glColor4d(1.0, 1.0, 0.0, 1.0);

    // 3重ジャイロスコープ
    glPushMatrix();
    // 1フレーム1度回転
    static double eDeg = 0.0;
    glRotated(eDeg, 0.0, 1.0, 0.0);
    eDeg += 1.0;
    if (eDeg >= 360.0)
        eDeg = 0.0;
    glScaled(0.5, 0.5, 0.5);
    glColor4d(1.0, 0.0, 0.0, 1.0);
    glutSolidTorus(1.0, 10.0, 10, 10);

    /*
    // 支柱
    glPushMatrix();
    glTranslated(7.0, 0.0, 0.0);
    glScaled(5.0, 1.0, 1.0);
    glutWireCube(1.0);

    // 支柱
    glTranslated(-2.8, 0.0, 0.0);
    glScaled(1.0, 1.0, 1.0);
    glutWireCube(1.0);
    glPopMatrix();
    */

    // 2次円
    glPushMatrix();

    glRotated(eDeg, 1.0, 1.0, 0.0);
    glScaled(0.5, 0.5, 0.5);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidTorus(1.0, 10.0, 10, 10);

    // 三次円
    glPushMatrix();

    glRotated(eDeg, 0.0, 1.0, 0.0);
    glScaled(0.5, 0.5, 0.5);
    glColor4d(0.0, 0.0, 1.0, 1.0);
    glutSolidTorus(1.0, 10.0, 10, 10);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();

    // 木の葉っぱ用のライトの設定
    GLfloat lightpos2[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT2, GL_POSITION, lightpos2);
    GLfloat col2[4], spe2[4], shi2[1]; // 材質設定用変数
    // 物体 1（球）
    col2[0] = 0.0;
    col2[1] = 1.0;
    col2[2] = 0.0;
    col2[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe2[0] = 1.0;
    spe2[1] = 1.0;
    spe2[2] = 1.0;
    spe2[3] = 1.0;                                                 // 鏡面反射係数
    shi2[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col2); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe2);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi2);           // ハイライト係数を設定

    // 木の葉っぱ
    glPushMatrix();
    glScaled(0.5, 0.5, 0.5);
    glTranslated(0.0, 0.0, 2.0);

    glPushMatrix();
    glTranslated(5.0, 1.5, 0.0);
    glScaled(0.8, 0.8, 0.8);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidIcosahedron();
    glPopMatrix();

    // 木の幹用のライトの設定
    GLfloat lightpos3[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT3, GL_POSITION, lightpos3);
    GLfloat col3[4], spe3[4], shi3[1]; // 材質設定用変数
    // 物体 1（球）
    col3[0] = 0.45;
    col3[1] = 0.31;
    col3[2] = 0.19;
    col3[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe3[0] = 1.0;
    spe3[1] = 1.0;
    spe3[2] = 1.0;
    spe3[3] = 1.0;                                                 // 鏡面反射係数
    shi3[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col3); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe3);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi3);           // ハイライト係数を設定

    // 木の幹
    glPushMatrix();
    glTranslated(5.0, 0.5, 0.0);

    glScaled(0.5, 1.0, 0.5);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();

    // 木の幹用のライトの設定
    GLfloat lightpos4[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT4, GL_POSITION, lightpos4);
    GLfloat col4[4], spe4[4], shi4[1]; // 材質設定用変数
    // 物体 1（球）
    col4[0] = 0.45;
    col4[1] = 0.31;
    col4[2] = 0.19;
    col4[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe4[0] = 1.0;
    spe4[1] = 1.0;
    spe4[2] = 1.0;
    spe4[3] = 1.0;                                                 // 鏡面反射係数
    shi4[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col4); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe4);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi4);           // ハイライト係数を設定

    // 木の幹
    glPushMatrix();
    glScaled(0.5, 0.5, 0.5);
    glTranslated(0.0, 0.0, -2.0);
    glPushMatrix();
    glTranslated(5.0, 0.5, 0.0);

    glScaled(0.5, 1.0, 0.5);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidCube(1.0);
    glPopMatrix();

    // 木の葉っぱ用のライトの設定
    GLfloat lightpos5[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT5, GL_POSITION, lightpos5);
    GLfloat col5[4], spe5[4], shi5[1]; // 材質設定用変数
    // 物体 1（球）
    col5[0] = 0.0;
    col5[1] = 1.0;
    col5[2] = 0.0;
    col5[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe5[0] = 1.0;
    spe5[1] = 1.0;
    spe5[2] = 1.0;
    spe5[3] = 1.0;                                                 // 鏡面反射係数
    shi5[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col5); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe5);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi5);           // ハイライト係数を設定

    // 木の葉っぱ
    glPushMatrix();
    glTranslated(5.0, 1.5, 0.0);
    glScaled(0.8, 0.8, 0.8);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidIcosahedron();
    glPopMatrix();
    glPopMatrix();

    // オリジナルオブジェクトのモデリング
    // 怪盗グルーのミニオンを描画
    // 全体を上にずらす
    glPushMatrix(); // 図形３の変換
    glScaled(0.1, 0.1, 0.1);
    glTranslated(-30.0, 0.0, 0.0);
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 10.0, 0.0);
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 10.0, 0.0);
    glScaled(5.0, 5.0, 5.0);

    //黄色のライトの設定
    GLfloat lightpos6[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT6, GL_POSITION, lightpos6);
    GLfloat col6[4], spe6[4], shi6[1]; // 材質設定用変数
    // 物体 1（球）
    col6[0] = 1.0;
    col6[1] = 0.86;
    col6[2] = 0.0;
    col6[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe6[0] = 1.0;
    spe6[1] = 1.0;
    spe6[2] = 1.0;
    spe6[3] = 1.0;                                                 // 鏡面反射係数
    shi6[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col6); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe6);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi6);           // ハイライト係数を設定


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

    //黒いライトの設定
    GLfloat lightpos7[] = {0.0, 0.0, 0.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT7, GL_POSITION, lightpos7);
    GLfloat col7[4], spe7[4], shi7[1]; // 材質設定用変数
    // 物体 1（球）
    col7[0] = 0.0;
    col7[1] = 0.0;
    col7[2] = 0.0;
    col7[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe7[0] = 1.0;
    spe7[1] = 1.0;
    spe7[2] = 1.0;
    spe7[3] = 1.0;                                                 // 鏡面反射係数
    shi7[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col7); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe7);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi7);           // ハイライト係数を設定

    // 黒い円柱を描画
    glColor4d(0.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 8.0, 0.0);
    cylinder(5.2, 1.0, 24);
    glPopMatrix();
    // 黒い円柱ここまで
    
    //灰色のマテリアルを生成
    GLfloat col8[4], spe8[4], shi8[1]; // 材質設定用変数
    // 物体 1（球）
    col8[0] = 0.5;
    col8[1] = 0.5;
    col8[2] = 0.5;
    col8[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe8[0] = 1.0;
    spe8[1] = 1.0;
    spe8[2] = 1.0;
    spe8[3] = 1.0;                                                 // 鏡面反射係数
    shi8[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col8); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe8);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi8);           // ハイライト係数を設定

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

    //白のマテリアルを生成
    GLfloat col9[4], spe9[4], shi9[1]; // 材質設定用変数
    // 物体 1（球）
    col9[0] = 0.8;
    col9[1] = 0.64;
    col9[2] = 0.37;
    col9[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe9[0] = 1.0;
    spe9[1] = 1.0;
    spe9[2] = 1.0;
    spe9[3] = 1.0;                                                 // 鏡面反射係数
    shi9[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col9); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe9);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi9);           // ハイライト係数を設定

    // 白の円柱の中に円柱描画
    glColor4d(0.8, 0.64, 0.37, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glTranslated(0.0, 8.0, 5.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    cylinder(0.8, 1.2, 24);
    glPopMatrix();
    // 白の円柱の中に円柱描画ここまで
    glPopMatrix(); // 眼帯の変換終わり

    // 青いマテリアル作成
    GLfloat col10[4], spe10[4], shi10[1]; // 材質設定用変数
    // 物体 1（球）
    col10[0] = 0.23;
    col10[1] = 0.33;
    col10[2] = 1.0;
    col10[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe10[0] = 1.0;
    spe10[1] = 1.0;
    spe10[2] = 1.0;
    spe10[3] = 1.0;                                                 // 鏡面反射係数
    shi10[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col10); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe10);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi10);           // ハイライト係数を設定

    // 青い球(下半身)を描画
    glColor4d(0.23, 0.33, 1.0, 1.0); // 色の設定（RGBA値）
    glPushMatrix();
    glScaled(5.0, 5.0, 5.0);
    glTranslated(0.0, 0.2, 0.0);
    glutSolidSphere(1.0, 24, 12);
    glPopMatrix();
    // 青い球を描画ここまで
    glPopMatrix(); // 図形３の変換終わり

    //マテリアルを生成
    GLfloat col11[4], spe11[4], shi11[1]; // 材質設定用変数
    // 物体 1（球）
    col11[0] = 0.23;
    col11[1] = 0.33;
    col11[2] = 1.0;
    col11[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe11[0] = 1.0;
    spe11[1] = 1.0;
    spe11[2] = 1.0;
    spe11[3] = 1.0;                                                 // 鏡面反射係数
    shi11[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col11); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe11);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi11);           // ハイライト係数を設定


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

    // 黒いマテリアルを生成
    GLfloat col12[4], spe12[4], shi12[1]; // 材質設定用変数
    // 物体 1（球）
    col12[0] = 0.0;
    col12[1] = 0.0;
    col12[2] = 0.0;
    col12[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe12[0] = 1.0;
    spe12[1] = 1.0;
    spe12[2] = 1.0;
    spe12[3] = 1.0;                                                 // 鏡面反射係数
    shi12[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col12); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe12);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi12);           // ハイライト係数を設定

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

    // マテリアルを生成
    GLfloat col13[4], spe13[4], shi13[1]; // 材質設定用変数
    // 物体 1（球）
    col13[0] = 1.0;
    col13[1] = 0.86;
    col13[2] = 0.0;
    col13[3] = 1.0; // 拡散反射係数，環境光反射係数
    spe13[0] = 1.0;
    spe13[1] = 1.0;
    spe13[2] = 1.0;
    spe13[3] = 1.0;                                                 // 鏡面反射係数
    shi13[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col13); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe13);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi13);           // ハイライト係数を設定

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
    glPopMatrix();

    glutSwapBuffers(); // 描画実行
}

// 2 ベクトルの外積計算
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;
    out.x = v1.y * v2.z - v1.z * v2.y;
    out.y = v1.z * v2.x - v1.x * v2.z;
    out.z = v1.x * v2.y - v1.y * v2.x;
    vectorNormalize(&out);
    return out; // 戻り値は外積ベクトル
}
// ベクトル正規化
double vectorNormalize(Vec_3D *vec)
{
    double len;
    // ベクトル長
    len = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    if (len > 0)
    {
        // 正規化
        vec->x = vec->x / len;
        vec->y = vec->y / len;
        vec->z = vec->z / len;
    }
    return len; // 戻り値はベクトル長
}
// リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h); // ビューポート設定
    // 投影変換の設定
    glMatrixMode(GL_PROJECTION);                              // 変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();                                         // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 1000.0); // 透視投影ビューボリューム設定
    winW = w;
    winH = h; // ウィンドウサイズ保存
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
