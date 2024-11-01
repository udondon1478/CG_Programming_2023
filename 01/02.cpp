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
    glutInitWindowSize(800, 600);   // ウィンドウサイズの指定
    glutCreateWindow("CG02");       // ウィンドウの生成

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
 glLineWidth(3.0);

 //Yの描画
 glBegin(GL_LINE_STRIP); //図形開始
 glColor3d(1.0, 1.0, 0.0); //色の指定
 glVertex2d(-0.7, 0.3); //Yの頂点
 glColor3d(1.0, 0.0, 0.0); //色の指定
 glVertex2d(-0.5, 0); //Yの中心
 glColor3d(0.0, 1.0, 0.0); //色の指定
 glVertex2d(-0.25, 0.3); //Yの頂点
    glColor3d(0.0, 0.0, 1.0); //色の指定
    glVertex2d(-0.5, 0); //Yの中心
    glColor3d(1.0, 1.0, 0.0); //色の指定
    glVertex2d(-0.5, -0.5); //Yの頂点
    glEnd(); //図形終了

    glBegin(GL_LINE_STRIP); //図形開始

//カンマ
 glColor3d(0.0, 0.0, 1.0); //色の指定
 glVertex2d(0, -0.45); //頂点 3
 glColor3d(1.0, 0.0, 1.0); //色の指定
 glVertex2d(0, -0.5); //頂点 4
 glEnd(); //図形終了

//Aの描画
    glBegin(GL_LINE_STRIP); //図形開始
    glColor3d(1.0, 1.0, 0.0); //色の指定
    glVertex2d(0.7, -0.5); //Aの頂点
    glColor3d(1.0, 0.0, 0.0); //色の指定
    glVertex2d(0.5, 0.3); //Aの頂点
    glColor3d(0.0, 1.0, 0.0); //色の指定
    glVertex2d(0.3, -0.5); //Aの頂点
    glColor3d(1.0, 1.0, 0.0); //色の指定
    glVertex2d(0.4, -0.1); //Aの頂点
    glColor3d(1.0, 0.0, 0.0); //色の指定
    glVertex2d(0.6, -0.1); //Aの頂点
    glEnd(); //図形終了

 glFlush(); //OpenGL 命令実行
}