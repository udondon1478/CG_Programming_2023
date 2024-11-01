// g++ -O3 main08.cpp -framework OpenGL -framework GLUT -Wno-deprecated
// 床頂点の高さに応じて色を変える
// 高さ0は青、高さ2は赤、中間色はヒートマップのようにグラデーションさせる

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUTの使用

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

// 定数宣言
#define FLOOR 10.0 // 床の幅の半分
#define TILE 100   // 床頂点格子分割数

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

// グローバル変数
double eDist, eDegX, eDegY;    // 視点極座標
Vec_3D floorPoint[TILE][TILE]; // 床頂点
int mX, mY, mButton, mState;
int winW, winH; // ウィンドウサイズ
Vec_3D boxPos;  // 視点座標

// Retina
double rDisp = 1.0;

// 高くするか低くするかのフラグ
int heightFlag = 1;

// 色の配列
GLfloat colors[13][4] = {
    // 頂点の高さで色を変える
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
};
int currentColor = 0; // 現在の色のインデックス

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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // ディスプレイモードの指定
    glutInitWindowSize(800, 600);                              // ウィンドウサイズの指定
    glutCreateWindow("CG_08");                                 // ウィンドウの生成

    // コールバック関数の指定
    glutDisplayFunc(display);    // ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape);    // リサイズコールバック関数の指定（"reshape()"）
    glutTimerFunc(33, timer, 0); // タイマーコールバック関数（"timer"）
    glutMouseFunc(mouse);        // マウスクリックコールバック関数（"mouse"）
    glutMotionFunc(motion);      // マウスドラッグコールバック関数（"motion"）
    glutKeyboardFunc(keyboard);  // キーボードコールバック関数（"keyboard"）

    // 各種設定
    glClearColor(0.0, 0.0, 0.0, 1.0); // ウィンドウクリア色の指定（RGBA）
    glEnable(GL_DEPTH_TEST);          // デプスバッファの有効化
    glEnable(GL_NORMALIZE);           // 法線ベクトル正規化の有効化

    // 光源設定
    GLfloat colGA[] = {0.2, 0.2, 0.2, 1.0};                // パラメータ設定
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, colGA);         // グローバル環境光の設定
    glEnable(GL_LIGHTING);                                 // 陰影付けの有効化
    glEnable(GL_LIGHT0);                                   // 光源0の有効化
    GLfloat col[] = {1.0, 1.0, 1.0, 1.0};                  // パラメータ設定
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);                 // 光源0の拡散反射に関する強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);                // 光源0の鏡面反射に関する強度
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0002); // 光源0の二次減衰率の設定

    // 視点関係
    eDist = 20.0;
    eDegX = 10.0;
    eDegY = 0.0; // 視点極座標

    // 床頂点座標
    for (int i = 0; i < TILE; i++)
    {
        for (int j = 0; j < TILE; j++)
        {
            floorPoint[i][j].x = -FLOOR * 0.5 + FLOOR / (TILE - 1) * i; // x座標
            floorPoint[i][j].y = 0.0;                                   // y座標(高さ)
            floorPoint[i][j].z = -FLOOR * 0.5 + FLOOR / (TILE - 1) * j; // z座標
        }
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
    GLfloat lightPos0[] = {1.0, 3.0, 1.5, 1.0};   // 点光源
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0); // 光源座標

    // 描画準備
    GLfloat col[4], spe[4], shi[1]; // 材質設定用配列
    Vec_3D v1, v2, nv;              // 辺ベクトル，法線ベクトル

    // 床の表示
    // 描画
    glBegin(GL_QUADS);
    for (int j = 0; j < TILE - 1; j++)
    {
        for (int i = 0; i < TILE - 1; i++)
        {
            // 床頂点の高さに応じて色を変える
            // 最低点はfloatPoint[i][j].y=0.0、最高点はfloorPoint[i][j].y = 3.0、中間色はヒートマップのようにグラデーションさせる
            // 0.0~3.0の範囲を13分割すると、0.0~0.23は赤、0.23~0.46はオレンジ、0.46~0.69は黄色、0.69~0.92は黄緑、0.92~1.15は緑、1.15~1.38は水緑、1.38~1.61は水色、1.61~1.84は青緑、1.84~2.07は青、2.07~2.30は薄紫、2.30~2.53は紫、2.53~2.76はピンク、2.76~3.0は灰色
            // 0.23ごとに色を変える
            // 0.0~0.23はcolors[12], 0.23~0.46はcolors[11], 0.46~0.69はcolors[10], 0.69~0.92はcolors[9], 0.92~1.15はcolors[8], 1.15~1.38はcolors[7], 1.38~1.61はcolors[6], 1.61~1.84はcolors[5], 1.84~2.07はcolors[4], 2.07~2.30はcolors[3], 2.30~2.53はcolors[2], 2.53~2.76はcolors[1], 2.76~3.0はcolors[0]
            if (floorPoint[i][j].y >= 0.0 && floorPoint[i][j].y < 0.23)
            {
                currentColor = 12;
            }
            else if (floorPoint[i][j].y >= 0.23 && floorPoint[i][j].y < 0.46)
            {
                currentColor = 11;
            }
            else if (floorPoint[i][j].y >= 0.46 && floorPoint[i][j].y < 0.69)
            {
                currentColor = 10;
            }
            else if (floorPoint[i][j].y >= 0.69 && floorPoint[i][j].y < 0.92)
            {
                currentColor = 9;
            }
            else if (floorPoint[i][j].y >= 0.92 && floorPoint[i][j].y < 1.15)
            {
                currentColor = 8;
            }
            else if (floorPoint[i][j].y >= 1.15 && floorPoint[i][j].y < 1.38)
            {
                currentColor = 7;
            }
            else if (floorPoint[i][j].y >= 1.38 && floorPoint[i][j].y < 1.61)
            {
                currentColor = 6;
            }
            else if (floorPoint[i][j].y >= 1.61 && floorPoint[i][j].y < 1.84)
            {
                currentColor = 5;
            }
            else if (floorPoint[i][j].y >= 1.84 && floorPoint[i][j].y < 2.07)
            {
                currentColor = 4;
            }
            else if (floorPoint[i][j].y >= 2.07 && floorPoint[i][j].y < 2.30)
            {
                currentColor = 3;
            }
            else if (floorPoint[i][j].y >= 2.30 && floorPoint[i][j].y < 2.53)
            {
                currentColor = 2;
            }
            else if (floorPoint[i][j].y >= 2.53 && floorPoint[i][j].y < 2.76)
            {
                currentColor = 1;
            }
            else if (floorPoint[i][j].y >= 2.76 && floorPoint[i][j].y <= 3.0)
            {
                currentColor = 0;
            }

            // 色の設定
            col[0] = colors[currentColor][0];
            col[1] = colors[currentColor][1];
            col[2] = colors[currentColor][2];
            col[3] = colors[currentColor][3]; // 拡散反射係数
            spe[0] = 1.0;
            spe[1] = 1.0;
            spe[2] = 1.0;
            spe[3] = 1.0;                                                 // 鏡面反射係数
            shi[0] = 100.0;                                               // ハイライト係数
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); // 拡散反射係数，環境光反射係数を設定
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);            // 鏡面反射
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);           // ハイライト

            // 法線ベクトル計算
            v1.x = floorPoint[i][j + 1].x - floorPoint[i][j].x;
            v1.y = floorPoint[i][j + 1].y - floorPoint[i][j].y;
            v1.z = floorPoint[i][j + 1].z - floorPoint[i][j].z;
            v2.x = floorPoint[i + 1][j].x - floorPoint[i][j].x;
            v2.y = floorPoint[i + 1][j].y - floorPoint[i][j].y;
            v2.z = floorPoint[i + 1][j].z - floorPoint[i][j].z;
            nv = normcrossprod(v1, v2);   // 辺ベクトル外積計算→法線ベクトル
            glNormal3d(nv.x, nv.y, nv.z); // 法線ベクトル設定
            // 頂点座標

            glVertex3d(floorPoint[i][j].x, floorPoint[i][j].y, floorPoint[i][j].z);
            glVertex3d(floorPoint[i][j + 1].x, floorPoint[i][j + 1].y, floorPoint[i][j + 1].z);
            glVertex3d(floorPoint[i + 1][j + 1].x, floorPoint[i + 1][j + 1].y, floorPoint[i + 1][j + 1].z);
            glVertex3d(floorPoint[i + 1][j].x, floorPoint[i + 1][j].y, floorPoint[i + 1][j].z);
        }
    }
    glEnd(); // 配置終了

    //
    glutSwapBuffers(); // 描画実行
}

// リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w * rDisp, h * rDisp); // ビューポート設定
    // 投影変換の設定
    glMatrixMode(GL_PROJECTION);                              // 変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();                                         // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 1000.0); // 透視投影ビューボリューム設定

    winW = w;
    winH = h; // 現在のウィンドウサイズをグローバル変数に格納
}

// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();         // ディスプレイイベント強制発生
    glutTimerFunc(33, timer, 0); // タイマー再設定
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

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    GLdouble model[16], proj[16]; // 変換行列格納用
    GLint view[4];                // ビューポート設定格納用
    GLfloat winX, winY, winZ;     // ウィンドウ座標
    GLdouble objX, objY, objZ;    // ワールド座標
    mX = x;
    mY = y;
    mButton = button;
    mState = state; // 現在のマウスのクリック情報をグローバル変数に格納
    // マウス座標からウィンドウ座標の取得
    winX = mX;
    winY = winH - mY;                                                                    // x 座標，y 座標
    glReadPixels(winX * rDisp, winY * rDisp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); // z 座標はデプス値
    // モデルビュー変換行列・投影変換行列・ビューポート設定取り出し
    glGetDoublev(GL_MODELVIEW_MATRIX, model); // モデルビュー変換行列
    glGetDoublev(GL_PROJECTION_MATRIX, proj); // 投影変換行列
    glGetIntegerv(GL_VIEWPORT, view);         // ビューポート設定
    for (int i = 0; i < 4; i++)
    {
        view[i] /= rDisp;
    }
    // ウィンドウ座標(winX, winY, winZ)をワールド座標(objX, objY, objZ)に変換
    gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);

    /*
        // クリックした座標にある頂点の高さを変更
        // クリックした座標の床頂点のインデックスを計算
        int i = (int)((objX + FLOOR * 0.5) / (FLOOR / (TILE - 1)));
        int j = (int)((objZ + FLOOR * 0.5) / (FLOOR / (TILE - 1)));
    */

    /*
    for (int k = -1; k <= 1; k++)
    {
        for (int l = -1; l <= 1; l++)
        {
            floorPoint[i + k][j + l].y += 0.1;
        }
    }
*/

    printf("winX = %f, winY = %f, winZ = %f\n", winX, winY, winZ);   // ウィンドウ座標表示
    printf("objX = %f ,objY = %f, objZ = %f\n\n", objX, objY, objZ); // ワールド座標表示
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    // スペースキーを押していてかつ左クリックをしているときだけカメラを動かす
    if (mButton == GLUT_LEFT_BUTTON && mState == GLUT_DOWN)
    {
        int dX = x - mX; // x座標の移動量
        int dY = y - mY; // y座標の移動量

        printf("dX = %d, dY = %d\n", dX, dY);

        eDegY -= dX * 0.5; // マウスのx方向の移動量（dX）をy軸周りの角度（eDegY）の変化量に適用
        eDegX += dY * 0.5; // マウスのy方向の移動量（dY）をx軸周りの角度（eDegX）の変化量に適用

        mX = x;
        mY = y; // 現在のマウス座標でmXとmYを更新
    }
    /*
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        int dX = x - mX; // x座標の移動量
        int dY = y - mY; // y座標の移動量

        eDegY -= dX * 0.5; // マウスのx方向の移動量（dX）をy軸周りの角度（eDegY）の変化量に適用
        eDegX += dY * 0.5; // マウスのy方向の移動量（dY）をx軸周りの角度（eDegX）の変化量に適用

        mX = x;
        mY = y; // 現在のマウス座標でmXとmYを更新
    }
    */

    //左ボタンマウスドラッグのときにもマウスカーソルのワールド座標を求めて，床面の各頂点との距離を計測する．
    //そして，マウスカーソルワールド座標からの変形領域内に頂点があった場合には，その頂点の y 座標を増加させる．
    //このとき，マウスカーソルワールド座標からの距離が小さいほど頂点の移動量を大きくすることで，滑らかな変形形状が実現できる．
    
}

// キーボードコールバック関数
void keyboard(unsigned char key, int x, int y)
{
    printf("Key = %d, X = %d, Y = %d\n", key, x, y);

    switch (key)
    {
    case 'q':
    case 'Q':
    case 27: //[ESC]キー
        exit(0);

    case 'n':
        eDist -= 0.5;
        break;
    case 'f':
        eDist += 0.5;
        break;
    case 'u':
        heightFlag = 1;
        break;
    case 'd':
        heightFlag = 0;
        break;
    // Y,HでeDegXを増減
    case 'y':
        eDegX += 10.0;
        break;
    case 'h':
        eDegX -= 10.0;
        break;
    // I,KでeDegYを増減
    case 'i':
        eDegY += 10.0;
        break;
    case 'k':
        eDegY -= 10.0;
        break;
    default:
        break;
    }
}