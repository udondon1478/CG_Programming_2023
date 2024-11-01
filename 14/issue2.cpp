// g++ -O3 issue2.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUTの使用

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

// 定数宣言
#define TILE 50 // 床頂点格子分割数

// 関数名の宣言
void initGL();
void display0();
void reshape0(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void display1();
void reshape1(int w, int h);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
void scene();
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
double vectorNormalize(Vec_3D *vec);
Vec_3D vectorDiff(Vec_3D v1, Vec_3D v2);

// グローバル変数
double eDist, eDegX, eDegY;                         // 視点極座標
double eDist2, eDegX2, eDegY2;                      // 視点極座標
int mX, mY, mState, mButton;                        // マウス座標
int winW[2], winH[2];                               // ウィンドウサイズ
int winID[2];                                       // ウィンドウID
Vec_3D fPoint[TILE][TILE];                          // 床頂点
Vec_3D fNormal[TILE][TILE];                         // 床頂点法線
double bottom = 0.0;                                // 床面高さ
double fWidth = 1000.0;                             // 床長
double fr = 30;                                     // フレームレート
Vec_3D objPos = {0, 10, 0}, objSize = {20, 20, 40}; // 物体の位置とサイズ
Vec_3D objDir = {0, 0, 0};                          // 物体の向き
double objAngle = 0.0;                              // 物体の方向（回転角度）
int direction = 0;
double rad = 0.0; // ラジアン
float deltaX = 0.0;
float deltaZ = 0.0;



// Catalina+retina
double rDisp = 1.0;

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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH); // ディスプレイモードの指定
    glutInitWindowSize(640, 480);                // ウィンドウサイズの指定
    glutInitWindowPosition(0, 0);                // ウィンドウ位置の指定
    winID[0] = glutCreateWindow("CG0");          // ウィンドウの生成

    // コールバック関数の指定
    glutDisplayFunc(display0);          // ディスプレイコールバック関数の指定（"display0()"）
    glutReshapeFunc(reshape0);          // リサイズコールバック関数の指定（"reshape0()"）
    glutMouseFunc(mouse);               // マウスコールバック関数の指定（"mouse()"）
    glutMotionFunc(motion);             // マウス移動コールバック関数の指定（"motion()"）
    glutKeyboardFunc(keyboard);         // キーボードコールバック関数の指定（"keyboard()"）
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバック関数（"timer"）

    // 描画ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH); // ディスプレイモードの指定
    glutInitWindowSize(640, 480);                // ウィンドウサイズの指定
    glutInitWindowPosition(640, 0);              // ウィンドウ位置の指定
    winID[1] = glutCreateWindow("CG1");          // ウィンドウの生成

    // コールバック関数の指定
    glutDisplayFunc(display1);  // ディスプレイコールバック関数の指定（"display1()"）
    glutReshapeFunc(reshape1);  // リサイズコールバック関数の指定（"reshape1()"）
    glutKeyboardFunc(keyboard); // キーボードコールバック関数の指定（"keyboard()"）

    for (int i = 0; i < 2; i++)
    {
        glutSetWindow(winID[i]);
        // 各種設定
        glClearColor(0.0, 0.0, 0.2, 1.0);                  // ウィンドウクリア色の指定（RGBA）
        glEnable(GL_DEPTH_TEST);                           // デプスバッファの有効化
        glEnable(GL_NORMALIZE);                            // 法線ベクトル正規化の有効化
        glEnable(GL_BLEND);                                // アルファブレンディング有効化
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ブレンディング関数指定

        // 光源設定
        GLfloat colGA[] = {0.2, 0.2, 0.2, 1.0};                  // パラメータ設定
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, colGA);           // グローバル環境光の設定
        glEnable(GL_LIGHTING);                                   // 陰影付けの有効化
        glEnable(GL_LIGHT0);                                     // 光源0の有効化
        GLfloat col[] = {1.0, 1.0, 1.0, 1.0};                    // パラメータ設定
        glLightfv(GL_LIGHT0, GL_DIFFUSE, col);                   // 光源0の拡散反射に関する強度
        glLightfv(GL_LIGHT0, GL_SPECULAR, col);                  // 光源0の鏡面反射に関する強度
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.000002); // 光源0の二次減衰率の設定

        // 視点関係
        eDist = 1600.0;
        eDegX = 10.0;
        eDegY = 0.0; // 視点極座標

        // 床頂点座標
        for (int i = 0; i < TILE; i++)
        {
            for (int j = 0; j < TILE; j++)
            {
                fPoint[i][j].x = -fWidth * 0.5 + fWidth / (TILE - 1) * i; // x座標
                fPoint[i][j].y = bottom;                                  // y座標(高さ)
                fPoint[i][j].z = -fWidth * 0.5 + fWidth / (TILE - 1) * j; // z座標
                fNormal[i][j].x = 0.0;                                    // 法線x成分
                fNormal[i][j].y = 1.0;                                    // 法線y成分
                fNormal[i][j].z = 0.0;                                    // 法線z成分
            }
        }
    }
}

void scene0()
{
    // 視点座標の計算
    Vec_3D e;
    e.x = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    e.y = eDist * sin(eDegX * M_PI / 180.0);
    e.z = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);

    // モデルビュー変換の設定
    glMatrixMode(GL_MODELVIEW);                             // 変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();                                       // 行列初期化
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点視線設定（視野変換行列を乗算）

    // 光源0の配置
    GLfloat lightPos0[] = {100.0, 600.0, 50.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);    // 光源座標

    // 描画準備
    GLfloat col[4], spe[4], shi[1]; // 材質設定用配列

    //----------床----------
    // 色の設定
    col[0] = 0.0;
    col[1] = 0.7;
    col[2] = 0.7;
    col[3] = 1.0; // 拡散反射係数
    spe[0] = 1.0;
    spe[1] = 1.0;
    spe[2] = 1.0;
    spe[3] = 1.0;                                                 // 鏡面反射係数
    shi[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射，環境光反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト設定
    // 描画
    glBegin(GL_QUADS);
    for (int j = 0; j < TILE - 1; j++)
    {
        for (int i = 0; i < TILE - 1; i++)
        {
            // 頂点法線・座標
            glNormal3d(fNormal[i][j].x, fNormal[i][j].y, fNormal[i][j].z);
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z);
            glNormal3d(fNormal[i][j + 1].x, fNormal[i][j + 1].y, fNormal[i][j + 1].z);
            glVertex3d(fPoint[i][j + 1].x, fPoint[i][j + 1].y, fPoint[i][j + 1].z);
            glNormal3d(fNormal[i + 1][j + 1].x, fNormal[i + 1][j + 1].y, fNormal[i + 1][j + 1].z);
            glVertex3d(fPoint[i + 1][j + 1].x, fPoint[i + 1][j + 1].y, fPoint[i + 1][j + 1].z);
            glNormal3d(fNormal[i + 1][j].x, fNormal[i + 1][j].y, fNormal[i + 1][j].z);
            glVertex3d(fPoint[i + 1][j].x, fPoint[i + 1][j].y, fPoint[i + 1][j].z);
        }
    }
    glEnd(); // 配置終了

    // 床網目
    glDisable(GL_LIGHTING);        // シェーディング無効化
    glColor4d(1.0, 1.0, 0.0, 0.5); // 色指定
    for (int j = 0; j < TILE - 1; j++)
    {
        for (int i = 0; i < TILE - 1; i++)
        {
            glBegin(GL_LINE_LOOP); // 閉じた折線を描画
            // 頂点座標
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y + 1.0, fPoint[i][j].z);
            glVertex3d(fPoint[i][j + 1].x, fPoint[i][j + 1].y + 1.0, fPoint[i][j + 1].z);
            glVertex3d(fPoint[i + 1][j + 1].x, fPoint[i + 1][j + 1].y + 1.0, fPoint[i + 1][j + 1].z);
            glVertex3d(fPoint[i + 1][j].x, fPoint[i + 1][j].y + 1.0, fPoint[i + 1][j].z);
            glEnd();
        }
    }
    glEnable(GL_LIGHTING); // シェーディング有効化

    // オブジェクト
    // 色の設定
    col[0] = 1.0;
    col[1] = 0.2;
    col[2] = 0.2;
    col[3] = 1.0; // 拡散反射係数
    spe[0] = 1.0;
    spe[1] = 1.0;
    spe[2] = 1.0;
    spe[3] = 1.0;                                                 // 鏡面反射係数
    shi[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射，環境光反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト設定
    glPushMatrix();
    glTranslated(objPos.x, objPos.y, objPos.z); // 位置
    glRotated(objAngle, 0.0, 1.0, 0.0);         // 方向
    glScaled(objSize.x, objSize.y, objSize.z);  // サイズ
    glutSolidCube(1.0);
    // キューブの向きと位置に視点を追従させる
    glPopMatrix();
}

void scene1()
{
    // モデルビュー変換の設定
    glMatrixMode(GL_MODELVIEW); // 変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();           // 行列初期化
    //視点はオブジェクトに追従、三角関数を用いて、オブジェクトを中心とし、objAngleの方向に長さ2の線をまっすぐ引いた際のX座標とZ座標を求める方法で視点を求める
    
    

    // 光源0の配置
    GLfloat lightPos0[] = {100.0, 600.0, 50.0, 1.0}; // 点光源
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);    // 光源座標

    // 描画準備
    GLfloat col[4], spe[4], shi[1]; // 材質設定用配列

    //----------床----------
    // 色の設定
    col[0] = 0.0;
    col[1] = 0.7;
    col[2] = 0.7;
    col[3] = 1.0; // 拡散反射係数
    spe[0] = 1.0;
    spe[1] = 1.0;
    spe[2] = 1.0;
    spe[3] = 1.0;                                                 // 鏡面反射係数
    shi[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射，環境光反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト設定
    // 描画
    glBegin(GL_QUADS);
    for (int j = 0; j < TILE - 1; j++)
    {
        for (int i = 0; i < TILE - 1; i++)
        {
            // 頂点法線・座標
            glNormal3d(fNormal[i][j].x, fNormal[i][j].y, fNormal[i][j].z);
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z);
            glNormal3d(fNormal[i][j + 1].x, fNormal[i][j + 1].y, fNormal[i][j + 1].z);
            glVertex3d(fPoint[i][j + 1].x, fPoint[i][j + 1].y, fPoint[i][j + 1].z);
            glNormal3d(fNormal[i + 1][j + 1].x, fNormal[i + 1][j + 1].y, fNormal[i + 1][j + 1].z);
            glVertex3d(fPoint[i + 1][j + 1].x, fPoint[i + 1][j + 1].y, fPoint[i + 1][j + 1].z);
            glNormal3d(fNormal[i + 1][j].x, fNormal[i + 1][j].y, fNormal[i + 1][j].z);
            glVertex3d(fPoint[i + 1][j].x, fPoint[i + 1][j].y, fPoint[i + 1][j].z);
        }
    }
    glEnd(); // 配置終了

    // 床網目
    glDisable(GL_LIGHTING);        // シェーディング無効化
    glColor4d(1.0, 1.0, 0.0, 0.5); // 色指定
    for (int j = 0; j < TILE - 1; j++)
    {
        for (int i = 0; i < TILE - 1; i++)
        {
            glBegin(GL_LINE_LOOP); // 閉じた折線を描画
            // 頂点座標
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y + 1.0, fPoint[i][j].z);
            glVertex3d(fPoint[i][j + 1].x, fPoint[i][j + 1].y + 1.0, fPoint[i][j + 1].z);
            glVertex3d(fPoint[i + 1][j + 1].x, fPoint[i + 1][j + 1].y + 1.0, fPoint[i + 1][j + 1].z);
            glVertex3d(fPoint[i + 1][j].x, fPoint[i + 1][j].y + 1.0, fPoint[i + 1][j].z);
            glEnd();
        }
    }
    glEnable(GL_LIGHTING); // シェーディング有効化

    // オブジェクト
    // 色の設定
    col[0] = 1.0;
    col[1] = 0.2;
    col[2] = 0.2;
    col[3] = 1.0; // 拡散反射係数
    spe[0] = 1.0;
    spe[1] = 1.0;
    spe[2] = 1.0;
    spe[3] = 1.0;                                                 // 鏡面反射係数
    shi[0] = 100.0;                                               // ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射，環境光反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト設定
    glPushMatrix();
    glTranslated(objPos.x, objPos.y, objPos.z); // 位置
    glRotated(objAngle, 0.0, 1.0, 0.0);         // 方向
    glScaled(objSize.x, objSize.y, objSize.z);  // サイズ
    glutSolidCube(1.0);
    // キューブの向きと位置に視点を追従させる
    glPushMatrix();
    gluLookAt(0.0, 0.0, 0.0, objPos.x, objPos.y, objPos.z, 0.0, 1.0, 0.0);
    glPopMatrix();
    glPopMatrix();
}

// ディスプレイコールバック関数
void display0()
{
    glMatrixMode(GL_MODELVIEW);                         // 変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();                                   // 行列初期化
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面消去
    // シーン構築
    scene0();

    glutSwapBuffers(); // 描画実行
}

// リシェイプコールバック関数
void reshape0(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ウィンドウ内の描画領域(ビューポート)の指定
    // 投影変換の設定
    glMatrixMode(GL_PROJECTION);                               // 変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();                                          // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); // 透視投影ビューボリューム設定

    winW[0] = w;
    winH[0] = h; // ウィンドウサイズ保持
}

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
    GLdouble objX, objY, objZ;    // オブジェクト座標

    // 右ボタンドラッグで視点移動
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        // マウスの移動量を角度変化量に変換
        eDegY = eDegY + (mX - x) * 0.5; // マウス横方向→水平角
        eDegX = eDegX + (mY - y) * 0.5; // マウス縦方向→垂直角
        // 視点移動
        if (eDegX > 89.0)
            eDegX = 89.0; // 上限
        if (eDegX < -89.0)
            eDegX = -89.0; // 下限
    }
    else if (mButton == GLUT_LEFT_BUTTON)
    {
        // マウス座標からウィンドウ座標の取得
        winX = (GLdouble)x;
        winY = (GLdouble)(winH[0] - y);
        glReadPixels(winX * rDisp, winY * rDisp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

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

        // 床面の各頂点とマウスワールド座標との座標の計算と床面変形
        double areaSize = 100.0; // 変形領域サイズ
        for (int j = 0; j < TILE; j++)
        {
            for (int i = 0; i < TILE; i++)
            {
                // 床面頂点とマウスワールド座標との距離の計算
                double len = sqrt(pow(fPoint[i][j].x - objX, 2.0) + pow(fPoint[i][j].y - objY, 2.0) + pow(fPoint[i][j].z - objZ, 2.0));
                // 床面頂点が変形領域内の時、マウス座標からの距離に応じて床面頂点を盛り上げ
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
void display1()
{
    glMatrixMode(GL_MODELVIEW);                         // 変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();                                   // 行列初期化
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面消去
    // シーン構築
    scene1();

    glutSwapBuffers(); // 描画実行
}

void reshape1(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ウィンドウ内の描画領域(ビューポート)の指定
    // 投影変換の設定
    glMatrixMode(GL_PROJECTION);                               // 変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();                                          // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); // 透視投影ビューボリューム設定

    winW[1] = w;
    winH[1] = h; // ウィンドウサイズ保持
}

// キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
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
    glutSetWindow(winID[1]); // 設定対象ウィンドウ選択
    glutPostRedisplay();     // ディスプレイイベント強制発生
    glutSetWindow(winID[1]);
    glutSetWindow(winID[0]);            // 設定対象ウィンドウ選択
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定

    // オブジェクトの移動と回転
    // case1の時はobjAngleの向いている向きに対して前に進む
    // case2の時はobjAngleの向いている向きに対して後ろに進む
    // case3の時は右に回転
    // case4の時は左に回転
    // angleの値に基づいて進行方向を変える
    switch (direction)
    {
        case 1:
        rad = objAngle * M_PI / 180;
        objPos.x = objPos.x + sin(rad);
        objPos.z = objPos.z + cos(rad);
        break;
        case 3:
        rad = objAngle * M_PI / 180;
        objPos.x = objPos.x - sin(rad);
        objPos.z = objPos.z - cos(rad);
        break;
        case 2:
        objAngle = objAngle + 5;
        break;
        case 4:
        objAngle = objAngle - 5;
        break;
    }
    //現在の向きの取得
    rad = objAngle * M_PI / 180;
    // オブジェクトの回転
    objDir.x = sin(rad);
    objDir.z = cos(rad);

    deltaX = 3 * cos(objAngle);
    deltaZ = 3 * sin(objAngle);
}

// v1とv2の外積計算
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out; // 戻り値用

    // 外積各成分の計算
    out.x = v1.y * v2.z - v1.z * v2.y;
    out.y = v1.z * v2.x - v1.x * v2.z;
    out.z = v1.x * v2.y - v1.y * v2.x;
    // 正規化
    vectorNormalize(&out);
    // 戻り値
    return out;
}

// ベクトル正規化
double vectorNormalize(Vec_3D *vec)
{
    double len;

    // ベクトル長
    len = sqrt(pow(vec->x, 2) + pow(vec->y, 2) + pow(vec->z, 2));
    // 各成分をベクトル長で割る
    if (len > 0)
    {
        vec->x /= len;
        vec->y /= len;
        vec->z /= len;
    }
    // 戻り値
    return len;
}

// ベクトル差分
Vec_3D vectorDiff(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;

    out.x = v1.x - v2.x;
    out.y = v1.y - v2.y;
    out.z = v1.z - v2.z;

    return out;
}
