// g++ -O3 issue1.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUT
#include "box.h"
#include "opencv2/opencv.hpp"

#define TILE 50  // 床頂点数
#define FMAX 300 // データフレーム数
#define numCubes 10

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
void drawFrameNum();
void drawEffectRight();
void cylinder(float radius, float height, int sides);

void drawCoreUnit();
void drawHip();
void drawLfootJointCylinder();
void drawLThigh();
void drawRfootJointCylinder();
void drawRThigh();
void drawLCalf();
void drawRCalf();
void drawLfootJoint();
void drawRfootJoint();

void drawLshoulderJointCylinder();
void drawRshoulderJointCylinder();
void drawLUpperArm();
void drawRUpperArm();
void drawLForeArm();
void drawRForeArm();

Vec_3D crossProduct(Vec_3D vec1, Vec_3D vec2);
Vec_3D diffVec(Vec_3D vec1, Vec_3D vec2);
double vectorLen(Vec_3D vec);
Vec_3D vectorNormalize(Vec_3D vec);

// グローバル変数
Vec_3D fPoint[TILE][TILE];   // 床頂点
double fWidth = 5000.0;      // 床長
double eDist, eDegX, eDegY;  // 視点極座標
int winW, winH;              // ウィンドウサイズ
int mButton, mState, mX, mY; // マウス情報
double f = 30.0;             // フレームレート
Vec_3D jData[FMAX][15];      // 座標データ
int fID = 0;                 // フレームID

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
    glutInitWindowSize(1024, 640);                             // ウィンドウサイズの指定
    glutCreateWindow("CG-13");                                 // ウィンドウ生成

    // コールバック関数指定
    glutDisplayFunc(display);          // ディスプレイコールバック関数（"display"）
    glutReshapeFunc(reshape);          // リサイズコールバック関数（"reshape"）
    glutTimerFunc(1000 / f, timer, 0); // タイマーコールバック関数（"timer", 1000/fミリ秒）
    glutMouseFunc(mouse);              // マウスクリックコールバック関数
    glutMotionFunc(motion);            // マウスドラッグコールバック関数
    glutKeyboardFunc(keyboard);        // キーボードコールバック関数

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

    // 視点極座標
    eDist = 7000.0; // 距離
    eDegX = 20.0;
    eDegY = 180.0; // x軸周り角度，y軸周り角度

    // 床頂点座標
    for (int j = 0; j < TILE; j++)
    {
        for (int i = 0; i < TILE; i++)
        {
            fPoint[i][j].x = -fWidth / 2.0 + i * fWidth / (TILE - 1);
            fPoint[i][j].y = -800.0;
            fPoint[i][j].z = -fWidth / 2.0 + j * fWidth / (TILE - 1);
        }
    }

    // 座標データ読み込み
    FILE *fp;
    fp = fopen("pdata2.txt", "r"); // ファイルオープン
    for (int j = 0; j < FMAX; j++)
    { // FMAX フレーム分
        for (int i = 0; i < 15; i++)
        { // 15 カ所の座標
            // ファイルから j フレーム目の座標 i の読み込み
            fscanf(fp, "%lf,%lf,%lf\n", &jData[j][i].x, &jData[j][i].y, &jData[j][i].z);
            // 表示用に z 座標を補正
            jData[j][i].z -= 2000.0;
            printf("%f, %f, %f\n", jData[j][i].x, jData[j][i].y, jData[j][i].z);
        }
        printf("--------------------------\n");
    }
    fclose(fp);

cv::Mat textureImage; //テクスチャ画像格納用
    //テクスチャ １ 作成
textureImage = cv::imread("gundam.png", cv::IMREAD_UNCHANGED); //画像読み込み
glBindTexture(GL_TEXTURE_2D, 1); //テクスチャ#0
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //テクスチャ拡大時の補間方法 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //テクスチャ縮小時の補間方法 
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.cols, textureImage.rows, 0, GL_BGRA,GL_UNSIGNED_BYTE, textureImage.data); //画像をテクスチャに割り当て}
}

// リサイズコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h); // ウィンドウ内の描画領域(ビューポート)の指定

    // 投影変換
    glMatrixMode(GL_PROJECTION);                               // カレント行列の設定
    glLoadIdentity();                                          // カレント行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 20000.0); // 投影変換行列生成

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
    GLfloat lightPos0[] = {200.0, 2000.0, -200.0, 1.0}; // 光源座標(点光源)
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);       // 光源配置

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

    //----------座標点----------

    // 関節の描画
    GLfloat jointCol[4], jointSpe[4], jointShi[1];
    jointCol[0] = 1.0;
    jointCol[1] = 1.0;
    jointCol[2] = 0.0;
    jointCol[3] = 1.0;
    jointSpe[0] = 1.0;
    jointSpe[1] = 1.0;
    jointSpe[2] = 1.0;
    jointSpe[3] = 1.0;
    jointShi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, jointCol);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, jointSpe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, jointShi);

    for (int i = 0; i < 15; i++)
    {
        glPushMatrix();
        glTranslated(jData[fID][i].x, jData[fID][i].y, jData[fID][i].z - 0.0);
        glScaled(30.0, 30.0, 30.0);
        glutSolidSphere(1.0, 36, 18);
        /*
        glPushMatrix();
        glBegin(GL_QUADS);
        glVertex3d(jData[fID][i].x, jData[fID][i].y, jData[fID][i].z - 0.0);
        glVertex3d(jData[fID+1][i].x, jData[fID+1][i].y, jData[fID+1][i].z - 0.0);
        glVertex3d(jData[fID+2][i].x, jData[fID+2][i].y, jData[fID+2][i].z - 0.0);
        glVertex3d(jData[fID+3][i].x, jData[fID+3][i].y, jData[fID+3][i].z - 0.0);
        glEnd();
        glPopMatrix();
        */
        glPopMatrix();
    }

    // Connect spheres 0 and 9 with a line

        //物体3（長方形パネル2）
    glEnable(GL_ALPHA_TEST); //アルファテスト有効化
    glEnable(GL_TEXTURE_2D); //テクスチャ有効化
    glBindTexture(GL_TEXTURE_2D, 1); //テクスチャオブジェクト指定(#1)
    glPushMatrix();  //行列一時保存
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;  //RGBA
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
   
    glNormal3d(0.0, 0.0, 1.0);  //法線
    glBegin(GL_QUADS);  //四角形
    
    glTexCoord2d(0.0, 0.0); //テクスチャ座標0
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z - 0.0);
    glTexCoord2d(0.0, 1.0); //テクスチャ座標0
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glTexCoord2d(1.0, 1.0); //テクスチャ座標0
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z - 0.0);
    glTexCoord2d(1.0, 0.0); //テクスチャ座標0
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z - 0.0);
    glEnd();
    glPopMatrix();  //行列復帰
    glDisable(GL_TEXTURE_2D); //テクスチャ無効化
    glDisable(GL_ALPHA_TEST); //アルファテスト無効化

    glDepthMask(GL_TRUE);

    glBegin(GL_LINES);
    glVertex3d(jData[fID][0].x, jData[fID][0].y, jData[fID][0].z - 0.0);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z - 0.0);
    glEnd();
    // 1-3
    // 1-3に追従させる立方体を描画
    // 左肩
    glBegin(GL_LINES);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z);
    glEnd();

    // 1-6
    // 右肩
    glBegin(GL_LINES);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z);
    glEnd();

    // 3-4
    // 左腕
    glBegin(GL_LINES);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z);
    glVertex3d(jData[fID][4].x, jData[fID][4].y, jData[fID][4].z - 0);
    glEnd();

    // 6-7
    // 右腕
    glBegin(GL_LINES);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z);
    glVertex3d(jData[fID][7].x, jData[fID][7].y, jData[fID][8].z - 0.0);
    glEnd();

    // 4-5
    // 左前腕
    glBegin(GL_LINES);
    glVertex3d(jData[fID][4].x, jData[fID][4].y, jData[fID][4].z - 0.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y, jData[fID][5].z - 0.0);
    glEnd();

    // 7-8
    // 右前腕

    glBegin(GL_LINES);
    glVertex3d(jData[fID][7].x, jData[fID][7].y, jData[fID][7].z - 0.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y, jData[fID][8].z - 0.0);
    glEnd();

    // 3-2
    // 左脇腹

    glBegin(GL_LINES);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z - 0.0);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glEnd();

    // 6-2
    // 右脇腹
    glBegin(GL_LINES);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z - 0.0);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glEnd();

    // 2-9
    // 左股関節
    glBegin(GL_LINES);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z - 0.0);
    glEnd();

    // 2-12
    // 右股関節
    glBegin(GL_LINES);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glEnd();

    // 9-10
    // 左太もも

    glBegin(GL_LINES);
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z - 0.0);
    glVertex3d(jData[fID][10].x, jData[fID][10].y, jData[fID][10].z - 0.0);
    glEnd();

    // 左太ももの正面
    // 色変更

    /*
    glPushMatrix();
    glTranslated(jData[fID][9].x + jData[fID][10].x / 2, jData[fID][9].y + jData[fID][10].y / 2 -100, jData[fID][9].z + jData[fID][10].z / 2);
    glRotated(45,1,0,0);
    glScaled(5, 5, 5);
    makebox(30, 30, 50);
    glPopMatrix();
    */

    // 10-11
    // 右太もも
    glBegin(GL_LINES);
    glVertex3d(jData[fID][10].x, jData[fID][10].y, jData[fID][10].z - 0.0);
    glVertex3d(jData[fID][11].x, jData[fID][11].y, jData[fID][11].z - 0.0);
    glEnd();

    // 12-13
    // 左脹脛
    glBegin(GL_LINES);
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glVertex3d(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glEnd();

    // 13-14
    // 右脹脛
    glBegin(GL_LINES);
    glVertex3d(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glVertex3d(jData[fID][14].x, jData[fID][14].y, jData[fID][14].z - 0.0);
    glEnd();

    glPushMatrix();
    //drawCoreUnit();
    glPushMatrix();
    
    glPopMatrix();

    
    //drawHip();

    glPushMatrix();
    glTranslated(-100, 0, 0);
    drawLfootJointCylinder();

    glPushMatrix();
    // 現在の法線を取得する
    GLfloat normal[3];
    glGetFloatv(GL_CURRENT_NORMAL, normal);

    // 法線を反転させる
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];

    // 反転された法線を設定する
    glNormal3fv(normal);

    drawLThigh();

    glPopMatrix();
    glPushMatrix();
    drawLCalf();

    drawLfootJoint();
    glPopMatrix();

    glPopMatrix();

    glPushMatrix();
    // 左膝にcylinderを描画
    glTranslated(jData[fID][10].x - 100, jData[fID][10].y, jData[fID][10].z);
    glRotated(90, 0, 0, 1);
    glRotated(90, 0, 1, 0);
    glScaled(30, 30, 30);
    cylinder(2, 2, 20);
    glPopMatrix();

    // 右股関節にcylinderを描画
    glPushMatrix();
    glTranslated(jData[fID][12].x + 100, jData[fID][12].y, jData[fID][12].z);
    glRotated(90, 0, 0, 1);
    glRotated(90, 0, 1, 0);
    glScaled(30, 30, 30);
    cylinder(2, 2, 20);
    glPopMatrix();

    // 右ひざにcylinderを描画
    glPushMatrix();
    glTranslated(jData[fID][13].x + 100, jData[fID][13].y, jData[fID][13].z);
    glRotated(90, 0, 0, 1);
    glRotated(90, 0, 1, 0);
    glScaled(30, 30, 30);
    cylinder(2, 2, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslated(100, 0, 0);
    drawRfootJointCylinder();

    glPushMatrix();

    // 現在の法線を取得する

    glGetFloatv(GL_CURRENT_NORMAL, normal);

    // 法線を反転させる
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];

    // 反転された法線を設定する
    glNormal3fv(normal);

    drawRThigh();

    glPopMatrix();
    glPushMatrix();
    drawRCalf();
    drawRfootJoint();
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();

    drawLshoulderJointCylinder();
    glPopMatrix();

    // 現在の法線を取得する

    glGetFloatv(GL_CURRENT_NORMAL, normal);

    // 法線を反転させる
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];

    // 反転された法線を設定する
    glNormal3fv(normal);
    drawLUpperArm();
    drawLForeArm();
    drawRshoulderJointCylinder();

    // 現在の法線を取得する

    glGetFloatv(GL_CURRENT_NORMAL, normal);

    // 法線を反転させる
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];

    // 反転された法線を設定する
    glNormal3fv(normal);
    drawRUpperArm();
    drawRForeArm();

    drawFrameNum(); // 現在のフレーム数を描画
    /*
    if (fID == 66)
    {
        drawEffectRight(); // エフェクト描画
    }
    */

    // 描画バッファ切り替え
    glutSwapBuffers();
}

// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();               // ディスプレイイベント強制発生
    glutTimerFunc(1000 / f, timer, 0); // タイマー再設定
    fID = (fID + 1) % FMAX;            // フレームIDを増やし、必要に応じてラップアラウンド
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
    glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

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
    if (mButton == GLUT_LEFT_BUTTON)
    {
        eDegY += (mX - x) * 0.2; // マウス横方向→水平角
        eDegX += (y - mY) * 0.2; // マウス縦方向→垂直角
        if (eDegX > 85)
            eDegX = 85.0;
        if (eDegX < -85)
            eDegX = -85.0;
    }
    else if (mButton == GLUT_RIGHT_BUTTON)
    {
        eDist += (y - mY) * 10; // マウス縦方向→Zoom In/Out
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

// ベクトルの差の計算((v1-v2)を出力)
Vec_3D diffVec(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;

    out.x = v1.x - v2.x;
    out.y = v1.y - v2.y;
    out.z = v1.z - v2.z;

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

// 現在のフレーム数を描画する関数
void drawFrameNum()
{
    char str[256];
    sprintf(str, "Frame: %d", fID);
    glColor3d(1.0, 1.0, 1.0);
    glRasterPos2d(-winW / 2.0 + 10, winH / 2.0 - 20);
    for (int i = 0; str[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
    }
}

// 特定のフレーム[fID]で関節の座標を中心に円を描くように立方体を配置する関数
struct JointData
{
    float x;
    float y;
    float z;
};

void drawEffectRight()
{
    // jData[fID][13]の座標を中心に一辺50の立方体を描画
    // jData[fID][13]の座標を中心に一辺50の立方体を描画
    GLfloat effectCol[4], effectSpe[4], effectShi[1];
    effectCol[0] = 1.0;
    effectCol[1] = 0.0;
    effectCol[2] = 0.0;
    effectCol[3] = 1.0;
    effectSpe[0] = 1.0;
    effectSpe[1] = 1.0;
    effectSpe[2] = 1.0;
    effectSpe[3] = 1.0;
    effectShi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, effectCol);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, effectSpe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, effectShi);

    glPushMatrix();
    glTranslated(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glScaled(50.0, 50.0, 50.0);
    makebox(30, 30, 50);
    glPopMatrix();
}

// 左太ももの描画
void drawLThigh()
{
    // 左太ももの正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][9].x + 40, jData[fID][9].y, jData[fID][9].z - 40.0);
    glVertex3d(jData[fID][9].x - 40, jData[fID][9].y, jData[fID][9].z - 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glEnd();

    // 左太ももの裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][9].x + 40, jData[fID][9].y, jData[fID][9].z + 40.0);
    glVertex3d(jData[fID][9].x - 40, jData[fID][9].y, jData[fID][9].z + 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glEnd();

    // 左太ももの右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][9].x + 40, jData[fID][9].y, jData[fID][9].z + 40.0);
    glVertex3d(jData[fID][9].x + 40, jData[fID][9].y, jData[fID][9].z - 40.0);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glEnd();

    // 左太ももの左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][9].x - 40, jData[fID][9].y, jData[fID][9].z - 40.0);
    glVertex3d(jData[fID][9].x - 40, jData[fID][9].y, jData[fID][9].z + 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glEnd();
}

// 左脹脛の描画
void drawLCalf()
{
    // 左脹脛の正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glVertex3d(jData[fID][11].x - 40, jData[fID][11].y, jData[fID][11].z - 40.0);
    glVertex3d(jData[fID][11].x + 40, jData[fID][11].y, jData[fID][11].z - 40.0);
    glEnd();

    // 左脹脛の裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glVertex3d(jData[fID][11].x - 40, jData[fID][11].y, jData[fID][11].z + 40.0);
    glVertex3d(jData[fID][11].x + 40, jData[fID][11].y, jData[fID][11].z + 40.0);
    glEnd();

    // 左脹脛の右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glVertex3d(jData[fID][10].x + 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glVertex3d(jData[fID][11].x + 40, jData[fID][11].y, jData[fID][11].z - 40.0);
    glVertex3d(jData[fID][11].x + 40, jData[fID][11].y, jData[fID][11].z + 40.0);
    glEnd();

    // 左脹脛の左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z - 40.0);
    glVertex3d(jData[fID][10].x - 40, jData[fID][10].y, jData[fID][10].z + 40.0);
    glVertex3d(jData[fID][11].x - 40, jData[fID][11].y, jData[fID][11].z + 40.0);
    glVertex3d(jData[fID][11].x - 40, jData[fID][11].y, jData[fID][11].z - 40.0);
    glEnd();
}

// 右太ももの描画
void drawRThigh()
{
    // 右太ももの正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][12].x + 40, jData[fID][12].y, jData[fID][12].z - 40.0);
    glVertex3d(jData[fID][12].x - 40, jData[fID][12].y, jData[fID][12].z - 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glEnd();

    // 右太ももの裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][12].x + 40, jData[fID][12].y, jData[fID][12].z + 40.0);
    glVertex3d(jData[fID][12].x - 40, jData[fID][12].y, jData[fID][12].z + 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glEnd();

    // 右太ももの右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][12].x + 40, jData[fID][12].y, jData[fID][12].z + 40.0);
    glVertex3d(jData[fID][12].x + 40, jData[fID][12].y, jData[fID][12].z - 40.0);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glEnd();

    // 右太ももの左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][12].x - 40, jData[fID][12].y, jData[fID][12].z - 40.0);
    glVertex3d(jData[fID][12].x - 40, jData[fID][12].y, jData[fID][12].z + 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glEnd();
}

// 右脹脛の描画
void drawRCalf()
{

    // 右脹脛の正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glVertex3d(jData[fID][14].x - 40, jData[fID][14].y, jData[fID][14].z - 40.0);
    glVertex3d(jData[fID][14].x + 40, jData[fID][14].y, jData[fID][14].z - 40.0);
    glEnd();

    // 右脹脛の裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glVertex3d(jData[fID][14].x - 40, jData[fID][14].y, jData[fID][14].z + 40.0);
    glVertex3d(jData[fID][14].x + 40, jData[fID][14].y, jData[fID][14].z + 40.0);
    glEnd();

    // 右脹脛の右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glVertex3d(jData[fID][13].x + 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glVertex3d(jData[fID][14].x + 40, jData[fID][14].y, jData[fID][14].z - 40.0);
    glVertex3d(jData[fID][14].x + 40, jData[fID][14].y, jData[fID][14].z + 40.0);
    glEnd();

    // 右脹脛の左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z - 40.0);
    glVertex3d(jData[fID][13].x - 40, jData[fID][13].y, jData[fID][13].z + 40.0);
    glVertex3d(jData[fID][14].x - 40, jData[fID][14].y, jData[fID][14].z + 40.0);
    glVertex3d(jData[fID][14].x - 40, jData[fID][14].y, jData[fID][14].z - 40.0);
    glEnd();
}

void drawLfootJoint()
{
    // 左脹脛の先端の関節
    glPushMatrix();
    glTranslated(((jData[fID][11].x - 40) + (jData[fID][11].x + 40)) / 2, ((jData[fID][11].y) + (jData[fID][11].y)) / 2, ((jData[fID][11].z - 40.0) + (jData[fID][11].z + 40.0)) / 2);
    glScaled(30.0, 15.0, 50.0);
    glutSolidCube(3.0);
    glPushMatrix();
    glTranslated(0.0, -3.0, 0.0);
    glScaled(1.0, 1.5, 1.5);
    glutSolidCube(3.0);
    glPopMatrix();
    glPopMatrix();
}

void drawRfootJoint()
{
    // 右脹脛の先端の関節
    glPushMatrix();
    glTranslated(((jData[fID][14].x - 40) + (jData[fID][14].x + 40)) / 2, ((jData[fID][14].y) + (jData[fID][14].y)) / 2, ((jData[fID][14].z - 40.0) + (jData[fID][14].z + 40.0)) / 2);
    glScaled(30.0, 15.0, 50.0);
    glutSolidCube(3.0);
    glPushMatrix();
    glTranslated(0.0, -3.0, 0.0);
    glScaled(1.0, 1.5, 1.5);
    glutSolidCube(3.0);
    glPopMatrix();
    glPopMatrix();
}

void drawCoreUnit()
{
    glPushMatrix();
    glTranslated(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z);
    glScaled(30.0, 15.0, 30.0);
    glutSolidCube(12.0);
    glPushMatrix();
    glTranslated(0.0, 10.0, 0.0);
    glScaled(2,2,2);
    cylinder(2, 4, 20);
    //円柱を描画
    glPopMatrix();
    glPushMatrix();
    glTranslated(0.0, -3.0, 0.0);
    glScaled(0.6, 1.5, 0.6);
    glutSolidCube(12.0);
    glPopMatrix();
    glPopMatrix();
}

// 腰の部分
void drawHip()
{
    glPushMatrix();
    glTranslated(jData[fID][2].x, jData[fID][2].y - 80, jData[fID][2].z);
    glScaled(30.0, 10.0, 30.0);
    glutSolidCube(12.0);
    glPopMatrix();
}

// 左足のジョイントとして円柱を描画
void drawLfootJointCylinder()
{
    glPushMatrix();
    glTranslated(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z);
    glRotated(90, 0, 1, 0);
    glRotated(90, 1, 0, 0);
    glScaled(30.0, 30.0, 30.0);
    cylinder(2, 2, 20);
    glPopMatrix();
}

// 右足のジョイントとして円柱を描画
void drawRfootJointCylinder()
{
    glPushMatrix();
    glTranslated(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z);
    glRotated(90, 0, 1, 0);
    glRotated(90, 1, 0, 0);
    glScaled(30.0, 30.0, 30.0);
    cylinder(2, 2, 20);
    glPopMatrix();
}

// 左肩のジョイントとして円柱を描画
void drawLshoulderJointCylinder()
{
    glPushMatrix();
    glTranslated(jData[fID][3].x - 50, jData[fID][3].y, jData[fID][3].z);
    glRotated(90, 1, 0, 0);
    glScaled(30.0, 30.0, 30.0);
    cylinder(2, 2, 20);
    glPopMatrix();
}

// 右肩のジョイントとして円柱を描画
void drawRshoulderJointCylinder()
{
    glPushMatrix();
    glTranslated(jData[fID][6].x + 50, jData[fID][6].y, jData[fID][6].z);
    glRotated(90, 1, 0, 0);
    glScaled(30.0, 30.0, 30.0);
    cylinder(2, 2, 20);
    glPopMatrix();
}

// 左上腕の描画
void drawLUpperArm()
{
    // 左上腕の正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][3].x, jData[fID][3].y + 40, jData[fID][3].z - 40.0);
    glVertex3d(jData[fID][3].x, jData[fID][3].y + 40, jData[fID][3].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z - 40.0);
    glEnd();

    // 左上腕の裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][3].x, jData[fID][3].y - 40, jData[fID][3].z + 40.0);
    glVertex3d(jData[fID][3].x, jData[fID][3].y - 40, jData[fID][3].z - 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z - 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z + 40.0);
    glEnd();

    // 左上腕の右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][3].x, jData[fID][3].y + 40, jData[fID][3].z + 40.0);
    glVertex3d(jData[fID][3].x, jData[fID][3].y - 40, jData[fID][3].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z + 40.0);
    glEnd();

    // 左上腕の左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][3].x, jData[fID][3].y + 40, jData[fID][3].z - 40.0);
    glVertex3d(jData[fID][3].x, jData[fID][3].y - 40, jData[fID][3].z - 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z - 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z - 40.0);
    glEnd();
}

// 右上腕の描画
void drawRUpperArm()
{
    // 右上腕の正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][6].x, jData[fID][6].y + 40, jData[fID][6].z - 40.0);
    glVertex3d(jData[fID][6].x, jData[fID][6].y + 40, jData[fID][6].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z - 40.0);
    glEnd();

    // 右上腕の裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][6].x, jData[fID][6].y - 40, jData[fID][6].z + 40.0);
    glVertex3d(jData[fID][6].x, jData[fID][6].y - 40, jData[fID][6].z - 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z - 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z + 40.0);
    glEnd();

    // 右上腕の右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][6].x, jData[fID][6].y + 40, jData[fID][6].z + 40.0);
    glVertex3d(jData[fID][6].x, jData[fID][6].y - 40, jData[fID][6].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z + 40.0);
    glEnd();

    // 右上腕の左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][6].x, jData[fID][6].y + 40, jData[fID][6].z - 40.0);
    glVertex3d(jData[fID][6].x, jData[fID][6].y - 40, jData[fID][6].z - 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z - 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z - 40.0);
    glEnd();
}

// 左前腕の描画
void drawLForeArm()
{
    // 左前腕の正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z - 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y + 40, jData[fID][5].z - 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y + 40, jData[fID][5].z + 40.0);
    glEnd();

    // 左前腕の裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z - 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y - 40, jData[fID][5].z - 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y - 40, jData[fID][5].z + 40.0);
    glEnd();

    // 左前腕の右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z + 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z + 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y - 40, jData[fID][5].z + 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y + 40, jData[fID][5].z + 40.0);
    glEnd();

    // 左前腕の左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][4].x, jData[fID][4].y + 40, jData[fID][4].z - 40.0);
    glVertex3d(jData[fID][4].x, jData[fID][4].y - 40, jData[fID][4].z - 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y - 40, jData[fID][5].z - 40.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y + 40, jData[fID][5].z - 40.0);
    glEnd();
}

// 右前腕の描画
void drawRForeArm()
{
    // 右前腕の正面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z - 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y + 40, jData[fID][8].z - 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y + 40, jData[fID][8].z + 40.0);
    glEnd();

    // 右前腕の裏面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z - 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y - 40, jData[fID][8].z - 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y - 40, jData[fID][8].z + 40.0);
    glEnd();

    // 右前腕の右面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z + 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z + 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y - 40, jData[fID][8].z + 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y + 40, jData[fID][8].z + 40.0);
    glEnd();

    // 右前腕の左面
    glBegin(GL_QUADS);
    glVertex3d(jData[fID][7].x, jData[fID][7].y + 40, jData[fID][7].z - 40.0);
    glVertex3d(jData[fID][7].x, jData[fID][7].y - 40, jData[fID][7].z - 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y - 40, jData[fID][8].z - 40.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y + 40, jData[fID][8].z - 40.0);
    glEnd();

    // 左手の描画
    glPushMatrix();
    glTranslated(jData[fID][5].x, jData[fID][5].y, jData[fID][5].z);
    glScaled(30.0, 30.0, 30.0);
    glutSolidCube(3.0);
    glPopMatrix();
}

// 円柱を描画する関数
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