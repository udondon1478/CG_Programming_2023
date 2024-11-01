// g++ -O3 main12.cpp -framework OpenGL -framework GLUT -Wno-deprecated
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
#define TILE 50    // 床頂点格子分割数
#define G -980.0   // 重力加速度
#define BALLNUM 10 // 球の個数

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
double vectorNormalize(Vec_3D *vec);
Vec_3D vectorDiff(Vec_3D v1, Vec_3D v2);

// グローバル変数
double eDist, eDegX, eDegY;  // 視点極座標
int mX, mY, mState, mButton; // マウス座標
int winW, winH;              // ウィンドウサイズ
Vec_3D fPoint[TILE][TILE];   // 床頂点
Vec_3D fNormal[TILE][TILE];  // 床頂点法線
double bottom = 0.0;         // 床面高さ
double fWidth = 1000.0;      // 床長
double r[BALLNUM];           // 球の半径
Vec_3D p[BALLNUM];           // 球の中心座標
Vec_3D v[BALLNUM];           // 球の速度
Vec_3D c[BALLNUM];           // 球の色
double fr = 30;              // フレームレート

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
    glutInitWindowSize(1280, 720);               // ウィンドウサイズの指定
    glutCreateWindow("CG_12");                   // ウィンドウの生成

    // コールバック関数の指定
    glutDisplayFunc(display);           // ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape);           // リサイズコールバック関数の指定（"reshape()"）
    glutMouseFunc(mouse);               // マウスクリックコールバック関数の指定（"mouse()"）
    glutMotionFunc(motion);             // マウスドラッグコールバック関数の指定（"motion()"）
    glutKeyboardFunc(keyboard);         // キーボードコールバック関数の指定（"keyboard()"）
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバック関数（"timer"）

    // 各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0); // ウィンドウクリア色の指定（RGBA）
    glEnable(GL_DEPTH_TEST);          // デプスバッファの有効化
    glEnable(GL_NORMALIZE);           // 法線ベクトル正規化の有効化

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

    // 球初期値
    srand((unsigned)time(NULL)); // 乱数初期化
    for (int k = 0; k < BALLNUM; k++)
    {
        r[k] = 25.0 * rand() / RAND_MAX + 25.0; // 半径（15.0〜35.0の乱数）
        p[k].x = 0.0;
        p[k].y = bottom + r[k];
        p[k].z = 0.0;                                // 初期座標
        v[k].x = 500.0 * rand() / RAND_MAX - 250.0;  // 初期速度x成分（-250.0〜255.0の乱数）
        v[k].y = 1000.0 * rand() / RAND_MAX + 500.0; // 初期速度y成分（500.0〜2000.0の乱数）
        v[k].z = 500.0 * rand() / RAND_MAX - 250.0;  // 初期速度z成分（-250.0〜255.0の乱数）
        c[k].x = 1.0;
        c[k].y = 1.0;
        c[k].z = 1.0;
    }
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

    //----------球----------
    // 速度更新
    double at1 = 0.9;  // はね返り係数
    double at2 = 0.99; // 転がり抵抗
    double at3 = 0.9;  // 球同士の衝突時の速度減衰

    for (int k = 0; k < BALLNUM; k++)
    {
        // 色の設定
        col[0] = c[k].x;
        col[1] = c[k].y;
        col[2] = c[k].z;
        col[3] = 1.0; // 拡散反射係数
        spe[0] = 1.0;
        spe[1] = 1.0;
        spe[2] = 1.0;
        spe[3] = 1.0;                                                 // 鏡面反射係数
        shi[0] = 100.0;                                               // ハイライト係数
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射，環境光反射設定
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射設定
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト設定
        // 配置
        glPushMatrix();                       // 変換行列の一時保存
        glTranslated(p[k].x, p[k].y, p[k].z); // 変換行列に平行移動行列を乗算
        glutSolidSphere(r[k], 36, 18);        // 球
        glPopMatrix();                        // 一時保存した行列の復帰

        // 球の位置更新
        p[k].x += v[k].x / fr;
        p[k].y += v[k].y / fr;
        p[k].z += v[k].z / fr;

        // x成分
        if (p[k].x < -fWidth / 2.0 + r[k])
        {
            v[k].x = -v[k].x * at1;        // 速度のx成分を反転
            p[k].x = -fWidth / 2.0 + r[k]; // めり込みを解消
        }
        if (p[k].x > fWidth / 2.0 - r[k])
        {
            v[k].x = -v[k].x * at1;       // 速度のx成分を反転
            p[k].x = fWidth / 2.0 - r[k]; // めり込みを解消
        }
        // z成分
        if (p[k].z < -fWidth / 2.0 + r[k])
        {
            v[k].z = -v[k].z * at1;        // 速度のz成分を反転
            p[k].z = -fWidth / 2.0 + r[k]; // めり込みを解消
        }
        if (p[k].z > fWidth / 2.0 - r[k])
        {
            v[k].z = -v[k].z * at1;       // 速度のz成分を反転
            p[k].z = fWidth / 2.0 - r[k]; // めり込みを解消
        }
        // y成分
        // if (p[k].y<bottom+r[k]) {
        //     v[k].y = -v[k].y*at1;  //速度のz成分を反転
        //     p[k].y = bottom+r[k];  //めり込みを解消
        // }

        //=====床面=====
        // 衝突判定
        double minLen = r[k]; // k 番目の球と床面との最小距離の初期値
        int minI, minJ;       // 最小距離となった格子番号保持用
        for (int j = 0; j < TILE; j++)
        {
            for (int i = 0; i < TILE; i++)
            {
                // k 番目の球と格子点[i][j]との距離 len を計算
                double len = sqrt(pow(p[k].x - fPoint[i][j].x, 2) + pow(p[k].y - fPoint[i][j].y, 2) + pow(p[k].z - fPoint[i][j].z, 2));
                // len が最小距離よりも小さかったら，最小距離とその格子番号を更新
                if (len < minLen)
                {
                    minLen = len;
                    minI = i;
                    minJ = j;
                }
            }
        }
        // 衝突発生時に速度更新
        if (minLen < r[k])
        { // 最小距離が球 k の半径未満→衝突発生
            // 衝突時の球 k の速度ベクトル v[k]と衝突格子点の法線ベクトル fNormal[minI][minJ]の内積 LN を計算
            double LN = v[k].x * fNormal[minI][minJ].x + v[k].y * fNormal[minI][minJ].y + v[k].z * fNormal[minI][minJ].z;
            // 速度ベクトルの正反射方向を求めて速度更新
            v[k].x = v[k].x - 2.0 * LN * fNormal[minI][minJ].x * at1;
            v[k].y = v[k].y - 2.0 * LN * fNormal[minI][minJ].y * at1;
            v[k].z = v[k].z - 2.0 * LN * fNormal[minI][minJ].z * at1;
            // 法線方向へのめり込み解消処理
            Vec_3D L0;
            L0.x = fNormal[minI][minJ].x * (r[k] - minLen);
            L0.y = fNormal[minI][minJ].y * (r[k] - minLen);
            L0.z = fNormal[minI][minJ].z * (r[k] - minLen);
            p[k].x += L0.x;
            p[k].y += L0.y;
            p[k].z += L0.z;
            // 転がりによる減衰
            v[k].x *= at2;
            v[k].z *= at2;
        }

        // 球同士の衝突判定と速度更新
        Vec_3D p0 = p[k]; // 現在の座標を一時保存
        for (int m = 0; m < BALLNUM; m++)
        {
            if (m != k)
            {
                // 球 k と球 m の衝突判定
                Vec_3D d = vectorDiff(p[m], p[k]); // 球同士の中心を結ぶベクトル
                if (vectorNormalize(&d) < r[k] + r[m])
                { // 球同士が衝突
                    //(vk・d - vm・d)
                    double dv = d.x * v[k].x + d.y * v[k].y + d.z * v[k].z - (d.x * v[m].x + d.y * v[m].y + d.z * v[m].z);
                    // 球同士がさらに接近する方向に移動中の場合
                    if (dv > 0.0)
                    {
                        // 双方の球の速度更新（減衰 at3 を考慮）
                        v[k].x = (v[k].x - d.x * dv) * at3;
                        v[k].y = (v[k].y - d.y * dv) * at3;
                        v[k].z = (v[k].z - d.z * dv) * at3;
                        v[m].x = (v[m].x + d.x * dv) * at3;
                        v[m].y = (v[m].y + d.y * dv) * at3;
                        v[m].z = (v[m].z + d.z * dv) * at3;
                        // 球の位置の補正（衝突前の位置に戻す）
                        p[k] = p0;
                    }
                }
            }
        }

        // 重力加速度による速度更新
        v[k].y += G / fr;
    }

    glutSwapBuffers(); // 描画実行
}

// リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ウィンドウ内の描画領域(ビューポート)の指定
    // 投影変換の設定
    glMatrixMode(GL_PROJECTION);                               // 変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();                                          // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); // 透視投影ビューボリューム設定

    winW = w;
    winH = h; // ウィンドウサイズ保持
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    GLdouble model[16], proj[16]; // 変換行列格納用
    GLint view[4];                // ビューポート設定格納用
    GLfloat winX, winY, winZ;     // ウィンドウ座標
    GLdouble objX, objY, objZ;    // ワールド座標

    // マウスボタンが押された
    if (state == GLUT_DOWN)
    {
        mX = x;
        mY = y;
        mState = state;
        mButton = button; // マウス情報保持
        // 左ボタンのとき
        if (button == GLUT_LEFT_BUTTON)
        {
            // マウス座標からウィンドウ座標の取得
            winX = mX;
            winY = winH - mY;                                                                    // x座標，y座標
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
        }
    }
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    GLdouble model[16], proj[16]; // 変換行列格納用
    GLint view[4];                // ビューポート設定格納用
    GLfloat winX, winY, winZ;     // ウィンドウ座標
    GLdouble objX, objY, objZ;    // ワールド座標

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
        winY = winH - y;                                                                     // x座標，y座標
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

// キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        exit(0);
    default:
        break;
    }
}

// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定
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
