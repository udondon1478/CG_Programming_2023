void makebox( float width, float depth, float height )
{	
	glEnable( GL_NORMALIZE );

	glBegin( GL_QUADS ); //四角形を作成
	glNormal3f( 0.0, 1.0, 0.0 ); //法線ベクトルを設定
		glVertex3f( -width/2, height, -depth/2 );//外側の面が表になるように反時計回りに４点を指定
		glVertex3f( width/2, height, -depth/2 );	
		glVertex3f( width/2, height, depth/2 );
		glVertex3f( -width/2, height, depth/2 );
	glEnd();
	glBegin( GL_QUADS );
	glNormal3f( 0.0, -1.0, 0.0 );
		glVertex3f( -width/2, 0, -depth/2 );
		glVertex3f( -width/2, 0, depth/2 );
		glVertex3f( width/2, 0, depth/2 );
		glVertex3f( width/2, 0, -depth/2 );
	glEnd();
	glBegin( GL_QUADS );
	glNormal3f( -1.0, 0.0, 0.0 );
		glVertex3f( -width/2, height, -depth/2 );
		glVertex3f( -width/2, height, depth/2 );
		glVertex3f( -width/2, 0, depth/2 );
		glVertex3f( -width/2, 0, -depth/2 );
	glEnd();
	glBegin( GL_QUADS );
	glNormal3f( 0.0, 0.0, 1.0 );
		glVertex3f( -width/2, height, depth/2 );
		glVertex3f( width/2, height, depth/2 );
		glVertex3f( width/2, 0, depth/2 );
		glVertex3f( -width/2, 0, depth/2 );
	glEnd();
	glBegin( GL_QUADS );
	glNormal3f( 1.0, 0.0, 0.0 );
		glVertex3f( width/2, height, depth/2 );
		glVertex3f( width/2, height, -depth/2 );
		glVertex3f( width/2, 0, -depth/2 );
		glVertex3f( width/2, 0, depth/2 );
	glEnd();
	glBegin( GL_QUADS );
	glNormal3f( 0.0, 0.0, -1.0 );
		glVertex3f( width/2, height, -depth/2 );
		glVertex3f( width/2, 0, -depth/2 );
		glVertex3f( -width/2, 0, -depth/2 );
		glVertex3f( -width/2, height, -depth/2 );
	glEnd();
	
	glDisable( GL_NORMALIZE );
}