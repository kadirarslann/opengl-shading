#include <stdlib.h>			
#include <stdio.h>			
#include <cmath>
#include <cstring>  
#include <math.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>

#include <sys/resource.h>

#include "Angel.h"

#define PI 3.14159265

using namespace std;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


int numVertices=0; ///
int numPoints=0; /// ucagı initı calistirmadan yürütcegimiz icin dısarda bu veriyi tutuyorum

GLfloat radius = 30.0; /// default value will be alter after reading data for given shape
GLfloat eyeX=0;  //// pilotun konumu gibi
GLfloat eyeY=0;

GLfloat velocitydegree=90; ///// plane hangi yone gidiyor
GLfloat velocityofplane=0.1; /// plane speed


bool godmode=false;   //// yukardan bakis
bool pilotmode=true;
bool pilotbehindmode=false;
bool shadingtype=true; /// true ise flat shadin değil ise Gouraud

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location

GLfloat  fovy = 120.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 100.0;

point4* Points;


GLuint vao; /// bunları disarda tuttum cunnku modlar arasına geciste referans vermem geekiyor
GLuint buffer;
GLuint program;

void increaseStackSize(){ 
    const rlim_t kStackSize = 128 * 1024 * 1024;   // min stack size = 16 MB
    struct rlimit rl;
    int result;
    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }
}

int getRandomInt(int limit){
	return rand()%limit;
}
float getRandomFloat(int max){ /// random generates for max 100 return 0-1.00
	return ( (float)( getRandomInt(max)))/100;
}



void init()
{
    srand(time(NULL));
    int grid=100;
    
        GLfloat width=100.0;
        GLfloat height=150.0;
        GLfloat negativehalfwidth=-1*(width/2);
        GLfloat negativehalfheight=-1*(height/2);
        GLfloat valueofinternalrow=width/(grid-1);
        GLfloat valueofinternalcolumn=height/(grid-1);
        GLfloat scalingfactor=10.0/grid; //// perturbated accordfingto vertexx num
        GLfloat maxheight=1.0;

        int pointindex=0; /// point yerlestirirken index
        int verticeindex=0;
        int normalindex=0;  /// normaller, aktarirken 
        int normalgouraudindex=0;
        int numpoints=((grid-1)*(grid-1)*6)+3;/// +3 plane yerlestirdigim icin
        int numvertices=(grid)*(grid);

        numPoints=numpoints;
        vec3   normals[numpoints];
        vec3   normalsGauraud[numpoints];
        vec4   materialColors[numPoints];
        point4 vertices[numvertices];
        color4 verticesColors[numvertices];
        color4 colors[numpoints];
        point4 points[numpoints];
        Points=points; /// plane yerlestirme icin
        
        for(int row=0;row<grid;row++){ //// gridkeri x-y üzerinde konumlandırır
            for(int column=0;column<grid;column++){
                vertices[row*grid+column]=point4(negativehalfwidth+column*valueofinternalrow,negativehalfheight+row*valueofinternalcolumn,
                1.01,1); 
            }
        }

        
        for(int index=0;index<numvertices;index++){ /// use little randomnoss to smoot area
            vertices[index].z+=getRandomFloat(200);
        }
        int walk=1;
        while(walk<(grid/2)){ /// burasi vertice yükseliği ayarlar
            int walkx=walk;
            int walky=walk;
            while(walky<grid-walk-1){
                GLfloat val1=vertices[(walky)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx)].z+vertices[(walky-1)*grid+(walkx+1)].z;
                GLfloat val2=vertices[(walky+1)*grid+(walkx-1)].z+vertices[(walky+1)*grid+(walkx)].z+vertices[(walky+1)*grid+(walkx+1)].z+vertices[(walky)*grid+(walkx+1)].z;
                vertices[walky*grid+walkx].z=((val1+val2)/8)*(1+scalingfactor)*(1.40-getRandomFloat(40));
                maxheight=vertices[walky*grid+walkx].z>maxheight?vertices[walky*grid+walkx].z:maxheight;
                walky++;
            
            }
            while(walkx<grid-walk-1){
                GLfloat val1=vertices[(walky)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx)].z+vertices[(walky-1)*grid+(walkx+1)].z;
                GLfloat val2=vertices[(walky+1)*grid+(walkx-1)].z+vertices[(walky+1)*grid+(walkx)].z+vertices[(walky+1)*grid+(walkx+1)].z+vertices[(walky)*grid+(walkx+1)].z;
                vertices[walky*grid+walkx].z=((val1+val2)/8)*(1+scalingfactor)*(1.40-getRandomFloat(40));
                maxheight=vertices[walky*grid+walkx].z>maxheight?vertices[walky*grid+walkx].z:maxheight;
                walkx++;
            }
            while(walky>=walk){
                GLfloat val1=vertices[(walky)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx)].z+vertices[(walky-1)*grid+(walkx+1)].z;
                GLfloat val2=vertices[(walky+1)*grid+(walkx-1)].z+vertices[(walky+1)*grid+(walkx)].z+vertices[(walky+1)*grid+(walkx+1)].z+vertices[(walky)*grid+(walkx+1)].z;
                vertices[walky*grid+walkx].z=((val1+val2)/8)*(1+scalingfactor)*(1.40-getRandomFloat(40));
                maxheight=vertices[walky*grid+walkx].z>maxheight?vertices[walky*grid+walkx].z:maxheight;
                walky--;
            }
            while(walkx>walk){
                GLfloat val1=vertices[(walky)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx-1)].z+vertices[(walky-1)*grid+(walkx)].z+vertices[(walky-1)*grid+(walkx+1)].z;
                GLfloat val2=vertices[(walky+1)*grid+(walkx-1)].z+vertices[(walky+1)*grid+(walkx)].z+vertices[(walky+1)*grid+(walkx+1)].z+vertices[(walky)*grid+(walkx+1)].z;
                vertices[walky*grid+walkx].z=((val1+val2)/8)*(1+scalingfactor)*(1.40-getRandomFloat(40));
                maxheight=vertices[walky*grid+walkx].z>maxheight?vertices[walky*grid+walkx].z:maxheight;
                walkx--;
            }
            
            walk++;
        }

        for(int row=0;row<grid-1;row++){ /// pointleri yerlestirme
            for(int column=0;column<grid-1;column++){
                points[pointindex] = point4(negativehalfwidth+column*valueofinternalrow,negativehalfheight+row*valueofinternalcolumn,
                    vertices[row*grid+column].z,1); pointindex++;
                points[pointindex] = point4(negativehalfwidth+column*valueofinternalrow,negativehalfheight+(row+1)*valueofinternalcolumn,
                    vertices[(row+1)*grid+column].z,1); pointindex++;
                points[pointindex] = point4(negativehalfwidth+(column+1)*valueofinternalrow,negativehalfheight+(row+1)*valueofinternalcolumn,
                    vertices[(row+1)*grid+(column+1)].z,1); pointindex++;

                points[pointindex] = point4(negativehalfwidth+column*valueofinternalrow,negativehalfheight+row*valueofinternalcolumn,
                    vertices[row*grid+column].z,1); pointindex++;
                points[pointindex] = point4(negativehalfwidth+(column+1)*valueofinternalrow,negativehalfheight+(row+1)*valueofinternalcolumn,
                    vertices[(row+1)*grid+(column+1)].z,1); pointindex++;
                points[pointindex] = point4(negativehalfwidth+(column+1)*valueofinternalrow,negativehalfheight+row*valueofinternalcolumn,
                    vertices[row*grid+(column+1)].z,1); pointindex++;
            }
        }
    
        
        for(int row=0;row<grid-1;row++){  /// normalleri atama flatt shading
            for(int column=0;column<grid-1;column++){
                vec4 u = vertices[(row+1)*grid+column] - vertices[row*grid+column];
                vec4 v = vertices[(row+1)*grid+(column+1)] - vertices[row*grid+column];
                vec3 normal = -normalize( cross(u, v) );

                normals[normalindex] = normal;  normalindex++;
                normals[normalindex] = normal;  normalindex++;
                normals[normalindex] = normal;  normalindex++;

                u = vertices[(row+1)*grid+(column+1)] - vertices[row*grid+column];
                v = vertices[row*grid+(column+1)] - vertices[row*grid+column];
                normal = -normalize( cross(u, v) );
                
                normals[normalindex] = normal; normalindex++;
                normals[normalindex] = normal; normalindex++;
                normals[normalindex] = normal; normalindex++;
            }
        }
        
        if(!shadingtype){ /// normalleri atama gourraud shading
            for(int row=1;row<grid-2;row++){  
                for(int column=1;column<grid-2;column++){
                 

                    normalsGauraud[normalgouraudindex] =(normals[row*(grid-1)*2*3+column*6]+normals[row*(grid-1)*2*3+column*6-3]+
                    normalsGauraud[row*(grid-1)*2*3+column*6-3]+normals[(row-1)*(grid-1)*2*3+column*6]+normals[(row-1)*(grid-1)*2*3+column*6-3]+
                    normalsGauraud[(row-1)*(grid-1)*2*3+column*6]+normals[(row-1)*(grid-1)*2*3+column*6-6])/6;  
                    normalgouraudindex++;

                    normalsGauraud[normalgouraudindex] = (normals[(row+1)*(grid-1)*2*3+column*6]+normals[(row+1)*(grid-1)*2*3+column*6-3]+
                    normalsGauraud[(row+1)*(grid-1)*2*3+column*6-3]+normals[((row+1)-1)*(grid-1)*2*3+column*6]+normals[((row+1)-1)*(grid-1)*2*3+column*6-3]+
                    normalsGauraud[((row+1)-1)*(grid-1)*2*3+column*6]+normals[((row+1)-1)*(grid-1)*2*3+column*6-6])/6;  
                    normalgouraudindex++;

                    normalsGauraud[normalgouraudindex] = (normals[(row+1)*(grid-1)*2*3+(column+1)*6]+normals[(row+1)*(grid-1)*2*3+(column+1)*6-3]+
                    normalsGauraud[(row+1)*(grid-1)*2*3+(column+1)*6-3]+normals[((row+1)-1)*(grid-1)*2*3+(column+1)*6]+normals[((row+1)-1)*(grid-1)*2*3+(column+1)*6-3]+
                    normalsGauraud[((row+1)-1)*(grid-1)*2*3+(column+1)*6]+normals[((row+1)-1)*(grid-1)*2*3+(column+1)*6-6])/6;  
                    normalgouraudindex++;

                    normalsGauraud[normalgouraudindex] = (normals[row*(grid-1)*2*3+column*6]+normals[row*(grid-1)*2*3+column*6-3]+
                    normalsGauraud[row*(grid-1)*2*3+column*6-3]+normals[(row-1)*(grid-1)*2*3+column*6]+normals[(row-1)*(grid-1)*2*3+column*6-3]+
                    normalsGauraud[(row-1)*(grid-1)*2*3+column*6]+normals[(row-1)*(grid-1)*2*3+column*6-6])/6;  
                    normalgouraudindex++;

                    normalsGauraud[normalgouraudindex] = (normals[(row+1)*(grid-1)*2*3+(column+1)*6]+normals[(row+1)*(grid-1)*2*3+(column+1)*6-3]+
                    normalsGauraud[(row+1)*(grid-1)*2*3+(column+1)*6-3]+normals[((row+1)-1)*(grid-1)*2*3+(column+1)*6]+normals[((row+1)-1)*(grid-1)*2*3+(column+1)*6-3]+
                    normalsGauraud[((row+1)-1)*(grid-1)*2*3+(column+1)*6]+normals[((row+1)-1)*(grid-1)*2*3+(column+1)*6-6])/6;  
                    normalgouraudindex++;

                    normalsGauraud[normalgouraudindex] = (normals[row*(grid-1)*2*3+(column+1)*6]+normals[row*(grid-1)*2*3+(column+1)*6-3]+
                    normalsGauraud[row*(grid-1)*2*3+(column+1)*6-3]+normals[(row-1)*(grid-1)*2*3+(column+1)*6]+normals[(row-1)*(grid-1)*2*3+(column+1)*6-3]+
                    normalsGauraud[(row-1)*(grid-1)*2*3+(column+1)*6]+normals[(row-1)*(grid-1)*2*3+(column+1)*6-6])/6;  
                    normalgouraudindex++;
                }
            }
        }
        color4  materialColoryellow = color4( 0.70, 0.70, 0.55, 1.0);
        color4  materialColorgreen = color4( 0.33, 0.47, 0.22, 1.0);
        color4  materialColorsnow = color4( 0.9, 0.9, 0.9, 1.0);
        for(int index=0;index<numpoints;index++){ // material color atama
            if(points[index].z<maxheight/2){
                GLfloat xx= ((points[index].z-1)/((maxheight-1)/2))*materialColorgreen.x*(1.10-getRandomFloat(20)) +
                (1-((points[index].z-1)/((maxheight-1)/2)))*materialColoryellow.x*(1.10-getRandomFloat(20));
                GLfloat yy= ((points[index].z-1)/((maxheight-1)/2))*materialColorgreen.y*(1.10-getRandomFloat(20)) +
                (1-((points[index].z-1)/((maxheight-1)/2)))*materialColoryellow.y *(1.10-getRandomFloat(20));
                GLfloat zz= ((points[index].z-1)/((maxheight-1)/2))*materialColorgreen.z*(1.10-getRandomFloat(20)) +
                (1-((points[index].z-1)/((maxheight-1)/2)))*materialColoryellow.z*(1.10-getRandomFloat(20));
                materialColors[index]=color4( xx, yy, zz, 1.0);
            }
            else{
                GLfloat xx= (((points[index].z-1)-maxheight/2)/((maxheight-1-maxheight/2)))*materialColorsnow.x*(1.10-getRandomFloat(20)) +
                (1-(((points[index].z-1)-maxheight/2)/((maxheight-1-maxheight/2))))*materialColorgreen.x*(1.10-getRandomFloat(20));
                GLfloat yy= (((points[index].z-1)-maxheight/2)/((maxheight-1-maxheight/2)))*materialColorsnow.y*(1.10-getRandomFloat(20)) +
                (1-(((points[index].z-1)-maxheight/2)/((maxheight-1-maxheight/2))))*materialColorgreen.y *(1.10-getRandomFloat(20));
                GLfloat zz= (((points[index].z-1)-maxheight/2)/((maxheight-1-maxheight/2)))*materialColorsnow.z*(1.10-getRandomFloat(20)) +
                (1-(((points[index].z-1)-maxheight/2)/((maxheight-1-maxheight/2))))*materialColorgreen.z*(1.10-getRandomFloat(20));
                materialColors[index]=color4( xx, yy, zz, 1.0);
            }
        }
   

        points[pointindex++]=point4(eyeX-3,eyeY-2,radius,1.0); /// plane initial position hesaplama
        points[pointindex++]=point4(eyeX+3,eyeY-2,radius,1.0);
        points[pointindex++]=point4(eyeX,eyeY+4,radius,1.0);
        materialColors[numpoints-3]=color4(0.0,0.0,0.0,1);
        materialColors[numpoints-3]=color4(0.0,0.0,0.0,1);
        materialColors[numpoints-3]=color4(0.0,0.0,0.0,1);

        glGenVertexArrays( 1, &vao );
        glBindVertexArray( vao );
        // Create and initialize a buffer object
        
        glGenBuffers( 1, &buffer );
        glBindBuffer( GL_ARRAY_BUFFER, buffer );
        glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals)+sizeof(materialColors),
            NULL, GL_STATIC_DRAW );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
        if(shadingtype){ /// normal gonderme
             glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
        }
        else{
            glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normalsGauraud );
        }

        glBufferSubData( GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals), sizeof(materialColors), materialColors );// metireial color gonderme
        
        // Load shaders and use the resulting shader program
        program = InitShader( "hw3sh.glsl", "hw3shCOLOR.glsl" );
        glUseProgram( program );
                
        // set up vertex arrays
        GLuint vPosition = glGetAttribLocation( program, "vPosition" );
        glEnableVertexAttribArray( vPosition );
        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                BUFFER_OFFSET(0) );
        GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
        glEnableVertexAttribArray( vNormal );
        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                BUFFER_OFFSET(sizeof(points)) );
        GLuint vMaterialColor = glGetAttribLocation( program, "materialcolor" ); 
        glEnableVertexAttribArray( vMaterialColor );
        glVertexAttribPointer( vMaterialColor, 4, GL_FLOAT, GL_FALSE, 0,
                BUFFER_OFFSET(sizeof(points)+sizeof(normals)) );

  
        //// colors normal--------------------------------------------------
        point4 light_position( 1.0, 1.0, 1.0, 1.0 ); /// modeldeki light kaynagı
        color4 light_ambient( 0.3, 0.3, 0.3, 1.0 );
        color4 light_diffuse( 0.9,0.9,0.9,1.0 );
        color4 light_specular( 0.5, 0.5, 0.5, 1.0 );

        float  material_shininess = 20.0;

        color4 ambient = light_ambient ; // sadece light
        color4 diffuse = light_diffuse ;
        color4 specular = light_specular ;

          glUniform4fv( glGetUniformLocation(program, "Ambient"),
		  1, ambient );
        glUniform4fv( glGetUniformLocation(program, "Diffuse"),
            1, diffuse );
        glUniform4fv( glGetUniformLocation(program, "Specular"),
            1, specular );
        
        glUniform4fv( glGetUniformLocation(program, "LightPosition"),
            1, light_position );

        glUniform1f( glGetUniformLocation(program, "Shininess"),
            material_shininess );
        //// colors normal--------------------------------------------------
        
    
        model_view = glGetUniformLocation( program, "model_view" );
        projection = glGetUniformLocation( program, "projection" );
        
        glEnable( GL_DEPTH_TEST );
        glShadeModel(GL_SMOOTH);
        glClearColor( 0.0, 0.0, 1.0, 1.0 ); 
    
    
}
//----------------------------------------------------------------------------


void display( void )
{
   
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    point4  eye;
    point4  at;
    point4  up;
    if(godmode){
        eye= point4(0,0,radius*2.0,1.0 );
        at = point4( 0.0, 0.0, 0.0, 1.0 );
        up=    vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    else if(pilotmode) {
        eye= point4( eyeX+cos(((velocitydegree)*(2 * 3.14159))/360),eyeY+3*(sin(((velocitydegree)*(2 * 3.14159))/360)),radius+1,1.0 );
        at = point4( eyeX+(radius*(cos((velocitydegree*(2 * 3.14159))/360))), eyeY+(radius*(sin((velocitydegree*(2 * 3.14159))/360))),
        0.0, 1.0 );
        up=    vec4( 0.0, 0.0, 1.0, 0.0 );
    }
        
    else if(pilotbehindmode){
        eye= point4( eyeX-2*cos(((velocitydegree)*(2 * 3.14159))/360),eyeY-2*(sin(((velocitydegree)*(2 * 3.14159))/360)),radius+3,1.0 );
        at = point4( eyeX+(radius*(cos((velocitydegree*(2 * 3.14159))/360))), eyeY+(radius*(sin((velocitydegree*(2 * 3.14159))/360))),
        0.0, 1.0 );
        up=    vec4( 0.0, 0.0, 1.0, 0.0 );
    }
    
    mat4  mv = LookAt( eye, at, up );
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );

    mat4  p = Perspective( fovy, aspect, zNear, zFar );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );

    
    glDrawArrays( GL_TRIANGLES, 0, numPoints ); 

    
    glutSwapBuffers();
}
void resetview(){ // view mod degisimlerinde
    godmode=false;
    pilotbehindmode=false;
    pilotmode=false;
}
//----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{
    
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    case 'f': shadingtype=!shadingtype; init(); break; 
    case 'c': resetview(); pilotmode=true; break; 
	case 't': resetview(); pilotbehindmode=true; break;
    case 'w': resetview(); godmode=true; break;
    case 'a': velocityofplane = velocityofplane*1.1; break;
    case 'd': velocityofplane = velocityofplane*0.9; break;
    case '4': velocitydegree=velocitydegree+1.0; break;
    case '6': velocitydegree=velocitydegree-1.0; break;
    
    }
    glutPostRedisplay();
}
void movePlane(){ // plane ve pilot konumu degistirme
    eyeX=eyeX+cos((velocitydegree*(2 * 3.14159))/360)*velocityofplane;
    eyeY=eyeY+sin((velocitydegree*(2 * 3.14159))/360)*velocityofplane;
    
        point4 plane[3];
        plane[0]=point4(eyeX+cos(((velocitydegree)*(2 * 3.14159))/360),eyeY+3*(sin(((velocitydegree)*(2 * 3.14159))/360)),radius,1.0);
        plane[1]=point4(eyeX+cos(((velocitydegree+120)*(2 * 3.14159))/360),eyeY+sin(((velocitydegree+120)*(2 * 3.14159))/360),radius,1.0);
        plane[2]=point4(eyeX+cos(((velocitydegree+240)*(2 * 3.14159))/360),eyeY+(sin(((velocitydegree+240)*(2 * 3.14159))/360)),radius,1.0);
        glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*(numPoints-3), sizeof(point4)*3, plane );
    
}

void myTimeOut(int id) {
	movePlane();
	glutPostRedisplay();			 
	glutTimerFunc(10, myTimeOut, 0);  
}
//----------------------------------------------------------------------------
void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = GLfloat(width)/height;
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    increaseStackSize();
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 662, 662 );
//    glutInitContextVersion( 3, 2 );
//    glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow( "hw3" );
	glewExperimental = GL_TRUE;
    glewInit();

    init();
    display();
    glutTimerFunc(1000, myTimeOut, 0);

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );

    glutMainLoop();
    return 0;
}
