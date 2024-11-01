/*g++ main02.cpp -framework OpenGL -framework GLUT -Wno-deprecated*/
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h> //OpenGL/GLUT の使用
#include <math.h>

#define RADIUS (300)
#define SQUARE_SIZE (15)
#define ORIGIN (90)

double circle_x(int r, double theta)
{
    return r * cos(theta);
}

double circle_y(int r, double theta)
{
    return r * sin(theta);
}

// 関数名の宣言
void display();
void initGL();
void reshape(int w, int h);
int main(int argc, char *argv[])
{
    //----------OpenGL/GLUT の初期化----------
    glutInit(&argc, argv);

    //----------初期設定----------
    glutInitDisplayMode(GLUT_RGBA); // ディスプレイモードの指定
    glutInitWindowSize(1024, 1024); // ウィンドウサイズの指定
    glutCreateWindow("CG02");       // ウィンドウの生成

    glutDisplayFunc(display); // ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape); // リシェイプコールバック関数の指定（"reshape()"）

    glClearColor(1.0, 1.0, 1.0, 1.0); // ウィンドウクリア色の指定（RGBA）
                                      //  glMatrixMode(GL_PROJECTION); //カレント行列の指定（投影変換行列を指定）
                                      //  glLoadIdentity(); //行列の初期化
                                      // gluOrtho2D(-2.0, 4.0, -2.0, 2.0); //二次元座標の設定（l, r, b, t）
    //----------イベント待ち無限ループ----------
    glutMainLoop();

    return 0;
}
// ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // ウィンドウクリア
    //----------背景----------
    // 画面いっぱいに四角形を描画
    glBegin(GL_QUAD_STRIP); // 図形開始
    // 下辺
    glColor3d(0.85, 0.83, 0.77);
    glVertex2d(-512, -512);
    glColor3d(0.85, 0.83, 0.77);
    glVertex2d(512, -512);
    // 中点
    glColor3d(0.13, 0.31, 0.4);
    glVertex2d(-512, 0);
    glColor3d(0.13, 0.31, 0.4);
    glVertex2d(512, 0);
    // 上辺
    glColor3d(0.02, 0.01, 0.15);
    glVertex2d(-512, 512);
    glColor3d(0.02, 0.01, 0.15);
    glVertex2d(512, 512);
    glEnd(); // 図形終了
    //----------背景終了----------

    //----------図形----------
    double vertices[48][2];
    double theta = 0.0;
    for (int i = 0; i < 48; i++) {
        vertices[i][0] = circle_x(RADIUS, theta);
        vertices[i][1] = circle_y(RADIUS, theta);
        theta += M_PI / 24.0;
    }

    //第一層    glBegin(GL_TRIANGLES);
    glColor3d(0.82, 0.28, 0.40);
    for (int i = 11; i < 14; i++) {
        glVertex2d(vertices[i][0], vertices[i][1]);
    }
    glEnd();

    //第二層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.86, 0.33, 0.44);
    glVertex2d(vertices[13][0], vertices[13][1]);
    glVertex2d(vertices[14][0], vertices[14][1]);
    glVertex2d(vertices[11][0], vertices[11][1]);
    glVertex2d(vertices[10][0], vertices[10][1]);
    glEnd();

    //第三層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.90, 0.43, 0.47);
    glVertex2d(vertices[14][0], vertices[14][1]);
    glVertex2d(vertices[15][0], vertices[15][1]);
    glVertex2d(vertices[10][0], vertices[10][1]);
    glVertex2d(vertices[9][0], vertices[9][1]);
    glEnd();

    //第四層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.91, 0.53, 0.48);
    glVertex2d(vertices[15][0], vertices[15][1]);
    glVertex2d(vertices[16][0], vertices[16][1]);
    glVertex2d(vertices[9][0], vertices[9][1]);
    glVertex2d(vertices[8][0], vertices[8][1]);
    glEnd();

    //第五層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.92, 0.60, 0.50);
    glVertex2d(vertices[16][0], vertices[16][1]);
    glVertex2d(vertices[17][0], vertices[17][1]);
    glVertex2d(vertices[8][0], vertices[8][1]);
    glVertex2d(vertices[7][0], vertices[7][1]);
    glEnd();

    //第六層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.95, 0.68, 0.55);
    glVertex2d(vertices[17][0], vertices[17][1]);
    glVertex2d(vertices[18][0], vertices[18][1]);
    glVertex2d(vertices[7][0], vertices[7][1]);
    glVertex2d(vertices[6][0], vertices[6][1]);
    glEnd();

    //第七層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.96, 0.73, 0.60);
    glVertex2d(vertices[18][0], vertices[18][1]);
    glVertex2d(vertices[19][0], vertices[19][1]);
    glVertex2d(vertices[6][0], vertices[6][1]);
    glVertex2d(vertices[5][0], vertices[5][1]);
    glEnd();

    //第八層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.96, 0.78, 0.66);
    glVertex2d(vertices[19][0], vertices[19][1]);
    glVertex2d(vertices[20][0], vertices[20][1]);
    glVertex2d(vertices[5][0], vertices[5][1]);
    glVertex2d(vertices[4][0], vertices[4][1]);
    glEnd();

    //第九層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.97, 0.83, 0.72);
    glVertex2d(vertices[20][0], vertices[20][1]);
    glVertex2d(vertices[21][0], vertices[21][1]);
    glVertex2d(vertices[4][0], vertices[4][1]);
    glVertex2d(vertices[3][0], vertices[3][1]);
    glEnd();

    //第十層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.97, 0.87, 0.78);
    glVertex2d(vertices[21][0], vertices[21][1]);
    glVertex2d(vertices[22][0], vertices[22][1]);
    glVertex2d(vertices[3][0], vertices[3][1]);
    glVertex2d(vertices[2][0], vertices[2][1]);
    glEnd();

    //第十一層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.96, 0.88, 0.79);
    glVertex2d(vertices[22][0], vertices[22][1]);
    glVertex2d(vertices[23][0], vertices[23][1]);
    glVertex2d(vertices[2][0], vertices[2][1]);
    glVertex2d(vertices[1][0], vertices[1][1]);
    glEnd();

    //第十二層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.91, 0.88, 0.81);
    glVertex2d(vertices[23][0], vertices[23][1]);
    glVertex2d(vertices[24][0], vertices[24][1]);
    glVertex2d(vertices[1][0], vertices[1][1]);
    glVertex2d(vertices[0][0], vertices[0][1]);
    glEnd();

    //第十三層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.78, 0.83, 0.79);
    glVertex2d(vertices[24][0], vertices[24][1]);
    glVertex2d(vertices[25][0], vertices[25][1]);
    glVertex2d(vertices[0][0], vertices[0][1]);
    glVertex2d(vertices[47][0], vertices[47][1]);
    glEnd();

    //第十四層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.62, 0.79, 0.77);
    glVertex2d(vertices[25][0], vertices[25][1]);
    glVertex2d(vertices[26][0], vertices[26][1]);
    glVertex2d(vertices[47][0], vertices[47][1]);
    glVertex2d(vertices[46][0], vertices[46][1]);
    glEnd();

    //第十五層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.47, 0.75, 0.74);
    glVertex2d(vertices[26][0], vertices[26][1]);
    glVertex2d(vertices[27][0], vertices[27][1]);
    glVertex2d(vertices[46][0], vertices[46][1]);
    glVertex2d(vertices[45][0], vertices[45][1]);
    glEnd();

    //第十六層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.30, 0.64, 0.7);
    glVertex2d(vertices[27][0], vertices[27][1]);
    glVertex2d(vertices[28][0], vertices[28][1]);
    glVertex2d(vertices[45][0], vertices[45][1]);
    glVertex2d(vertices[44][0], vertices[44][1]);
    glEnd();

    //第十七層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.23, 0.52, 0.62);
    glVertex2d(vertices[28][0], vertices[28][1]);
    glVertex2d(vertices[29][0], vertices[29][1]);
    glVertex2d(vertices[44][0], vertices[44][1]);
    glVertex2d(vertices[43][0], vertices[43][1]);
    glEnd();

    //第十八層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.14, 0.33, 0.45);
    glVertex2d(vertices[29][0], vertices[29][1]);
    glVertex2d(vertices[30][0], vertices[30][1]);
    glVertex2d(vertices[43][0], vertices[43][1]);
    glVertex2d(vertices[42][0], vertices[42][1]);
    glEnd();

    //第十九層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.07, 0.17, 0.29);
    glVertex2d(vertices[30][0], vertices[30][1]);
    glVertex2d(vertices[31][0], vertices[31][1]);
    glVertex2d(vertices[42][0], vertices[42][1]);
    glVertex2d(vertices[41][0], vertices[41][1]);
    glEnd();

    //第二十層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.02, 0.03, 0.14);
    glVertex2d(vertices[31][0], vertices[31][1]);
    glVertex2d(vertices[32][0], vertices[32][1]);
    glVertex2d(vertices[41][0], vertices[41][1]);
    glVertex2d(vertices[40][0], vertices[40][1]);
    glEnd();

    //第二十一層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.02, 0.03, 0.14);
    glVertex2d(vertices[32][0], vertices[32][1]);
    glVertex2d(vertices[33][0], vertices[33][1]);
    glVertex2d(vertices[40][0], vertices[40][1]);
    glVertex2d(vertices[39][0], vertices[39][1]);
    glEnd();

    //第二十二層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.007, 0.003, 0.12);
    glVertex2d(vertices[33][0], vertices[33][1]);
    glVertex2d(vertices[34][0], vertices[34][1]);
    glVertex2d(vertices[39][0], vertices[39][1]);
    glVertex2d(vertices[38][0], vertices[38][1]);
    glEnd();

    //第二十三層
    glBegin(GL_QUAD_STRIP);
    glColor3d(0.003, 0.003, 0.11);
    glVertex2d(vertices[34][0], vertices[34][1]);
    glVertex2d(vertices[35][0], vertices[35][1]);
    glVertex2d(vertices[38][0], vertices[38][1]);
    glVertex2d(vertices[37][0], vertices[37][1]);
    glEnd();

    //第二十四層
    glBegin(GL_TRIANGLES);
    glColor3d(0.0, 0.0, 0.09);
    glVertex2d(vertices[35][0], vertices[35][1]);
    glVertex2d(vertices[36][0], vertices[36][1]);
    glVertex2d(vertices[37][0], vertices[37][1]);
    glEnd();


    //白い円を描画、半径はRADIUS/2
    glBegin(GL_POLYGON);
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 0; i < 48; i++)
    {
        glVertex2d(vertices[i][0] / 2, vertices[i][1] / 2);
    }
    glEnd();

    glFlush(); // OpenGL 命令実行
}

// リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);                           // ビューポート設定
    glMatrixMode(GL_PROJECTION);                      // カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();                                 // 行列の初期化
    gluOrtho2D(-w / 2.0, w / 2.0, -h / 2.0, h / 2.0); // 二次元座標の設定（投影変換行列に二次元座標変換のための行列乗算）
    glMatrixMode(GL_MODELVIEW);                       // カレント行列の指定（モデルビュー行列を指定）
}