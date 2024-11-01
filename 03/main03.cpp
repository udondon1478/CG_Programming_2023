/* コンパイルコマンド
g++ main03.cpp -framework OpenGL -framework GLUT -Wno-deprecated
*/
#include <stdio.h>
#include <GLUT/glut.h> //OpenGL/GLUTの使用
#include <math.h>      //数学関数の使用

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void glMyWireCircle(double cx, double cy, double r, double w);

int main(int argc, char *argv[])
{
    //----------OpenGL/GLUTの初期化----------
    glutInit(&argc, argv);

    //----------初期設定----------
    glutInitDisplayMode(GLUT_RGBA); // ディスプレイモードの指定
    glutInitWindowSize(800, 600);   // ウィンドウサイズの指定
    glutCreateWindow("CG03");       // ウィンドウの生成

    glutDisplayFunc(display); // ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape); // リシェイプコールバック関数の指定（"reshape()"）

    glClearColor(0.0, 0.0, 1.0, 1.0); // ウィンドウクリア色の指定（RGBA）

    //----------イベント待ち無限ループ----------
    glutMainLoop();

    return 0;
}

// ディスプレイコールバック関数
void display()
{
    glMatrixMode(GL_MODELVIEW); // カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();           // 行列の初期化

    glClear(GL_COLOR_BUFFER_BIT); // ウィンドウクリア


    glColor3d(1.0, 0.0, 0.0);  //色の指定
    glMyWireCircle(250.0, 100.0, 120.0, 10.0);

    glColor3d(0.0, 0.0, 0.0); // 色の指定
    glMyWireCircle(0.0, 100.0, 120.0, 10.0);

    glColor3d(0.0, 0.5, 1.0); // 色の指定
    glMyWireCircle(-250.0, 100.0, 120.0, 10.0);

    glColor3d(1.0, 1.0, 0.0); // 色の指定
    glMyWireCircle(-150.0, -50.0, 120.0, 10.0);

    glColor3d(0.0, 1.0, 0.0); // 色の指定
    glMyWireCircle(150.0, -50.0, 120.0, 10.0);

        glFlush(); // OpenGL命令実行
}

// リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);                      // カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();                                 // 行列の初期化
    gluOrtho2D(-w / 2.0, w / 2.0, -h / 2.0, h / 2.0); // 二次元座標の設定（投影変換行列に二次元座標変換のための行列乗算）
}

// 塗りつぶしのない円の描画（中心座標(cx, cy)，半径r 線幅w）
void glMyWireCircle(double cx, double cy, double r, double w)
{
    int i;
    double x, y;
    double pi = 3.14159265358979323846;

    glLineWidth(w); // 線幅の指定

    glBegin(GL_LINE_LOOP); // 描画の開始（線分の連結）
    for (i = 0; i < 360; i++)
    {
        x = cx + r * cos(i * pi / 180.0);
        y = cy + r * sin(i * pi / 180.0);
        glVertex2d(x, y); // 頂点の指定
    }
    glEnd(); // 描画の終了
}