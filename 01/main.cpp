/* コンパイルコマンド
g++ main.cpp -framework OpenGL -framework GLUT -Wno-deprecated
*/
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h> //OpenGL/GLUT の使用
// 関数名の宣言
void initGL();
void display();
int main(int argc, char *argv[])
{
    //----------OpenGL/GLUT の初期化----------
    glutInit(&argc, argv);

    //----------初期設定----------
    glutInitDisplayMode(GLUT_RGBA); // ディスプレイモードの指定
    glutInitWindowSize(600, 600);   // ウィンドウサイズの指定
    glutCreateWindow("CG01");       // ウィンドウの生成

    glutDisplayFunc(display); // ディスプレイコールバック関数の指定（"display()"）

    glClearColor(0.0, 0.0, 0.0, 1.0); // ウィンドウクリア色の指定（RGBA）

    //----------イベント待ち無限ループ----------
    glutMainLoop();

    return 0;
}
//ディスプレイコールバック関数
void display()
{
 glClear(GL_COLOR_BUFFER_BIT); //ウィンドウクリア
 //----------図形----------
 glBegin(GL_TRIANGLES); //図形開始
 //幅1高さ0.866の三角形
 glColor3d(1.0, 1.0, 0.0); //色の指定
 glVertex2d(0, 0.6); //頂点 0
 glColor3d(1.0, 0.0, 0.0); //色の指定
 glVertex2d(-0.5, -0.25); //頂点 1
 glColor3d(0.0, 1.0, 0.0); //色の指定
 glVertex2d(0.5, -0.25); //頂点 2

//幅1高さ0.866の三角形
 glColor3d(0.0, 0.0, 1.0); //色の指定
 glVertex2d(-0.5, 0.3); //頂点 3
 glColor3d(1.0, 0.0, 1.0); //色の指定
 glVertex2d(0, -0.5666); //頂点 4
 glColor3d(0.0, 1.0, 1.0); //色の指定
 glVertex2d(0.5, 0.3); //頂点 5
 glEnd(); //図形終了
     glFlush(); //OpenGL 命令実行

}