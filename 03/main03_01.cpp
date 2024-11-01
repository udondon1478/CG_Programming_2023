/* コンパイルコマンド
g++ main03.cpp -framework OpenGL -framework GLUT -Wno-deprecated
*/
#include <stdio.h>
#include <GLUT/glut.h>  //OpenGL/GLUTの使用
#include <math.h>  //数学関数の使用

//関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void glMySolidCircle(double cx, double cy, double r);
void glMySolidEllipse(double cx, double cy, double a, double b);
void glMyWireEllipse(double cx, double cy, double a, double b, double w);
void glMyWireCircle(double cx, double cy, double r, double w);
void glMyWireCircle2(double cx, double cy, double r, double w, int p);
void glBezier(double sx, double sy, double ex, double ey, double tx, double ty);

int main(int argc, char *argv[])
{
    //----------OpenGL/GLUTの初期化----------
    glutInit(&argc, argv);
    
    //----------初期設定----------
    glutInitDisplayMode(GLUT_RGBA);  //ディスプレイモードの指定
    glutInitWindowSize(800, 600);  //ウィンドウサイズの指定
    glutCreateWindow("CG03");  //ウィンドウの生成
    
    glutDisplayFunc(display);  //ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape);  //リシェイプコールバック関数の指定（"reshape()"）
    
    glClearColor(0.0, 0.0, 1.0, 1.0);  //ウィンドウクリア色の指定（RGBA）
    
    //----------イベント待ち無限ループ----------
    glutMainLoop();
    
    return 0;
}

//ディスプレイコールバック関数
void display()
{
    glMatrixMode(GL_MODELVIEW);  //カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();  //行列の初期化

    glClear(GL_COLOR_BUFFER_BIT);  //ウィンドウクリア
    
    //----------円1----------
    glColor3d(0.0, 1.0, 0.0);  //色の指定
    glMySolidCircle(0.0, 0.0, 150.0);
    //----------円2----------
    glColor3d(1.0, 1.0, 0.0);  //色の指定
    glMySolidCircle(100.0, -150.0, 100.0);
    
    //----------円3----------
    glColor3d(1.0, 0.0, 0.0);  //色の指定
    glMyWireCircle(250.0, 100.0, 120.0, 10.0);
    
    glColor3d(0.0, 0.0, 0.0);  //色の指定
    glMyWireCircle(0.0, 100.0, 120.0, 10.0);
    
    glColor3d(0.0, 0.0, 1.0);  //色の指定
    glMyWireCircle(-250.0, 100.0, 120.0, 10.0);
    
    glColor3d(1.0, 1.0, 0.0);  //色の指定
    glMyWireCircle(-150.0, -50.0, 120.0, 10.0);
    
    glColor3d(0.0, 1.0, 0.0);  //色の指定
    glMyWireCircle(150.0, -50.0, 120.0, 10.0);
    
    glColor3d(1.0, 0.0, 0.0);  //色の指定
    glMySolidEllipse(0.0, 0.0, 120.0, 50.0);
    
    glColor3d(1.0, 1.0, 1.0);  //色の指定
    glMyWireEllipse(0.0, 0.0, 150.0, 100.0, 2);
    glMyWireEllipse(0.0, 59.0, 100.0, 40.0, 2);
    glMyWireEllipse(0.0, 0.0, 40.0, 100.0, 2);
    
    //----------円4----------
    glColor3d(1.0, 0.0, 1.0);  //色の指定
    glMyWireCircle2(150.0, 100.0, 120.0, 5.0, 12);
    
    //ベジエ
    glColor3d(1.0, 1.0, 1.0);  //色の指定
    glLineWidth(5.0);
    glBezier(-350.0, -250.0, 200.0, 200.0, 600.0, -400.0);
    
    glFlush();  //OpenGL命令実行
}

//リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);  //カレント行列の指定（投影変換行列を指定）
    glLoadIdentity();  //行列の初期化
    gluOrtho2D(-w/2.0, w/2.0, -h/2.0, h/2.0);  //二次元座標の設定（投影変換行列に二次元座標変換のための行列乗算）
}

//塗りつぶし円の描画（中心座標(cx, cy)，半径r）
void glMySolidCircle(double cx, double cy, double r)
{
    double xMin, xMax, yMin, yMax;  //計算領域
    double f;  //陰関数の計算用
    
    //計算領域の決定
    xMin = cx-r; xMax = cx+r;
    yMin = cy-r; yMax = cy+r;
    
    glPointSize(1.5);   //点の描画サイズ指定
    
    //点の描画開始
    glBegin(GL_POINTS);
    //計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
    for (int j=yMin; j<=yMax; j++){
        for (int i=xMin; i<=xMax; i++){
            //中心(cx, cy)，半径rの円を表す陰関数f(x, y)に点(i, j)を代入
            f = (i-cx)*(i-cx)+(j-cy)*(j-cy)-r*r;
            
            //計算結果の判定：f<=0 → 点(i, j)は円の境界または内部 → 点を描画
            if(f<=0){
                glVertex2d(i, j);
            }
        }
    }
    glEnd();   //描画終了
}
    
    //塗りつぶしのない円の描画（中心座標(cx, cy)，半径r 線幅w）
    void glMyWireCircle(double cx, double cy, double r, double w)
    {
        double xMin, xMax, yMin, yMax;  //計算領域
        double f,f2;  //陰関数の計算用
        
        //計算領域の決定
        xMin = cx-r; xMax = cx+r;
        yMin = cy-r; yMax = cy+r;
        
        glPointSize(1.5);   //点の描画サイズ指定
        
        //点の描画開始
        glBegin(GL_POINTS);
        //計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
        for (int j=yMin; j<=yMax; j++){
            for (int i=xMin; i<=xMax; i++){
                //中心(cx, cy)，半径rの円を表す陰関数f(x, y)に点(i, j)を代入
                f = (i-cx)*(i-cx)+(j-cy)*(j-cy)-r*r;
                
                f2 = (i-cx)*(i-cx)+(j-cy)*(j-cy)-(r-w)*(r-w);
                
                //計算結果の判定：f<=0 → 点(i, j)は円の境界または内部 → 点を描画
                if(f<=0||f2<=0){
                    glVertex2d(i, j);
                }
            }
        }
        glEnd();   //描画終了
}

//塗りつぶし円の描画（中心座標(cx, cy)，半径r）
void glMySolidEllipse(double cx, double cy, double a, double b)
{
    double xMin, xMax, yMin, yMax;  //計算領域
    double f,f2;  //陰関数の計算用
    
    //計算領域の決定
    xMin = cx-a; xMax = cx+a;
    yMin = cy-b; yMax = cy+b;
    
    glPointSize(1.5);   //点の描画サイズ指定
    
    //点の描画開始
    glBegin(GL_POINTS);
    //計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
    for (int j=yMin; j<=yMax; j++){
        for (int i=xMin; i<=xMax; i++){
            //中心(cx, cy)，半径rの円を表す陰関数f(x, y)に点(i, j)を代入
            f = (b*b)*(i-cx)*(i-cx)+(j-cy)*(j-cy)*(a*a)-a*a*b*b;
            
            //計算結果の判定：f<=0 → 点(i, j)は円の境界または内部 → 点を描画
            if(f<=0){
                glVertex2d(i, j);
            }
        }
    }
    glEnd();   //描画終了
}

void glMyWireEllipse(double cx, double cy, double a, double b, double w)
{
    double xMin, xMax, yMin, yMax;  //計算領域
    double f,f2;  //陰関数の計算用
    
    //計算領域の決定
    xMin = cx-a; xMax = cx+a;
    yMin = cy-b; yMax = cy+b;
    
    glPointSize(1.5);   //点の描画サイズ指定
    
    //点の描画開始
    glBegin(GL_POINTS);
    //計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
    for (int j=yMin; j<=yMax; j++){
        for (int i=xMin; i<=xMax; i++){
            //中心(cx, cy)，半径rの円を表す陰関数f(x, y)に点(i, j)を代入
            f = (b*b)*(i-cx)*(i-cx)+(j-cy)*(j-cy)*(a*a)-a*a*b*b;
            f2 = ((b-w)*(b-w))*(i-cx)*(i-cx)+(j-cy)*(j-cy)*((a-w)*(a-w))-(a-w)*(a-w)*(b-w)*(b-w);
            
            //計算結果の判定：f<=0 → 点(i, j)は円の境界または内部 → 点を描画
            if(f<=0&&f2>=0){
                glVertex2d(i, j);
            }
        }
    }
    glEnd();   //描画終了
}

//塗りつぶしのない円の描画（中心座標(cx, cy)，半径r 線幅w）
void glMyWireCircle2(double cx, double cy, double r, double w, int p)
{
    double x, y;  //計算領域
    double t, delta;  //陰関数の計算用
    
    delta=2.0*M_PI/p;
    glLineWidth(w);
    
    //点の描画開始
    glBegin(GL_LINE_STRIP);
    //計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
    for (int i=0; i<=p; i++){
        t=delta*i;
        x=r*cos(t)+cx;
        y=r*sin(t)+cy;
        glVertex2d(x,y);
        }
    glEnd();   //描画終了
}


//ベジェ曲線（始点(sx, sy)， 終点(ex, ey)， 制御点(tx, ty)）
void glBezier(double sx, double sy, double ex, double ey, double tx, double ty)
{
double t; //パラメータ（0〜1）
double step; //パラメータ増加量
double px, py; //描画点
t = 0;
step = 0.001;
glBegin(GL_LINE_STRIP); //描画開始（閉じた折れ線）
while (t<=1) {
//パラメータ t を 0〜1 に変化させながら，描画点(px, py)を逐次計算
px = (sx+ex-2.0*tx)*t*t+2.0*(tx-sx)*t+sx;
py = (sy+ey-2.0*ty)*t*t+2.0*(ty-sy)*t+sy;
glVertex2d(px, py); //描画点に点を描画
t += step;
}
glEnd();
}