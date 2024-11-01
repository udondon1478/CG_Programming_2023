/*g++ main02.cpp -framework OpenGL -framework GLUT -Wno-deprecated*/
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h> //OpenGL/GLUT の使用
//関数名の宣言
void display();
void initGL();
void reshape(int w, int h);
int main(int argc, char *argv[])
{
 //----------OpenGL/GLUT の初期化----------
 glutInit(&argc, argv);

 //----------初期設定----------
 glutInitDisplayMode(GLUT_RGBA); //ディスプレイモードの指定
 glutInitWindowSize(800, 600); //ウィンドウサイズの指定
 glutCreateWindow("CG02"); //ウィンドウの生成

 glutDisplayFunc(display); //ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape); //リシェイプコールバック関数の指定（"reshape()"）

 glClearColor(1.0, 1.0, 1.0, 1.0); //ウィンドウクリア色の指定（RGBA）
   // glMatrixMode(GL_PROJECTION); //カレント行列の指定（投影変換行列を指定）
    // glLoadIdentity(); //行列の初期化
     //gluOrtho2D(-2.0, 4.0, -2.0, 2.0); //二次元座標の設定（l, r, b, t）
 //----------イベント待ち無限ループ----------
 glutMainLoop();

 return 0;
}
//ディスプレイコールバック関数
void display()
{
 glClear(GL_COLOR_BUFFER_BIT); //ウィンドウクリア
 //----------図形----------
    //軸の描画
    //X 軸
    glColor3d(0.0, 0.0, 0.0); //色の指定
    glBegin(GL_LINES); //図形開始
    glVertex2d(-400.0, 0.0);
    glVertex2d( 400.0, 0.0);
    glEnd();
    //Y 軸
    glBegin(GL_LINES); //図形開始
    glVertex2d( 0.0, 300.0);
    glVertex2d( 0.0,-300.0);
    glEnd();
    
    
    
    glLineWidth(1);
    glPushMatrix(); //平行移動(200,150)
    glTranslated(200, 150, 0);
    for(int y=-150; y<=150; y+=30){ //横線の描画
        glPushMatrix(); //スケーリング，平行移動
        glTranslated(0, y, 0); //y だけ平行移動
        glScaled(3.0, 1.0, 1.0); //スケーリング
    //直線のプリミティブ
        glColor3d(1.0, 0.0, 0.0); //色の指定
        glBegin(GL_LINES); //図形開始
        glVertex2d(-50.0, 0.0);
        glVertex2d( 50.0, 0.0);
        glEnd();
        glPopMatrix(); //スケーリング，平行移動終わり
    }
    for(int x=-150; x<= 150; x+=30){ //縦線の描画
    glPushMatrix(); //スケーリング，回転，平行移動
    glTranslated(x, 0, 0); //x だけ平行移動
    glRotated(90.0, 0, 0, 1); //90 度回転
    glScaled(3.0, 1.0, 1.0); //スケーリング
    //直線のプリミティブ
    glColor3d(1.0, 0.0, 0.0); //色の指定
    glBegin(GL_LINES); //図形開始
    glVertex2d(-50.0, 0.0);
    glVertex2d( 50.0, 0.0);
    glEnd();
    glPopMatrix(); //スケーリング，回転，平行移動終わり
    }
    glPopMatrix();
    
    //---------------------------------------------------
    glPushMatrix(); //平行移動(-200,150)
    glTranslated(-200, 150, 0);
    
    for(double angle=0.0; angle<= 360; angle+=10){
    glPushMatrix(); //スケーリング，回転，平行移動
    glTranslated(0, 0, 0); //x だけ平行移動
    glRotated(angle, 0, 0, 1); //90 度回転
    glScaled(2.0, 2.0, 2.0); //スケーリング
    //直線のプリミティブ
    glColor3d(0.0, 1.0, 0.0); //色の指定
    glBegin(GL_LINE_LOOP); //図形開始
        glVertex2d(0.0, 50.0); //頂点 0
        glVertex2d(-43.0, -25.0); //頂点 1
        glVertex2d(43.0, -25.0);
    glEnd();
    glPopMatrix(); //スケーリング，回転，平行移動終わり
    }
    glPopMatrix();
    //-----------------------------------
    glPushMatrix(); //平行移動(-200,-150)
    glTranslated(-200, -150, 0);
    
    for(double angle=0.0; angle<= 360; angle+=5){
    glPushMatrix(); //スケーリング，回転，平行移動
    glTranslated(0, 0, 0); //x だけ平行移動
    glRotated(angle, 0, 0, 1); //90 度回転
    glScaled(2.0, 2.0, 2.0); //スケーリング
    //直線のプリミティブ
    glColor3d(0.0, 0.0, 1.0); //色の指定
    glBegin(GL_LINE_LOOP); //図形開始
        glVertex2d(50.0, 50.0); //頂点 0
        glVertex2d(50.0, -50.0); //頂点 1
        glVertex2d(-50.0, -50.0);
        glVertex2d(-50.0, 50.0);
    glEnd();
    glPopMatrix(); //スケーリング，回転，平行移動終わり
    }
    glPopMatrix();
    //-----------------------------------------
    
	glPushMatrix(); //平行移動(200,-150)
	glTranslated(200, -150, 0);

	//正方形の中に菱形を描く
	for (double angle = 0.0; angle <= 360; angle += 10)
	{
		/* ループで繰り返し菱形を描く */
		glPushMatrix(); //スケーリング，回転，平行移動
		glTranslated(0, 0, 0); //x だけ平行移動
		glRotated(angle, 0, 0, 1); //90 度回転
		glScaled(2.0, 2.0, 2.0); //スケーリング
		//直線のプリミティブ
		glColor3d(0.0, 1.0, 1.0); //色の指定
		glBegin(GL_LINE_LOOP); //図形開始
		glVertex2d(0.0, 50.0); //頂点 0
		glVertex2d(-43.0, -25.0); //頂点 1
		glVertex2d(43.0, -25.0);
		glEnd();
		glPopMatrix(); //スケーリング，回転，平行移動終わり
	}
	

	//正方形の中に正三角形を描画

	for (double angle = 0.0; angle <= 360; angle += 10)
	{
		/* ループで繰り返し正三角形を描く */
		glPushMatrix(); //スケーリング，回転，平行移動
		glTranslated(0, 0, 0); //x だけ平行移動
		glRotated(angle, 0, 0, 1); //90 度回転
		glScaled(2.0, 2.0, 2.0); //スケーリング
		//直線のプリミティブ
		glColor3d(0.0, 1.0, 1.0); //色の指定
		glBegin(GL_LINE_LOOP); //図形開始
		glVertex2d(50.0, 50.0); //頂点 0
		glVertex2d(50.0, -50.0); //頂点 1
		glVertex2d(-50.0, -50.0);
		glEnd();
		glPopMatrix(); //スケーリング，回転，平行移動終わり
	}

	//1.2倍で星型を描画
	for (double angle = 0.0; angle <= 360; angle += 10)
	{
		/* ループで繰り返し星型を描く */
		glPushMatrix(); //スケーリング，回転，平行移動
		glTranslated(0, 0, 0); //x だけ平行移動
		glRotated(angle, 0, 0, 1); //90 度回転
		glScaled(1.2, 1.2, 1.2); //スケーリング
		//直線のプリミティブ
		glColor3d(0.0, 1.0, 1.0); //色の指定
		glBegin(GL_LINE_LOOP); //図形開始
		glVertex2d(0.0, 50.0); //頂点 0
		glVertex2d(-43.0, -25.0); //頂点 1
		glVertex2d(43.0, -25.0);
		glEnd();
		glPopMatrix(); //スケーリング，回転，平行移動終わり
	}

	//Scaleのx軸を-1にして三角形を描画
	

	glPopMatrix(); //平行移動終わり

 glFlush(); //OpenGL 命令実行
}



//リシェイプコールバック関数
void reshape(int w, int h)
{
 glViewport(0, 0, w, h); //ビューポート設定
 glMatrixMode(GL_PROJECTION); //カレント行列の指定（投影変換行列を指定）
 glLoadIdentity(); //行列の初期化
 gluOrtho2D(-w/2.0, w/2.0, -h/2.0, h/2.0); //二次元座標の設定（投影変換行列に二次元座標変換のための行列乗算）
 glMatrixMode(GL_MODELVIEW); //カレント行列の指定（モデルビュー行列を指定）
}