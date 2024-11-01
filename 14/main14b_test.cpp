/* コンパイルコマンド
g++ main14b_test.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GLUT/glut.h> //OpenGL/GLUT
#include <math.h>      //数学関数

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

// 定数宣言
#define TILE 50 // 床頂点格子分割数

// 関数名の宣言
void initGL();
void display0();                                 // ウィンドウ0用ディスプレイコールバック関数
void reshape0(int w, int h);                     // ウィンドウ0用リシェイプコールバック関数
void display1();                                 // ウィンドウ1用ディスプレイコールバック関数
void reshape1(int w, int h);                     // ウィンドウ1用リシェイプコールバック関数
void timer(int value);                           // タイマーコールバック関数
void keyboard(unsigned char key, int x, int y);  // キーボードコールバック関数
void mouse(int button, int state, int x, int y); // マウスコールバック関数
void motion(int x, int y);                       // マウスモーションコールバック関数
void scene();                                    // シーン描画関数
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);      // 二つのベクトルの外積の正規化
double vectorNormalize(Vec_3D *vec);             // ベクトルの正規化
Vec_3D vectorDiff(Vec_3D v1, Vec_3D v2);         // ベクトルの差分

// グローバル変数
double eDist, eDegX, eDegY;                                     // 視点の位置
int mX, mY, mState, mButton;                                    // マウスの位置
int winW[2], winH[2];                                           // ウィンドウサイズ
Vec_3D fPoint[TILE][TILE];                                      // 床頂点
Vec_3D fNormal[TILE][TILE];                                     // 床頂点法線
double bottom = 0.0;                                            // 床面高さ
double fWidth = 1000.0;                                         // 床長
double fr = 30.0;                                               // フレームレート
Vec_3D objPos = {0.0, 10.0, 0.0}, objSize = {20.0, 20.0, 40.0}; // 物体の位置とサイズ
double objAngle = 0.0;                                          // 物体の方向（回転角度）
int winID[2];                                                   // ウィンドウID

// Catalina+retina
double rDisp = 1.0;

int main(int argc, char *argv[])
{
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
    glutMouseFunc(mouse);               // マウスコールバック関数
    glutMotionFunc(motion);             // マウスモーションコールバック関数
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

// ディスプレイコールバック関数
void display0()
{
    glMatrixMode(GL_MODELVIEW);                         // カレント行列の指定（モデルビュー変換行列を指定）
    glLoadIdentity();                                   // 行列の初期化
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面消去
    // シーン構築
    scene();

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
    default:
        break;
    }
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    // マウスボタンが押された
    if (state == GLUT_DOWN)
    {
        mX = x;
        mY = y;
        mState = state;
        mButton = button; // マウス情報保持
    }
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    GLdouble model[16], proj[16]; // 変換行列格納用
    GLint view[4];                // ビューポート設定格納用
    GLfloat winX, winY, winZ;     // ウィンドウ座標
    GLdouble objX, objY, objZ;    // ワールド座標

    //winH[0]をfloatに変換
    float winH0 = (float)winH[0];

    if (mButton == GLUT_RIGHT_BUTTON)
    { // 右ボタンドラッグのとき
        // マウスの移動量を角度変化量に変換
        eDegY = eDegY + (mX - x) * 0.5; // マウス横方向→水平角
        eDegX = eDegX + (y - mY) * 0.5; // マウス縦方向→垂直角
        if (eDegX > 89.0)
            eDegX = 89.0;
        if (eDegX < -89.0)
            eDegX = -89.0;
    }
    else if (mButton == GLUT_LEFT_BUTTON)
    { // 左ボタンドラッグのとき
        // マウス座標からウィンドウ座標の取得
        winX = x;
        winY = winH0 - y;                                                                     // x座標，y座標
        glReadPixels(winX * rDisp, winY * rDisp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); // z座標はデプス値

        // モデルビュー変換行列・投影変換行列・ビューポート設定取り出し
        glGetDoublev(GL_MODELVIEW_MATRIX, model); // モデルビュー変換行列
        glGetDoublev(GL_PROJECTION_MATRIX, proj); // 投影変換行列
        glGetIntegerv(GL_VIEWPORT, view);         // ビューポート設定
        for (int i = 0; i < 4; i++)
        {
            view[i] /= rDisp;
        }

        // ウィンドウ座標をワールド座標に変換
        gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);

        // 床面の各頂点とマウスワールド座標との距離の計算と床面変形
        double areaSize = 100.0; // 変形領域サイズ
        for (int j = 0; j < TILE; j++)
        {
            for (int i = 0; i < TILE; i++)
            {
                // 床面頂点とマウスワールド座標との距離の計算
                double len = sqrt(pow(fPoint[i][j].x - objX, 2.0) + pow(fPoint[i][j].y - objY, 2.0) + pow(fPoint[i][j].z - objZ, 2.0));
                // 床面頂点が変形領域内のとき，マウス座標からの距離に応じて（距離が小さいほど大きな変形量）床面頂点を盛り上げ
                if (len < areaSize)
                {
                    fPoint[i][j].y += (areaSize - len) * 0.05;
                }
            }
        }

        // 法線ベクトル計算
        Vec_3D v1, v2, nv, nv1, nv2, nv3, nv4; // 辺ベクトル，法線ベクトル
        for (int j = 1; j < TILE - 1; j++)
        {
            for (int i = 1; i < TILE - 1; i++)
            {
                // 頂点周囲の4つの三角形に基づく法線をそれぞれ計算
                v1 = vectorDiff(fPoint[i][j - 1], fPoint[i][j]);
                v2 = vectorDiff(fPoint[i - 1][j], fPoint[i][j]);
                nv1 = normcrossprod(v1, v2);
                v1 = vectorDiff(fPoint[i - 1][j], fPoint[i][j]);
                v2 = vectorDiff(fPoint[i][j + 1], fPoint[i][j]);
                nv2 = normcrossprod(v1, v2);
                v1 = vectorDiff(fPoint[i][j + 1], fPoint[i][j]);
                v2 = vectorDiff(fPoint[i + 1][j], fPoint[i][j]);
                nv3 = normcrossprod(v1, v2);
                v1 = vectorDiff(fPoint[i + 1][j], fPoint[i][j]);
                v2 = vectorDiff(fPoint[i][j - 1], fPoint[i][j]);
                nv4 = normcrossprod(v1, v2);
                // 4つの法線の和をその頂点の法線として決定
                nv.x = nv1.x + nv2.x + nv3.x + nv4.x;
                nv.y = nv1.y + nv2.y + nv3.y + nv4.y;
                nv.z = nv1.z + nv2.z + nv3.z + nv4.z;
                vectorNormalize(&nv);
                // 法線ベクトルを格納
                fNormal[i][j] = nv;
            }
        }
    }

    // マウス座標をグローバル変数に保存
    mX = x;
    mY = y;
}

// タイマーコールバック関数
void timer(int value)
{
    glutSetWindow(winID[1]);            // 設定対象ウィンドウ選択
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutSetWindow(winID[0]);            // 設定対象ウィンドウ選択
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定

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
}

//v1とv2の外積計算
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;  //戻り値用
    
    //外積各成分の計算
    out.x = v1.y*v2.z-v1.z*v2.y;
    out.y = v1.z*v2.x-v1.x*v2.z;
    out.z = v1.x*v2.y-v1.y*v2.x;
    //正規化
    vectorNormalize(&out);
    //戻り値
    return out;
}

//ベクトル正規化
double vectorNormalize(Vec_3D* vec)
{
    double len;
    
    //ベクトル長
    len = sqrt(pow(vec->x,2)+pow(vec->y,2)+pow(vec->z,2));
    //各成分をベクトル長で割る
    if (len>0) {
        vec->x /= len;
        vec->y /= len;
        vec->z /= len;
    }
    //戻り値
    return len;
}

//ベクトル差分
Vec_3D vectorDiff(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;
    
    out.x = v1.x-v2.x;
    out.y = v1.y-v2.y;
    out.z = v1.z-v2.z;
    
    return out;
}
