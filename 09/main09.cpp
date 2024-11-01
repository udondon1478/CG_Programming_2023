// g++ -O3 main09.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <GLUT/glut.h> //OpenGL/GLUTの使用

// 三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

// 定数宣言
#define FLOOR 10.0  // 床の幅の半分
#define TILE 100    // 床頂点格子分割数
#define BOXMAX 1000 // ボックスの最大数

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
void shift_keyboard(unsigned char key, int x, int y);
Vec_3D addVec(Vec_3D p1, Vec_3D p2);
Vec_3D diffVec(Vec_3D p1, Vec_3D p2);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
double vectorNormalize(Vec_3D *vec);

// グローバル変数
double eDist, eDegX, eDegY;     // 視点極座標
double mX, mY, mButton, mState; // マウスクリック情報
int winW, winH;                 // ウィンドウサイズ
Vec_3D floorPoint[TILE][TILE];  // 床頂点
double boxSize = 1.0;           // ボックスの大きさ
Vec_3D boxPos[100];             // ボックスの中心座標（ワールド座標系） 101個以上は置かない前提.置いたら落ちる.
float boxCol[100][4];           // ボックスの色
int boxNum = 0;                 // 最初は 0.クリックしてオブジェクトを配置する度に増やす. ボックスの現在の数
float color_pallette[10][4] = {
    // カラーパレット RGBA
    {1.0, 1.0, 1.0, 1.0}, // 0 番 white
    {1.0, 0.0, 0.0, 1.0}, // 1 番 red
    {0.0, 1.0, 0.0, 1.0}, // 2 番 grean
    {0.0, 0.0, 1.0, 1.0}, // 3 番 blue
    {1.0, 1.0, 0.0, 1.0}, // 4 番 yellow
    {0.0, 1.0, 1.0, 1.0}, // 5 番 cyan
    {1.0, 0.0, 1.0, 1.0}, // 6 番 magenta
    {0.0, 0.0, 0.0, 1.0}, // 7 番 black
    {0.6, 0.4, 0.2, 1.0}, // 8 番 orange
    {0.5, 0.5, 0.5, 1.0}  // 9 番 gray
};
int color_index = 0;      // 指定色の管理用変数.デフォルトは 0(白色) 現在の色番号
int mouse_event_mode = 0; // 操作モード用変数（０：オブジェクト配置　１：地面盛り上げ）
double heap_width = 1.0;  // 盛り上げ幅
double heap_height = 0.1; // 盛り上げ量
int heap_type = 0;        // 盛り上げの型　　0：平刀、　1：三角刀、2：丸刀
int mModifier = 0;        // キーボード修飾子
// 高くするか低くするかのフラグ
int heightFlag = 1;

double T = 0.0;        // 時刻
double fps = 30.0;     // フレームレート
double dT = 1.0 / fps; // 時間間隔
double theta = 0.0;    // 角度
double armAngle = -50.0;
int direction = -1;

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

// 色の配列
GLfloat colors[14][4] = {
    // 頂点の高さで色を変える
    {1.0, 0.0, 0.0, 1.0},          // 赤
    {1.0, 0.5, 0.0, 1.0},          // オレンジ
    {1.0, 1.0, 0.0, 1.0},          // 黄色
    {0.5, 1.0, 0.0, 1.0},          // 黄緑
    {0.0, 0.404, 0.533, 1.0},      // 緑
    {0.8627, 0.8314, 0.7098, 1.0}, // 水緑
    {0.322, 0.667, 0.263, 1.0},    // 水色
    {0.0, 0.5, 1.0, 1.0},          // 青緑
    {0.0, 0.0, 1.0, 1.0},          // 青
    {0.498, 0.0, 1.0, 1.0},        // 薄紫
    {1.0, 0.0, 1.0, 1.0},          // 紫
    {1.0, 0.0, 0.498, 1.0},        // ピンク
    {0.5, 0.5, 0.5, 1.0},          // 灰色
    {1.0, 1.0, 1.0, 1.0}           // 白
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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH); // ディスプレイモードの指定
    glutInitWindowSize(800, 600);                // ウィンドウサイズの指定
    glutCreateWindow("CG_09");                   // ウィンドウの生成

    // コールバック関数の指定
    glutDisplayFunc(display);    // ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape);    // リサイズコールバック関数の指定（"reshape()"）
    glutTimerFunc(33, timer, 0); // タイマーコールバック関数（"timer"）
    glutMouseFunc(mouse);        // マウスクリックコールバック関数の指定（"mouse()"）
    glutMotionFunc(motion);      // マウスドラッグコールバック関数の指定（"mouse()"）
    glutKeyboardFunc(keyboard);  // キーボードコールバック関数の指定（"keyboard()"）

    // 各種設定
    glClearColor(0.5, 0.5, 1.0, 1.0); // ウィンドウクリア色の指定（RGBA）
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
            // 最低点はfloatPoint[i][j].y=-3.0、最高点はfloorPoint[i][j].y = 3.0、中間色はヒートマップのようにグラデーションさせる
            // -3.0~3.0の範囲を13分割すると、-3.0~-2.54は赤、-2.54~-2.08はオレンジ、-2.08~-1.62は黄色、-1.62~-1.15は黄緑、-1.15~-0.69は緑、-0.69~-0.23は水緑、-0.23~0.23は水色、0.23~0.69は青緑、0.69~1.15は青、1.15~1.61は薄紫、1.61~2.08は紫、2.08~2.54はピンク、2.54~3.0は灰色
            // 0.46ごとに色を変える
            // -3.0~-2.54はcolors[0], -2.54~-2.08はcolors[1], -2.08~-1.62はcolors[2], -1.62~-1.15はcolors[3], -1.15~-0.69はcolors[4], -0.69~-0.23はcolors[5], -0.23~0.23はcolors[6], 0.23~0.69はcolors[7], 0.69~1.15はcolors[8], 1.15~1.61はcolors[9], 1.61~2.08はcolors[10], 2.08~2.54はcolors[11], 2.54~3.0はcolors[12]
            if (floorPoint[i][j].y >= -3.0 && floorPoint[i][j].y < -2.54)
            {
                currentColor = 0;
            }
            else if (floorPoint[i][j].y >= -2.54 && floorPoint[i][j].y < -2.08)
            {
                currentColor = 1;
            }
            else if (floorPoint[i][j].y >= -2.08 && floorPoint[i][j].y < -1.62)
            {
                currentColor = 2;
            }
            else if (floorPoint[i][j].y >= -1.62 && floorPoint[i][j].y < -1.15)
            {
                currentColor = 3;
            }
            else if (floorPoint[i][j].y >= -1.15 && floorPoint[i][j].y < -0.01)
            {
                currentColor = 4;
            }
            else if (floorPoint[i][j].y >= 0.0 && floorPoint[i][j].y < 0.01)
            {
                currentColor = 5;
            }
            else if (floorPoint[i][j].y >= 0.01 && floorPoint[i][j].y < 0.23)
            {
                currentColor = 6;
            }
            else if (floorPoint[i][j].y >= 0.23 && floorPoint[i][j].y < 0.69)
            {
                currentColor = 7;
            }
            else if (floorPoint[i][j].y >= 0.69 && floorPoint[i][j].y < 1.15)
            {
                currentColor = 8;
            }
            else if (floorPoint[i][j].y >= 1.15 && floorPoint[i][j].y < 1.61)
            {
                currentColor = 9;
            }
            else if (floorPoint[i][j].y >= 1.61 && floorPoint[i][j].y < 2.08)
            {
                currentColor = 10;
            }
            else if (floorPoint[i][j].y >= 2.08 && floorPoint[i][j].y < 2.54)
            {
                currentColor = 11;
            }
            else if (floorPoint[i][j].y >= 2.54 && floorPoint[i][j].y <= 3.0)
            {
                currentColor = 13;
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
            v1 = diffVec(floorPoint[i][j + 1], floorPoint[i][j]);
            v2 = diffVec(floorPoint[i + 1][j], floorPoint[i][j]);
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

    // ボックスの描画
    for (int i = 0; i < boxNum; i++)
    {
        // 色の設定
        //  col[0] = 1.0; col[1] = 1.0; col[2] = 0.0; col[3] = 1.0;
        spe[0] = 1.0;
        spe[1] = 1.0;
        spe[2] = 1.0;
        spe[3] = 1.0;                                                       // 鏡面反射係数
        shi[0] = 10.0;                                                      // ハイライト係数
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, boxCol[i]); // 拡散反射係数，環境光反射係数を設定
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);                  // 鏡面反射
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);                 // ハイライト
        glPushMatrix();                                                     // ボックスの設置
        glTranslated(boxPos[i].x, boxPos[i].y, boxPos[i].z);                // 指定した座標へ平行移動
        glScaled(boxSize, boxSize, boxSize);                                // 指定したサイズに拡大・縮小
        glutSolidCube(1.0);
        glPopMatrix(); // ボックスの設置　終わり
    }

    // 島に生える植物の描画
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
    glScaled(0.4, 0.5, 0.4);
    glTranslated(0.0, 0.0, 2.0);

    glPushMatrix();
    glTranslated(0.0, 1.5, 2.0);
    glScaled(0.8, 0.8, 0.8);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidIcosahedron();
    glPopMatrix();

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
    glTranslated(0, 0.5, 2.0);

    glScaled(0.3, 0.6, 0.3);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();

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
    glScaled(0.4, 0.5, 0.4);
    glTranslated(0.0, 0.0, 2.0);

    glPushMatrix();
    glTranslated(2.0, 1.5, -1.0);
    glScaled(0.8, 0.8, 0.8);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidIcosahedron();
    glPopMatrix();

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
    glTranslated(2.0, 0.5, -1.0);

    glScaled(0.3, 0.6, 0.3);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();

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
    glScaled(0.4, 0.5, 0.4);
    glTranslated(0.0, 0.2, 2.0);

    glPushMatrix();
    glTranslated(3, 1.5, 2.0);
    glScaled(0.8, 0.8, 0.8);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidIcosahedron();
    glPopMatrix();

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
    glTranslated(3, 0.5, 2.0);

    glScaled(0.3, 0.6, 0.3);
    glColor4d(0.0, 1.0, 0.0, 1.0);
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();
    glScaled(0.04, 0.04, 0.04);
    glTranslated(0.0, 0.2, 2.0);
    //90度みぎを向く
    glRotated(-135, 0.0, 1.0, 0.0);

    // オリジナルオブジェクトのモデリング
    // 怪盗グルーのミニオンを描画
    // 全体を上にずらす
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 10.0, 0.0);
    glPushMatrix(); // 図形３の変換
    glTranslated(0.0, 10.0, 0.0);
    glScaled(5.0, 5.0, 5.0);

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

    // 灰色のマテリアルを生成
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

    // 白のマテリアルを生成
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

    // マテリアルを生成
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
    // 指
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

    // 反対の腕
    //  黄色い円柱を描画
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
    // 指
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

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col3); // 拡散&環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe3);            // 鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi3);           // ハイライト係数を設定
    //ヨットの描画
    // 船体
    glPushMatrix();
    glTranslated(0.0, 0.0, 0.0);
    glScaled(0.1, 0.1, 0.1);
    glRotated(theta, 0.0, 1.0, 0.0);
    glPushMatrix();
    glTranslated(40.0, 0.0, 0.0);   
    //180度右に回転
    glRotated(180.0, 0.0, 1.0, 0.0);
    //立方体を描画
    glutSolidCube(10.0);
    glPopMatrix();
    glPushMatrix();
    glTranslated(40.0, 0.0, 0.0);

    //隣の面にも同様に描画
    glBegin(GL_TRIANGLES);
    glVertex3d(5.0, -5.0, -5.0);
    glVertex3d(5.0, 5.0, -15.0);
    glVertex3d(5.0, 5.0, -5.0);
    glVertex3d(-5.0, -5.0, -5.0);
    glVertex3d(-5.0, 5.0, -15.0);
    glVertex3d(-5.0, 5.0, -5.0);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3d(5.0, -5.0, -5.0);
    glVertex3d(5.0, 5.0, -15.0);
    glVertex3d(-5.0, 5.0, -15.0);
    glVertex3d(-5.0, -5.0, -5.0);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3d(5.0, 5.0, -5.0);
    glVertex3d(5.0, 5.0, -15.0);
    glVertex3d(-5.0, 5.0, -15.0);
    glVertex3d(-5.0, 5.0, -5.0);
    glEnd();





    glPopMatrix();
    glPopMatrix();


    glFlush(); // 描画実行
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
    winH = h;
}

// タイマーコールバック関数
void timer(int value)
{
    //	eDegY += 1.0;  //視点自動回転

    glutPostRedisplay();         // ディスプレイイベント強制発生
    glutTimerFunc(33, timer, 0); // タイマー再設定
    theta += 360.0 * dT * 0.1;

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
    mState = state;

    mModifier = glutGetModifiers(); // キーボード修飾子の取得

    // マウス座標からウィンドウ座標の取得
    winX = x;
    winY = winH - y;                                                     // x座標とy座標
    glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); // z座標はデプス値

    // モデルビュー変換行列・投影変換行列・ビューボート設定の取り出し
    glGetDoublev(GL_MODELVIEW_MATRIX, model); // モデルビュー変換行列
    glGetDoublev(GL_PROJECTION_MATRIX, proj); // 投影変換行列
    glGetIntegerv(GL_VIEWPORT, view);         // ビューポート設定

    // ウィンドウ座標をワールド座標に変換
    gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);

    // 左クリックされたら
    if (mButton == GLUT_LEFT_BUTTON && mState == GLUT_DOWN && mouse_event_mode == 0)
    {
        // クリック点をboxPosに設定
        boxPos[boxNum].x = objX;
        boxPos[boxNum].y = objY + boxSize / 2.0;
        boxPos[boxNum].z = objZ;
        for (int k = 0; k < 4; k++)
        { // ボックスの色を現在の色に設定
            boxCol[boxNum][k] = color_pallette[color_index][k];
        }
        boxNum++; // 新しくオブジェクトを置いたので，一つ増やす
    }
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    // 右ドラッグの時，カメラの位置を変更
    if (mButton == GLUT_RIGHT_BUTTON && mState == GLUT_DOWN && mouse_event_mode == 0)
    {
        eDegY += (mX - x) * 0.5;
        eDegX -= (mY - y) * 0.5;
        mX = x;
        mY = y;
    }
    else if (mButton == GLUT_RIGHT_BUTTON && mState == GLUT_DOWN && mouse_event_mode == 1)
    {
        for (int i = 0; i < TILE; i++)
        {
            for (int j = 0; j < TILE; j++)
            {
                floorPoint[i][j].y = 0.0; // 床を平坦に
            }
        }
    }
    if (mButton == GLUT_LEFT_BUTTON && mState == GLUT_DOWN && mouse_event_mode == 1)
    {
        printf("Left button is dragging on MODE 1\n");
        GLdouble model[16], proj[16]; // 変換行列格納用
        GLint view[4];                // ビューポート設定格納用
        GLfloat winX, winY, winZ;     // ウィンドウ座標
        GLdouble objX, objY, objZ;    // ワールド座標

        // マウス座標からウィンドウ座標の取得
        winX = x;
        winY = winH - y;                                                     // x座標とy座標
        glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); // z座標はデプス値

        // モデルビュー変換行列・投影変換行列・ビューボート設定の取り出し
        glGetDoublev(GL_MODELVIEW_MATRIX, model); // モデルビュー変換行列
        glGetDoublev(GL_PROJECTION_MATRIX, proj); // 投影変換行列
        glGetIntegerv(GL_VIEWPORT, view);         // ビューポート設定
        // ウィンドウ座標をワールド座標に変換
        gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);

        Vec_3D drag_point = {objX, objY, objZ}; // ドラッグ点
        for (int i = 0; i < TILE; i++)
        {
            for (int j = 0; j < TILE; j++)
            {
                // 床頂点p(i,j)とdrag_pointの距離計算
                Vec_3D diff = diffVec(floorPoint[i][j], drag_point);
                double distance = vectorNormalize(&diff);

                if (heightFlag == 1)
                {
                    // 盛り上げ
                    if (distance < heap_width / 2)
                    { // 距離が1.0未満だったら
                        // floorPoint[i][j].y=0.1; // 盛り上げ
                        // floorPoint[i][j].y += (-distance + 1.0) * 0.1;
                        if (heap_type == 0)
                        {
                            floorPoint[i][j].y = heap_height; // 盛り上げ y=0.1
                        }
                        else if (heap_type == 1)
                        {
                            floorPoint[i][j].y += (-distance + heap_width / 2) * heap_height; // y=-x+1
                        }
                        else if (heap_type == 2)
                        {
                            floorPoint[i][j].y += (-distance * distance + heap_width / 2) * heap_height; // y=x*x+1
                        }
                    }
                }
                else if (heightFlag == 0)
                {
                    // 掘り下げ
                    if (distance < heap_width / 2)
                    {
                        if (heap_type == 0)
                        {
                            floorPoint[i][j].y = -heap_height;
                        }
                        else if (heap_type == 1)
                        {
                            floorPoint[i][j].y -= (-distance + heap_width / 2) * heap_height;
                        }
                        else if (heap_type == 2)
                        {
                            floorPoint[i][j].y -= (-distance * distance + heap_width / 2) * heap_height;
                        }
                    }
                }
            }
        }
    }
}

// キーボードコールバック関数
void keyboard(unsigned char key, int x, int y)
{
    printf("key = %c (%d)\n", key, key);

    switch (key)
    {
    case 'q':
    case 'Q':
        exit(0);
    case 'n':
        eDist -= 1.0;
        break;
    case 'N':
        eDist -= 5.0;
        break;
    case 'f':
        eDist += 1.0;
        break;
    case 'F':
        eDist += 5.0;
        break;

    case 127: // delete
        if (boxNum == 0)
        {
            break;
        }
        else
        {
            boxNum--; // オブジェクトの数を一つ減らす
        }
        break;

    case '0':
        color_index = 0;
        // if(boxNum>0){
        //     for(int k=0; k<4; k++){
        //         boxCol[boxNum-1][k] = color_pallette[color_index][k];
        //     }
        // }
        break;
    case '1':
        color_index = 1;
        break;
    case '2':
        color_index = 2;
        break;
    case '3':
        color_index = 3;

        break;
    case '4':
        color_index = 4;
        break;
    case '5':
        color_index = 5;
        break;
    case '6':
        color_index = 6;
        break;
    case '7':
        color_index = 7;
        break;
    case '8':
        color_index = 8;
        break;
    case '9':
        color_index = 9;
        break;

    case ' ':
        mouse_event_mode = (mouse_event_mode + 1) % 2;
        if (mouse_event_mode)
            glClearColor(0.0, 0.0, 0.0, 1.0);
        else
            glClearColor(0.5, 0.5, 1.0, 1.0);
        break;

    case 'i': // 平刀
        heap_type = 0;
        break;

    case 'o': // 三角刀
        heap_type = 1;
        break;

    case 'p': // 丸刀
        heap_type = 2;
        break;

    case '-': // 彫刻刀のサイズ小
        if (heap_width > 0.2)
        {
            heap_width -= 0.1;
            heap_height = heap_width / 10.0;
        }
        break;

    case '+': // 彫刻刀のサイズ大
        if (heap_width < 1.0)
        {
            heap_width += 0.1;
            heap_height = heap_width / 10.0;
        }
        break;

    case '[': // 彫刻刀のサイズ小(小)
        if (heap_width > 0.2)
        {
            heap_width -= 0.01;
            heap_height = heap_width / 10.0;
        }
        break;

    case ']': // 彫刻刀のサイズ大(小)
        if (heap_width < 1.0)
        {
            heap_width += 0.01;
            heap_height = heap_width / 10.0;
        }
        break;

    // シフトキーが押されている時
    case 'y':
        heightFlag = 1;
        printf("heightFlag = %d\n", heightFlag);
        break;

    case 'u':
        heightFlag = 0;
        printf("heightFlag = %d\n", heightFlag);
        break;

    default:
        break;
    }
}

// ベクトルの和
Vec_3D addVec(Vec_3D p1, Vec_3D p2)
{
    Vec_3D out;

    out.x = p1.x + p2.x;
    out.y = p1.y + p2.y;
    out.z = p1.z + p2.z;

    return out;
}

// ベクトルの差
Vec_3D diffVec(Vec_3D p1, Vec_3D p2)
{
    Vec_3D out;

    out.x = p1.x - p2.x;
    out.y = p1.y - p2.y;
    out.z = p1.z - p2.z;

    return out;
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
