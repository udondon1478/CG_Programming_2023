// g++ main07.cpp -framework OpenGL -framework GLUT -Wno-deprecated
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
//光源設定
glEnable(GL_LIGHTING); //陰影付けの有効化
glEnable(GL_LIGHT0); //光源 0 の有効化
GLfloat col[] = { 1.0, 1.0, 1.0, 1.0 }; //パラメータ設定
glLightfv(GL_LIGHT1, GL_DIFFUSE, col); //光源 1 の拡散反射に関する強度
glLightfv(GL_LIGHT1, GL_SPECULAR, col); //光源 1 の鏡面反射に関する強度
glLightfv(GL_LIGHT1, GL_AMBIENT, col); //光源 1 の環境光反射に関する強度
glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.0); //光源 1 の一定減衰率の設定
glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05); //光源 1 の二次減衰率の設定

glLightfv(GL_LIGHT0, GL_DIFFUSE, col); //光源 1 の拡散反射に関する強度
glLightfv(GL_LIGHT0, GL_SPECULAR, col); //光源 1 の鏡面反射に関する強度
glLightfv(GL_LIGHT0, GL_AMBIENT, col); //光源 1 の環境光反射に関する強度
glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0); //光源 1 の一定減衰率の設定
glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05); //光源 1 の二次減衰率の設定
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
    glPushMatrix();               // 行列一時保存
    glBegin(GL_QUADS);            // 物体（四角形）頂点配置開始
    glVertex3d(-4.0, 0.0, -4.0); // 頂点
    glVertex3d(4.0, 0.0, -4.0);  // 頂点
    glVertex3d(4.0, 0.0, 4.0);   // 頂点
    glVertex3d(-4.0, 0.0, 4.0);  // 頂点
    glEnd();                      // 物体頂点配置終了
    glPopMatrix();                // 行列復帰

//三角形4つからなるピラミッド
Vec_3D p0, p1, p2; //頂点
p0.x = 0.0; p0.y = 2.0; p0.z = 0.3; //p0
p1.x = -2.0; p1.y = 0.0; p1.z = -2.0; //p1
p2.x = 2.0; p2.y = 0.0; p2.z = -2.0; //p2
Vec_3D v1, v2, nv; //辺ベクトル，法線ベクトル
v1.x = p1.x-p0.x; v1.y = p1.y-p0.y; v1.z = p1.z-p0.z; //辺ベクトル v1 の計算
v2.x = p2.x-p0.x; v2.y = p2.y-p0.y; v2.z = p2.z-p0.z; //辺ベクトル v2 の計算
nv = normcrossprod(v1, v2); //v1 と v2 の外積の計算
glPushMatrix(); //行列一時保存
glNormal3d(nv.x, nv.y, nv.z); //法線ベクトル設定
glBegin(GL_TRIANGLES); //物体（三角形）頂点配置開始
glVertex3d(p0.x, p0.y, p0.z); //頂点 p0
glVertex3d(p1.x, p1.y, p1.z); //頂点 p1
glVertex3d(p2.x, p2.y, p2.z); //頂点 p2
glEnd(); //物体頂点配置終了
glPopMatrix(); //行列復帰

p0.x = 0.0; p0.y = 2.0; p0.z = 0.3; //p0
p1.x = 2.0; p1.y = 0.0; p1.z = 2.0; //p1
p2.x = 2.0; p2.y = 0.0; p2.z = -2.0; //p2

v1.x = p1.x-p0.x; v1.y = p1.y-p0.y; v1.z = p1.z-p0.z; //辺ベクトル v1 の計算
v2.x = p2.x-p0.x; v2.y = p2.y-p0.y; v2.z = p2.z-p0.z; //辺ベクトル v2 の計算
nv = normcrossprod(v1, v2); //v1 と v2 の外積の計算
glPushMatrix(); //行列一時保存
glNormal3d(nv.x, nv.y, nv.z); //法線ベクトル設定
glBegin(GL_TRIANGLES); //物体（三角形）頂点配置開始
glVertex3d(p0.x, p0.y, p0.z); //頂点 p0
glVertex3d(p1.x, p1.y, p1.z); //頂点 p1
glVertex3d(p2.x, p2.y, p2.z); //頂点 p2
glEnd(); //物体頂点配置終了
glPopMatrix(); //行列復帰

p0.x = 0.0; p0.y = 2.0; p0.z = 0.3; //p0
p1.x = -2.0; p1.y = 0.0; p1.z = 2.0; //p1
p2.x = 2.0; p2.y = 0.0; p2.z = 2.0; //p2

v1.x = p1.x-p0.x; v1.y = p1.y-p0.y; v1.z = p1.z-p0.z; //辺ベクトル v1 の計算
v2.x = p2.x-p0.x; v2.y = p2.y-p0.y; v2.z = p2.z-p0.z; //辺ベクトル v2 の計算
nv = normcrossprod(v1, v2); //v1 と v2 の外積の計算
glPushMatrix(); //行列一時保存
glNormal3d(nv.x, nv.y, nv.z); //法線ベクトル設定
glBegin(GL_TRIANGLES); //物体（三角形）頂点配置開始
glVertex3d(p0.x, p0.y, p0.z); //頂点 p0
glVertex3d(p1.x, p1.y, p1.z); //頂点 p1
glVertex3d(p2.x, p2.y, p2.z); //頂点 p2
glEnd(); //物体頂点配置終了
glPopMatrix(); //行列復帰

p0.x = 0.0; p0.y = 2.0; p0.z = 0.3; //p0
p1.x = -2.0; p1.y = 0.0; p1.z = -2.0; //p1
p2.x = -2.0; p2.y = 0.0; p2.z = 2.0; //p2

v1.x = p1.x-p0.x; v1.y = p1.y-p0.y; v1.z = p1.z-p0.z; //辺ベクトル v1 の計算
v2.x = p2.x-p0.x; v2.y = p2.y-p0.y; v2.z = p2.z-p0.z; //辺ベクトル v2 の計算
nv = normcrossprod(v1, v2); //v1 と v2 の外積の計算
glPushMatrix(); //行列一時保存
glNormal3d(nv.x, nv.y, nv.z); //法線ベクトル設定
glBegin(GL_TRIANGLES); //物体（三角形）頂点配置開始
glVertex3d(p0.x, p0.y, p0.z); //頂点 p0
glVertex3d(p1.x, p1.y, p1.z); //頂点 p1
glVertex3d(p2.x, p2.y, p2.z); //頂点 p2
glEnd(); //物体頂点配置終了
glPopMatrix(); //行列復帰



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
    // マウスボタンが押されたとき，マウス座標をグローバル変数に保存
    if (state == GLUT_DOWN)
    {
        mX = x;
        mY = y;
        mButton = button;
        mState = state;
    }
}
// マウスドラッグコールバック関数
void motion(int x, int y)
{
    // マウスの移動量を角度変化量に変換
    eDegY = eDegY + (mX - x) * 0.5; // マウス横方向→水平角
    eDegX = eDegX + (y - mY) * 0.5; // マウス縦方向→垂直角
    // マウス座標をグローバル変数に保存
    mX = x;
    mY = y;
}
// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();         // ディスプレイイベント強制発生
    glutTimerFunc(33, timer, 0); // タイマー再設定
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
