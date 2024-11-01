// g++ -O3 main13-3.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUT

#define TILE 50  // 床頂点数
#define FMAX 300 // データフレーム数

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
void drawCharacter();
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
    fp = fopen("pdata3.txt", "r"); // ファイルオープン
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
    drawCharacter();

    glPushMatrix();
    glTranslated(500.0, 0.0, 0.0);
    drawCharacter();
    glPopMatrix();

    glPushMatrix();
    glTranslated(-500.0, 0.0, 0.0);
    drawCharacter();
    glPopMatrix();

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

void drawCharacter()
{
    // Draw joints
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

    // 黄色のぴえん
    glPushMatrix();
    // 頭
    glTranslated(jData[fID][0].x, jData[fID][0].y, jData[fID][0].z - 0.0);
    glScaled(150.0, 150.0, 150.0);
    glutSolidSphere(1.0, 36, 18);

    glRotated(90, 1.0, 0.0, 0.0);
    GLfloat eyeColor[] = {0.9, 0.6, 0.3, 1.0}; // コーンの色
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor);
    glutSolidCone(1.0, 5.0, 36, 100); // 円錐

    glPushMatrix();
    // 右目
    glTranslated(-0.5, -0.8, 0.0); // 目の位置を調整
    glScaled(-0.3, -0.3, -0.3);    // 目のサイズを変更
    // 材質プロパティの設定
    GLfloat eyeColor1[] = {0.0, 0.0, 0.0, 1.0}; // 黒
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor1);
    glutSolidSphere(1.0, 36, 18);

    // 右目　大きい
    glTranslated(-0.2, 0.7, 0.25); // 目の位置を調整
    glScaled(0.5, 0.5, 0.5);       // 目のサイズを変更
    // 材質プロパティの設定
    GLfloat eyeColor2[] = {1.0, 1.0, 1.0, 1.0}; // 白
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor2);
    glutSolidSphere(1.0, 36, 18);

    // 右目　小さい
    glTranslated(1.5, 0.3, -0.4); // 目の位置を調整
    glScaled(0.5, 0.5, 0.5);      // 目のサイズを変更
    // 材質プロパティの設定
    GLfloat eyeColor3[] = {1.0, 1.0, 1.0, 1.0}; // 白
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor3);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();

    glPushMatrix();
    // 左目
    glTranslated(0.5, -0.8, 0.0); // 目の位置を調整
    glScaled(-0.3, -0.3, -0.3);   // 目のサイズを変更
    // 材質プロパティの設定
    GLfloat eyeColor4[] = {0.0, 0.0, 0.0, 1.0}; // 黒
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor4);
    glutSolidSphere(1.0, 36, 18);

    // 左目　大きい
    glTranslated(0.2, 0.7, 0.25); // 目の位置を調整
    glScaled(0.5, 0.5, 0.5);      // 目のサイズを変更
    // 材質プロパティの設定
    GLfloat eyeColor5[] = {1.0, 1.0, 1.0, 1.0}; // 白
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor5);
    glutSolidSphere(1.0, 36, 18);

    // 左目　小さい
    glTranslated(-1.5, 0.3, -0.4); // 目の位置を調整
    glScaled(0.5, 0.5, 0.5);       // 目のサイズを変更
    // 材質プロパティの設定
    GLfloat eyeColor6[] = {1.0, 1.0, 1.0, 1.0}; // 白
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor6);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();

    glPopMatrix();

    // 手足
    // 5
    glPushMatrix();
    glTranslated(jData[fID][5].x, jData[fID][5].y, jData[fID][5].z - 0.0);
    glScaled(20.0, 20.0, 20.0);
    // 材質プロパティの設定
    GLfloat eyeColor7[] = {0.9, 0.6, 0.3, 1.0}; // コーンの色
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, eyeColor7);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();

    // 8
    glPushMatrix();
    glTranslated(jData[fID][8].x, jData[fID][8].y, jData[fID][8].z - 0.0);
    glScaled(20.0, 20.0, 20.0);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();

    // 11
    glPushMatrix();
    glTranslated(jData[fID][11].x, jData[fID][11].y, jData[fID][13].z - 0.0);
    glScaled(20.0, 20.0, 20.0);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();

    // 14
    glPushMatrix();
    glTranslated(jData[fID][14].x, jData[fID][14].y, jData[fID][14].z - 0.0);
    glScaled(20.0, 20.0, 20.0);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();
    // 手足終わり

    // Connect spheres 0 and 9 with a line
    glBegin(GL_LINES);
    glVertex3d(jData[fID][0].x, jData[fID][0].y, jData[fID][0].z - 0.0);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z - 0.0);
    glEnd();
    // 1-3
    glBegin(GL_LINES);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z);
    glEnd();

    // 1-6
    glBegin(GL_LINES);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z);
    glEnd();

    // 3-4
    glBegin(GL_LINES);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z);
    glVertex3d(jData[fID][4].x, jData[fID][4].y, jData[fID][4].z - 0);
    glEnd();

    // 6-7
    glBegin(GL_LINES);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z);
    glVertex3d(jData[fID][7].x, jData[fID][7].y, jData[fID][7].z - 0.0);
    glEnd();

    // 4-5
    glBegin(GL_LINES);
    glVertex3d(jData[fID][4].x, jData[fID][4].y, jData[fID][4].z - 0.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y, jData[fID][5].z - 0.0);
    glEnd();

    // 7-8
    glBegin(GL_LINES);
    glVertex3d(jData[fID][7].x, jData[fID][7].y, jData[fID][7].z - 0.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y, jData[fID][8].z - 0.0);
    glEnd();

    // 3-2
    glBegin(GL_LINES);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z - 0.0);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glEnd();

    // 6-2
    glBegin(GL_LINES);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z - 0.0);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glEnd();

    // 2-9
    glBegin(GL_LINES);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z - 0.0);
    glEnd();

    // 2-12
    glBegin(GL_LINES);
    glVertex3d(jData[fID][2].x, jData[fID][2].y, jData[fID][2].z - 0.0);
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glEnd();

    // 9-10
    glBegin(GL_LINES);
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z - 0.0);
    glVertex3d(jData[fID][10].x, jData[fID][10].y, jData[fID][10].z - 0.0);
    glEnd();

    // 10-11
    glBegin(GL_LINES);
    glVertex3d(jData[fID][10].x, jData[fID][10].y, jData[fID][10].z - 0.0);
    glVertex3d(jData[fID][11].x, jData[fID][11].y, jData[fID][11].z - 0.0);
    glEnd();

    // 12-13
    glBegin(GL_LINES);
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glVertex3d(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glEnd();

    // 13-14
    glBegin(GL_LINES);
    glVertex3d(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glVertex3d(jData[fID][14].x, jData[fID][14].y, jData[fID][14].z - 0.0);
    glEnd();
}