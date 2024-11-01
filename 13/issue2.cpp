// g++ -O3 main13.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUT

#define TILE 50  // 床頂点数
#define FMAX 600 // データフレーム数

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
void drawString(char *str, int w, int h, int x0, int y0);
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
Vec_3D jData[FMAX][1500];    // 座標データ
Vec_3D jVel[FMAX][1500];     // 座標点の移動速度
float dt = 0.01;             // 時間刻み幅
int fID = 0;                 // フレームID
int trailFlag = 0;               // 残像フラグ

// 色の配列
GLfloat colors[14][4] = {
    // 座標に応じて色を変える
    //中心点に近いほど白、遠いほど赤
    {1.0, 0.0, 0.0, 1.0},   // 赤
    {1.0, 0.5, 0.0, 1.0},   // オレンジ
    {1.0, 1.0, 0.0, 1.0},   // 黄色
    {0.5, 1.0, 0.0, 1.0},   // 黄緑
    {0.0, 1.0, 0.0, 1.0},   // 緑
    {0.0, 1.0, 0.5, 1.0},   // 水緑
    {0.0, 1.0, 1.0, 1.0},   // 水色
    {0.0, 0.5, 1.0, 1.0},   // 青緑
    {0.0, 0.0, 1.0, 1.0},   // 青
    {0.498, 0.0, 1.0, 1.0}, // 薄紫
    {1.0, 0.0, 1.0, 1.0},   // 紫
    {1.0, 0.0, 0.498, 1.0}, // ピンク
    {0.5, 0.5, 0.5, 1.0},   // 灰色
    {1.0, 1.0, 1.0, 1.0}    // 白
};
int colorID = 0; // 色ID


// 残像の描画
    void drawTrail()
    {
        // 残像の数
        int trailCount = 5;

        if (trailFlag == 0){
            trailCount = 0;
        }
        // 残像の描画
        for (int i = 0; i < trailCount; i++)
        {
            // 残像の透明度
            GLfloat alpha = 0.1 * (5- i);

            // 残像の色
            GLfloat trailCol[4] = {1.0, 1.0, 1.0, alpha};

            // 残像の描画
            for (int j = 0; j < 1500; j++)
            {
                glPushMatrix();
                glTranslated(jData[(fID - i + FMAX) % FMAX][j].x, jData[(fID - i + FMAX) % FMAX][j].y, jData[(fID - i + FMAX) % FMAX][j].z - 0.0);
                glScaled(30.0, 30.0, 30.0);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, trailCol);
                glutSolidSphere(1.0, 36, 18);
                glPopMatrix();
            }
        }
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
    fp = fopen("posdata.txt", "r"); // ファイルオープン
    for (int j = 0; j < FMAX; j++)
    { // FMAX フレーム分
        for (int i = 0; i < 1500; i++)
        { // 15 カ所の座標
            // ファイルから j フレーム目の座標 i の読み込み
            fscanf(fp, "%lf,%lf,%lf\n", &jData[j][i].x, &jData[j][i].y, &jData[j][i].z);
            // 表示用に z 座標を補正
            jData[j][i].z -= 0.0;
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

    // Draw point

    for (int i = 0; i < 1500; i++)
    {
        //座標に基づいて色をつける
        //中心点に近いほどcolorIDが大きく、遠いほどcolorIDが小さい
        colorID = (int)(vectorLen(jData[fID][i]) / 100.0);
        if (colorID > 13)
            colorID = 13;
        // 質感設定
        GLfloat jointCol[4], jointSpe[4], jointShi[1];
        jointCol[0] = colors[colorID][0];
        jointCol[1] = colors[colorID][1];
        jointCol[2] = colors[colorID][2];
        jointCol[3] = colors[colorID][3];
        jointSpe[0] = 1.0;
        jointSpe[1] = 1.0;
        jointSpe[2] = 1.0;
        jointSpe[3] = 1.0;
        jointShi[0] = 50;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, jointCol);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, jointSpe);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, jointShi);
        glPushMatrix();
        glTranslated(jData[fID][i].x, jData[fID][i].y, jData[fID][i].z - 0.0);
        glScaled(30.0, 30.0, 30.0);
        glutSolidSphere(1.0, 36, 18);
        glPopMatrix();

        //座標点の移動速度を計算
        if (fID > 0)
        {
            jVel[fID][i].x = (jData[fID][i].x - jData[fID - 1][i].x) / dt;
            jVel[fID][i].y = (jData[fID][i].y - jData[fID - 1][i].y) / dt;
            jVel[fID][i].z = (jData[fID][i].z - jData[fID - 1][i].z) / dt;
        }
        //移動速度を可視化
        //矢印の先端の座標
        Vec_3D arrowEnd;
        arrowEnd.x = jData[fID][i].x + jVel[fID][i].x * 0.1;
        arrowEnd.y = jData[fID][i].y + jVel[fID][i].y * 0.1;
        arrowEnd.z = jData[fID][i].z + jVel[fID][i].z * 0.1;
        //矢印の根元は座標点と同じ
        Vec_3D arrowRoot;
        arrowRoot.x = jData[fID][i].x;
        arrowRoot.y = jData[fID][i].y;
        arrowRoot.z = jData[fID][i].z;
        //矢印の描画
        glPushMatrix();
        glTranslated(jData[fID][i].x, jData[fID][i].y, jData[fID][i].z - 0.0);
        glScaled(30.0, 30.0, 30.0);
        glBegin(GL_LINES);
        glVertex3d(arrowRoot.x, arrowRoot.y, arrowRoot.z);
        glVertex3d(arrowEnd.x, arrowEnd.y, arrowEnd.z);
        glEnd();
        glPopMatrix();

    }

    char str[256];
    sprintf(str, "The outer line is a visualization of the moving speed, and the length of the line represents the length of the speed vector.");
    drawString(str, winW, winH, 10, 10); // 文字列描画

    char str2[256];
    sprintf(str2, "Trail_mode: %d", trailFlag);
    drawString(str2, winW, winH, 10, 30); // 文字列描画
    // 残像の描画
    drawTrail();

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
    //スペースキーで残像の描画を切り替え
    case ' ':
        trailFlag = (trailFlag + 1) % 2;
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

void drawString(char *str, int w, int h, int x0, int y0)
{
    // 画面上に文字列を描画する関数
    // str: 文字列
    // w, h: ウィンドウサイズ
    // x0, y0: 文字列の左下の座標
    int i;
    glDisable(GL_LIGHTING);      // ライティングの無効化
    glDisable(GL_DEPTH_TEST);    // デプスバッファの無効化
    glMatrixMode(GL_PROJECTION); // 行列モードの設定
    glPushMatrix();              // 行列の保存
    glLoadIdentity();            // 単位行列の設定
    gluOrtho2D(0, w, 0, h);      // 正射影の設定
    glMatrixMode(GL_MODELVIEW);  // 行列モードの設定
    glPushMatrix();              // 行列の保存
    glLoadIdentity();            // 単位行列の設定
    glColor3d(1.0, 1.0, 1.0);    // 色の設定
    glRasterPos2i(x0, y0);       // ラスタ位置の設定
    for (i = 0; str[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]); // 文字の描画
    }
    glPopMatrix();               // 行列の復元
    glMatrixMode(GL_PROJECTION); // 行列モードの設定
    glPopMatrix();               // 行列の復元
    glMatrixMode(GL_MODELVIEW);  // 行列モードの設定
    glEnable(GL_DEPTH_TEST);     // デプスバッファの有効化
    glEnable(GL_LIGHTING);       // ライティングの有効化
}