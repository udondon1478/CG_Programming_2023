// g++ main05.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUT

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

// グローバル変数
int mButton, mState, mX, mY;

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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE); // ディスプレイ表示モード指定
    glutInitWindowSize(800, 500);                 // ウィンドウサイズの指定（800×500）
    glutCreateWindow("CG");                       // ウィンドウ生成
    // コールバック関数指定
    glutDisplayFunc(display);    // ディスプレイコールバック関数（"display"）
    glutReshapeFunc(reshape);    // リシェイプコールバック関数（"reshape"）
    glutTimerFunc(33, timer, 0); // タイマーコールバック関数（"timer"）
    glutMouseFunc(mouse);        // マウスクリックコールバック関数の指定（"mouse()"）
    glutMotionFunc(motion);      // マウスドラッグコールバック関数の指定（"motion()"）
    // 各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0); // ウィンドウクリア色の指定（RGBA値）
}

// リシェイプコールバック関数(w：ウィンドウ幅，h：ウィンドウ高さ)
void reshape(int w, int h)
{
    glViewport(0, 0, w, h); // ビューポート設定（ウィンドウ全体を表示領域に設定）
    // 投影変換
    glMatrixMode(GL_PROJECTION); // カレント行列の設定（投影変換行列）
    glLoadIdentity();            // カレント行列（投影変換行列）初期化
    // gluOrtho2D(-50.0, 50.0, -50.0*h/w, 50.0*h/w);  //平行投影による投影変換（z:-1~1）
    // glOrtho(-50.0, 50.0, -50.0*h/w, 50.0*h/w, -100.0, 100.0);  //平行投影による投影変換（z:-100~100）
    gluPerspective(30.0, 1.0 * w / h, 1.0, 1000.0); // 透視投影による投影変換（z:1~100）
}

// ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // ウィンドウクリア

    glLineWidth(2); // 線の太さ

    glMatrixMode(GL_MODELVIEW); // カレント行列の設定（モデルビュー変換行列）
    glLoadIdentity();           // カレント行列（モデルビュー変換行列）初期化

    // gluLookAt(20.0, 30.0, 100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //視点の設定（視点座標，視線方向，上方向）

    static double eDegY = 0.0;                                                                                                                  // 視点の仰角
    gluLookAt(100 * cos(M_PI / 6.0) * sin(eDegY), 100.0 * sin(M_PI / 6.0), 100.0 * cos(M_PI / 6.0) * cos(eDegY), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点の設定（視点座標，視線方向，上方向）
    eDegY += 0.01;

    glPushMatrix(); // 現在のモデルビュー変換行列をスタックに保存
    // glTranslated(20.0, -10.0, 0.0);  //平行移動
    // glScaled(0.5, 1.0, 1.0);  //拡大縮小
    // glRotated(30.0, 0.0, 0.0, 1.0);  //回転

    //----------図形0 (座標軸)----------
    // ＊＊＊モデルビュー変換行列は視点情報のみ（ビューイング変換そのもの）＊＊＊
    // 図形定義
    glPushMatrix();                // 現在のモデルビュー変換行列をスタックに保存
    glBegin(GL_LINES);             // 図形（複数の直線）の開始
    glColor4d(1.0, 0.0, 0.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(-20.0, 0.0, 20.0);  // 頂点座標0
    glVertex3d(20.0, 0.0, 20.0);   // 頂点座標1

    glColor4d(0.0, 0.0, 1.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(-20.0, 0.0, 20.0);  // 頂点座標2
    glVertex3d(-20.0, 0.0, -20.0); // 頂点座標3

    glColor4d(0.0, 1.0, 0.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(-20.0, 0.0, -20.0); // 頂点座標4
    glVertex3d(20.0, 0.0, -20.0);  // 頂点座標5

    glColor4d(1.0, 1.0, 1.0, 1.0); // 色の設定（RGBA値）
    glVertex3d(20.0, 0.0, -20.0);  // 頂点座標6
    glVertex3d(20.0, 0.0, 20.0);   // 頂点座標7
    glEnd();                       // 図形の終了
    glPopMatrix();                 // スタックからモデルビュー変換行列を取り出し

    glPushMatrix();
    glTranslated(0.0, 10.0, 0.0);
    glScaled(2.0, 2.0, 2.0);
    glColor4d(1.0, 1.0, 0.0, 1.0);
    glutWireOctahedron();
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.0, 10.0, 0.0);
    glScaled(5.0, 5.0, 5.0);
    glColor4d(0.59, 0.318, 0.929, 1.0);
    glutWireIcosahedron();
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
    glutWireTorus(1.0, 10.0, 10, 10);
    
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

    //2次円
    glPushMatrix();

    glRotated(eDeg, 1.0, 1.0, 0.0);
    glScaled(0.5, 0.5, 0.5);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutWireTorus(1.0, 10.0, 10, 10);

    //三次円
    glPushMatrix();

    glRotated(eDeg, 0.0, 1.0, 0.0);
    glScaled(0.5, 0.5, 0.5);
    glColor4d(0.0, 0.0, 1.0, 1.0);
    glutWireTorus(1.0, 10.0, 10, 10);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();



    glutSwapBuffers(); // OpenGLの命令実行
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
