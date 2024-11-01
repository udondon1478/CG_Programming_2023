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
void glMySolidCircle(double cx, double cy, double r);
void glMyWireCircle(double cx, double cy, double r, double w);
void glMySolidEllipse(double cx, double cy, double a, double b);
void glBezier(double sx, double sy, double ex, double ey, double tx, double ty);
void glMyWireCircle2(double cx, double cy, double r, double w, int p);
void glMySpiral(double cx, double cy, double r, double w, int p, int turns);



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

    glColor3d(0.941, 0.714, 0.761); // 色の指定
    glMySolidCircle(0.0, 0.0, 180.0);

    //縦長の縁を描画
    //頬
    glColor3d(0.929, 0.620, 0.678);
    glMySolidEllipse(-80.0, 0.0, 30.0, 15.0);

    glColor3d(0.914, 0.490, 0.549);
    glMySolidEllipse(-80.0, 0.0, 25.0, 10.0);

    glColor3d(0.929, 0.620, 0.678);
    glMySolidEllipse(80.0, 0.0, 30.0, 15.0);

    glColor3d(0.914, 0.490, 0.549);
    glMySolidEllipse(80.0, 0.0, 25.0, 10.0);

    glColor3d(0.463, 0.184, 0.247); // 色の指定
    glMySolidEllipse(-50.0, 50.0, 20.0, 50.0);

    glColor3d(0.463, 0.184, 0.247); // 色の指定
    glMySolidEllipse(50.0, 50.0, 20.0, 50.0);

    glColor3d(1.0, 1.0, 1.0);
    glMySolidEllipse(50.0, 70.0, 13.0, 20.0);

    glColor3d(1.0, 1.0, 1.0);
    glMySolidEllipse(-50.0, 70.0, 13.0, 20.0);

    //逆反円のベジェ曲線を描画
    glColor3d(0.463, 0.184, 0.247); // 色の指定
    glLineWidth(5.0);
    glBezier(30.0, -30.0, -30.0, -30.0, 0.0, -60.0);

    glColor3d(0.463, 0.184, 0.247); // 色の指定
    glMySolidCircle(30.0, -30.0, 5.0);

    glColor3d(0.463, 0.184, 0.247); // 色の指定
    glMySolidCircle(-30.0, -30.0, 5.0);

    glColor3d(0.463, 0.184, 0.247); // 色の指定
    glMyWireCircle(0.0, 0.0, 180.0, 15.0);

    








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

// 塗りつぶし円の描画（中心座標(cx, cy)，半径 r）
void glMySolidCircle(double cx, double cy, double r)
{
    int xMin, xMax, yMin, yMax; // 計算領域
    double f;                   // 陰関数の計算用
    // 円の半径に基づく計算領域の決定
    xMin = cx - r;
    xMax = cx + r;
    yMin = cy - r;
    yMax = cy + r;
    glPointSize(2.0); // 点の描画サイズ指定
    // 点の描画開始
    glBegin(GL_POINTS);
    // 計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
    for (int j = yMin; j <= yMax; j++)
    {
        for (int i = xMin; i <= xMax; i++)
        {
            // 円を表す陰関数 f(x, y)に点(i, j)を代入
            f = (i - cx) * (i - cx) + (j - cy) * (j - cy) - r * r;
            // 計算結果の判定：f<=0 → 点(i, j)は円の境界または内部 → 点を描画
            if (f <= 0)
            {
                glVertex2d(i, j);
            }
        }
    }
    glEnd(); // 描画終了
}

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

//中心(cx, cy)，半径 r，線幅 w，分割数 p の円(正多角形)の描画
void glMyWireCircle2(double cx, double cy, double r, double w, int p)
{
double x, y; //座標計算結果
double t, delta; //パラメータ t および変位量 delta
delta = 2.0*M_PI/p; //delta=2π/p
glLineWidth(w); //線幅の指定
//直線の描画開始
glBegin(GL_LINE_STRIP); //描画開始（閉じた折れ線）
//i を 0 から p まで変化させることで t を delta ずつ変化させて，頂点座標を計算
for(int i=0; i<=p; i++){
t = delta*i; //t：中心から見た頂点座標の角度
x = i*cos(t)+cx; //頂点の x 座標
y = i*sin(t)+cy; //頂点の y 座標
glVertex2d(x, y); //頂点描画
}
glEnd(); //描画終了
}

void glMySpiral(double cx, double cy, double r, double w, int p, int turns) {
    double x, y; //座標計算結果
    double t, delta; //パラメータ t および変位量 delta
    delta = 2.0 * M_PI / p; //delta=2π/p
    glLineWidth(w); //線幅の指定

    for (int j = 0; j < turns; j++) { // 指定した回数だけうずまきを描画
        glBegin(GL_LINE_STRIP); //描画開始（閉じた折れ線）

        // i を 0 から p まで変化させることで t を delta ずつ変化させて，頂点座標を計算
        for (int i = 0; i <= p; i++) {
            t = delta * i + j * M_PI; // t：中心から見た頂点座標の角度（j * M_PI を追加）
            x = r * cos(t) + cx; // 頂点の x 座標
            y = r * sin(t) + cy; // 頂点の y 座標
            glVertex2d(x, y); // 頂点描画
        }

        glEnd(); // 描画終了
    }
}
