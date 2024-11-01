#include <GLUT/glut.h>

// ウィンドウへの描画関数
void display(void)
{
}

int main(int argc, char *argv[])
{
  // OpenGL/GLUT 環境の初期化
  glutInit(&argc, argv);
  // ウィンドウを開く
  glutCreateWindow("Hello OpenGL/GLUT world.");
  // ウィンドウへの描画関数の登録
  glutDisplayFunc(display);
  // メインループ開始
  glutMainLoop();
  return 0;
}