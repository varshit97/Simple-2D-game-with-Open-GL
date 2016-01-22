/*********************************** Nothing to be Changed in this section **********************************/
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define PI M_PI
using namespace std;
typedef struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
}VAO;
struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;
GLuint programID;
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    return ProgramID;
}
VAO* create3DObject(GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    glGenVertexArrays(1, &(vao->VertexArrayID));
    glGenBuffers (1, &(vao->VertexBuffer));
    glGenBuffers (1, &(vao->ColorBuffer));
    glBindVertexArray (vao->VertexArrayID); 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer);
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    return vao;
}
void draw3DObject (struct VAO* vao)
{
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices);
}
float formatAngle(float A)
{
    if(A<0.0f)
        return A+360.0f;
    if(A>=360.0f)
        return A-360.0f;
    return A;
}
float D2R(float A)
{
    return (A*PI)/180.0f;
}
/*** collision part ***/
typedef pair< float, float > dub;
typedef pair< dub ,float > tup;
#define F first
#define S second
#define mp make_pair
#define pb push_back
float sqr(float x)
{
    return x*x;
}
float dis(dub p,dub q)
{
    return sqr(p.F-q.F)+sqr(p.S-q.S);
}
int collided(tup p,tup q)
{
    float cdis=dis(p.F,q.F);
    float rsum=sqr(p.S+q.S);
    if(cdis<=rsum)
        return 1;
    else
        return 0;
}
/******************************************* Beginning of Main Code  *************************************************/
float rectangle_rot_dir = 1;
bool rectangle_rot_status = false;
int width,height;
float xpos=-200.0,ypos=-200.0;
float xpos1,ypos1;
float xcenter=-200.0,ycenter=-200.0;
float radius=0.0,rotation=180.0;
bool atstart=true;
bool noconstraints=false;
float T=0;
VAO *rectangle;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
VAO* partCircle[20];
float vertCircle[100];
VAO* sector;
VAO* shoot;
VAO* ball;
VAO* bottom;
float rotationAngle;
float dist;
float velocity;
/************ collision *******************/
vector< tup > centre[1000];
glm::vec3 trans[1000];
float rotat[1000];
VAO* objects[1000];
float velx[1000];
float vely[1000];
bool movable[1000];
float Timer[1000];
float Mass[1000];
float COR=0.6f;
float startX[1000];
float startY[1000];
float currentX[1000];
float currentY[1000];
float ADG=1.0f;
float tick=0.6f;
bool checkCollision(int i,int j)
{
    float R;
    tup A,B,V;
    for(int k=0;k<centre[i].size();k++)
    {
        for(int l=0;l<centre[j].size();l++)
        {
            V=centre[i][k];
            R=V.F.F;
            A.F.F=trans[i][0]+R*cos(D2R(formatAngle(rotat[i])));
            A.F.S=trans[i][1]+R*sin(D2R(formatAngle(rotat[i])));
            A.S=V.S;

            V=centre[j][l];
            R=V.F.F;
            B.F.F=trans[j][0]+R*cos(D2R(formatAngle(rotat[j])));
            B.F.S=trans[j][1]+R*sin(D2R(formatAngle(rotat[j])));
            B.S=V.S;
            if(collided(A,B))
            {
                //cout << currentX[0] << " " << currentY[0] << "\n";
                //cout << A.F.F << " " << A.F.S << " " << A.S << "\n";
                //cout << B.F.F << " " << B.F.S << " " << B.S << "\n";
                //cout << k << " bajesh " << l << " kajesh " << i << " varshit " << j << "\n";
                return true;
            }
        }
    }
    return false;
}
/******************************************/
/* Executed when a regular key is pressed */
void keyboardDown(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}
/* Executed when a regular key is released */
void keyboardUp(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 32:
            if(atstart)
            {
                atstart=false;
                velx[0]=velocity*cos(D2R(formatAngle(rotationAngle)));
                vely[0]=velocity*sin(D2R(formatAngle(rotationAngle)));
            }
            break;
        case 'b':
            atstart=true;
            T=0;
            break;
    }
}
/* Executed when a special key is pressed */
void keyboardSpecialDown(int key, int x, int y)
{
}
/* Executed when a special key is released */
void keyboardSpecialUp(int key, int x, int y)
{
}
/* Executed when a mouse button 'button' is put into state 'state' at screen position ('x', 'y') */
void mouseClick(int button, int state, int x, int y)
{
    cerr << x << y << "\n";
}
/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion(int x, int y)
{
    //cerr << x << y << "\n";
}
void cursorPos(int x, int y)
{
    if(atstart)
    {
        rotationAngle=(atan2(650.0f-y,x-100.0f)*180.0f)/PI;
        dist=(650.0f-y)*(650.0f-y)+(x-100.0f)*(x-100.0f);
        dist=min(dist,80000.0f);
        velocity=(38.0f*dist)/80000.0f;
    }
}
/* Executed when window is resized to 'width' and 'height' */
void reshapeWindow(int width, int height)
{
    GLfloat fov = 90.0f;
    glViewport (0, 0, (GLsizei) width, (GLsizei) height);
    Matrices.projection = glm::ortho(-width/2.0f,width/2.0f,-height/2.f,height/2.0f,0.1f, 500.0f);
}
VAO* createRectangle(float x,float y)
{
    GLfloat vertex_buffer_data [] = {
        -x,-y,0.0, // vertex 1
        x,-y,0.0, // vertex 2
        x, y,0.0, // vertex 3

        x, y,0.0, // vertex 3
        -x, y,0.0, // vertex 4
        -x,-y,0.0  // vertex 1
    };

    GLfloat color_buffer_data [] = {
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0, // color 3

        0,1,0, // color 3
        0.3,0.3,0.3, // color 4
        1,0,0  // color 1
    };
    // create3DObject creates and returns a handle to a VAO that can be used later
    return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void drawobject(VAO* obj,glm::vec3 trans,float angle,glm::vec3 rotat)
{
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 VP = Matrices.projection * Matrices.view;
    glm::mat4 MVP;	// MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translatemat = glm::translate(trans);
    glm::mat4 rotatemat = glm::rotate(D2R(formatAngle(angle)), rotat);
    Matrices.model *= (translatemat * rotatemat);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(obj);
}
float EKMT(float K,float M,float T)
{
    return exp(-((K/M)*T));
}
float xvel(float V0,float K,float M,float T)
{
    return V0*EKMT(K,M,T);
}
float xdis(float V0,float K,float M,float T)
{
    return ((M/K)*V0)*(1.0f-EKMT(K,M,T));
}
float yvel(float V0,float K,float M,float T,float G)
{
    return ((V0+((M*G)/K))*EKMT(K,M,T))-((M*G)/K);
}
float ydis(float V0,float K,float M,float T,float G)
{
    return ((M/K)*(V0+((M*G)/K))*(1-EKMT(K,M,T)))-((M*G*T)/K);
}
float equilib(int i)
{
    return (Mass[i]/0.3f)*((vely[i]*0.3f+1.0f)/(Mass[i]*ADG));
}
/* Render the scene with openGL */
void conserveMomentum(int i,int j)
{
    float u1,u2,v1,v2;
    u1=xvel(velx[i],0.3f,Mass[i],Timer[i]);
    u2=xvel(velx[j],0.3f,Mass[j],Timer[j]);
    v1=(((Mass[i]-COR*Mass[j])*u1)/(Mass[i]+Mass[j]))+(((Mass[j]+COR*Mass[j])*u2)/(Mass[i]+Mass[j]));
    v2=u1*COR-u2*COR+v1;
    velx[i]=v1;
    velx[j]=v2;
    u1=yvel(vely[i],0.3f,Mass[i],Timer[i],ADG);
    u2=yvel(vely[j],0.3f,Mass[j],Timer[j],ADG);
    v1=(((Mass[i]-COR*Mass[j])*u1)/(Mass[i]+Mass[j]))+(((Mass[j]+COR*Mass[j])*u2)/(Mass[i]+Mass[j]));
    v2=u1*COR-u2*COR+v1;
    vely[i]=v1;
    vely[j]=v2;
}
bool flag=true;
void applyCollisions()
{
    for(int i=0;i<5;i++)
    {
        for(int j=i+1;j<5;j++)
        {
            //if(i==1 || j==1)
            //continue;
            if( ((velx[i]!=0 || vely[i]!=0) || (velx[j]!=0 || vely[j]!=0) ) && (movable[i] || movable[j]) && checkCollision(i,j) )
            {
            //if(checkCollision(0,3))
                //cout << "Oho\n";
                //cout << i << " " << j << "HOHOHO\n";
                if(!movable[i] && abs(yvel(vely[j],0.3f,Mass[j],Timer[j],ADG))>0.01f)
                {
                    if(i==2)
                    {
                        vely[j]=-COR*yvel(vely[j],0.3f,Mass[j],Timer[j],ADG);
                        startX[j]=currentX[j];
                        startY[j]=currentY[j];
                        Timer[j]=tick;
                    }
                }
                //else if(!movable[j])
                else if((movable[i] && !movable[j]) && abs(yvel(vely[i],0.3f,Mass[i],Timer[i],ADG))>0.01f)
                {
                    if(j==2)
                    {
                        //cout << i << "\n";
                        //cout << currentX[1] << " " << currentY[1] << " raja cheppadu\n";
                        vely[i]=-COR*yvel(vely[i],0.3f,Mass[i],Timer[i],ADG);
                        startX[i]=currentX[i];
                        startY[i]=currentY[i];
                        Timer[i]=0.0f;
                        while(abs(yvel(vely[i],0.3f,Mass[i],Timer[i],ADG))>0.01f && checkCollision(i,j))
                        {
                            Timer[i]+=0.05f;
                            trans[i][0]=currentX[i]=startX[i]+xdis(velx[i],0.3f,Mass[i],Timer[i]);
                            trans[i][1]=currentY[i]=startY[i]+ydis(vely[i],0.3f,Mass[i],Timer[i],ADG);
                        }
                        if(trans[i][1]<-320.0f)
                        {
                            trans[i][1]=currentY[i]=-319.0f;
                            if(Timer[i]>equilib(i))
                            {
                                velx[i]=0.0f;
                                vely[i]=0.0f;
                                Timer[i]=0.0f;
                                startX[i]=currentX[i];
                                startY[i]=currentY[i];
                            }
                            /*int temp=true;
                              for(int k=0;k<3;k++)
                              if(k!=i && k!=j && checkCollision(i,k))
                              temp=false;
                              if(temp)
                              velx[i]=0.0f;*/
                        }
                        /*bool falg=false;
                          if(checkCollision(i,j))
                          {
                          flag=true;
                          vely[i]=2.0f;
                          startX[i]=currentX[i];
                          startY[i]=currentY[i];
                          while(checkCollision(i,j))
                          {
                          Timer[i]+=0.1f;
                          trans[i][0]=currentX[i]=startX[i]+xdis(velx[i],0.3f,Mass[i],Timer[i]);
                          trans[i][1]=currentY[i]=startY[i]+ydis(vely[i],0.3f,Mass[i],Timer[i],ADG);
                          }
                          }
                          if(falg)
                          vely[i]=velx[i]=0.0f;
                          cout << currentX[1] << " " << currentY[1] << " bajesh cheppadu\n";*/
                        //int x;
                        //cin >> x;
                    }
                    if(j==3)
                    {
                        //cout << "Hola " << i << "\n";
                        velx[i]=-COR*xvel(velx[i],0.3f,Mass[i],Timer[i]);
                        startX[i]=currentX[i];
                        startY[i]=currentY[i];
                        Timer[i]=0.0f;
                        while(abs(xvel(vely[i],0.3f,Mass[i],Timer[i]))>0.01f && checkCollision(i,j))
                        {
                            Timer[i]+=0.05f;
                            trans[i][0]=currentX[i]=startX[i]+xdis(velx[i],0.3f,Mass[i],Timer[i]);
                            trans[i][1]=currentY[i]=startY[i]+ydis(vely[i],0.3f,Mass[i],Timer[i],ADG);
                        }
                    }
                }
                else if(movable[i] && movable[j])
                {
                    //cout << velx[i] << " " << vely[i] << "\n";
                    //cout << velx[j] << " " << vely[j] << "\n";
                    conserveMomentum(i,j);
                    //cout << velx[i] << " " << vely[i] << "\n";
                    //cout << velx[j] << " " << vely[j] << "\n";
                    // Iterate over all objects to move time frame - TODO
                    startX[i]=currentX[i];
                    startY[i]=currentY[i];
                    startX[j]=currentX[j];
                    startY[j]=currentY[j];
                    Timer[i]=tick;
                    Timer[j]=tick;
                    //rotat[i]=30.0f;
                    //rotat[j]=30.0f;
                    while(checkCollision(i,j))
                    {
                        Timer[i]+=0.05f;
                        Timer[j]+=0.05f;
                        trans[i][0]=currentX[i]=startX[i]+xdis(velx[i],0.3f,Mass[i],Timer[i]);
                        trans[i][1]=currentY[i]=startY[i]+ydis(vely[i],0.3f,Mass[i],Timer[i],ADG);

                        trans[j][0]=currentX[j]=startX[j]+xdis(velx[j],0.3f,Mass[j],Timer[j]);
                        trans[j][1]=currentY[j]=startY[j]+ydis(vely[j],0.3f,Mass[j],Timer[j],ADG);
                    }
                    flag=false;
                }
            }
        }
    }
}
void updatePositions()
{
    radius=83.0f;
    if(atstart)
    {
        currentX[0]=-550.0f+radius*cos(D2R(rotationAngle));
        currentY[0]=-300.0f+radius*sin(D2R(rotationAngle));
        startX[0]=currentX[0];
        startY[0]=currentY[0];
    }
    for(int i=atstart;i<5;i++)
    {
        float veloy=yvel(vely[i],0.3f,Mass[i],Timer[i],ADG);
        float velox=xvel(velx[i],0.3f,Mass[i],Timer[i]);
        if(velx[i]!=0.0f || vely[i]!=0.0f)
        {
            //currentX[i]=startX[i]+velx[i]*Timer[i];
            //currentY[i]=startY[i]+vely[i]*Timer[i];
            currentX[i]=startX[i]+xdis(velx[i],0.3f,Mass[i],Timer[i]);
            currentY[i]=startY[i]+ydis(vely[i],0.3f,Mass[i],Timer[i],ADG);
            trans[i][0]=currentX[i];
            trans[i][1]=currentY[i];
            Timer[i]+=tick;
        }
    }

}
void draw ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (programID);

    applyCollisions();
    updatePositions();
    // ball
    for(int i=0;i<18;i++)
    {
        drawobject(ball,glm::vec3(currentX[0],currentY[0],0.0f),i*20.0f,glm::vec3(0,0,1));
    }

    // shoot
    drawobject(shoot,glm::vec3(-550.0f+55.0*cos(D2R(formatAngle(rotationAngle))),-300.0f+55.0*sin(D2R(formatAngle(rotationAngle))),0.0f),rotationAngle,glm::vec3(0,0,1));

    // cannon
    for(int i=0;i<18;i++)
    {
        drawobject(sector,glm::vec3(-550.0f,-300.0f,0.0f),i*20.0f+rotationAngle,glm::vec3(0,0,1));
    }
    //cout << "1 " << trans[1][0] << " " << trans[1][1] << " " << velx[1] << " " << vely[1] << " " << Timer[1] << "\n";
    //cout << "0 " << trans[0][0] << " " << trans[0][1] << " " << velx[0] << " " << vely[0] << " " << Timer[0] << "\n";
    //cout << yvel(12.2594,0.3f,Mass[0],12.15,ADG) << "\n";
    drawobject(objects[4],trans[4],rotat[4],glm::vec3(0,0,1));
    drawobject(objects[3],trans[3],rotat[3],glm::vec3(0,0,1));
    drawobject(objects[2],trans[2],rotat[2],glm::vec3(0,0,1));
    drawobject(objects[1],trans[1],rotat[1],glm::vec3(0,0,1));
    glutSwapBuffers ();
}

void idle()
{
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}
/* Initialise glut window, I/O callbacks and the renderer to use */
void initGLUT(int& argc, char** argv, int width, int height)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_CORE_PROFILE);
    glutInitWindowSize(width, height);
    glutCreateWindow("Learning OGL3.3");
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutPassiveMotionFunc( cursorPos);
    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)

    //glutIgnoreKeyRepeat (true); // Ignore keys held down*/
}
VAO* createTriangle(float X1,float Y1,float X2,float Y2,float X3,float Y3)
{
    GLfloat vertex_buffer_data[]={X1,Y1,0.0f,X2,Y2,0.0f,X3,Y3,0.0f};
    GLfloat color_buffer_data[]={1,0,0,1,0,0,1,0,0};
    return create3DObject(GL_TRIANGLES,3,vertex_buffer_data,color_buffer_data,GL_FILL);
}
VAO* createLine(float X1,float Y1,float X2,float Y2)
{
    GLfloat vertex_buffer_data[]={X1,Y1,0.0f,X2,Y2,0.0f};
    GLfloat color_buffer_data[]={1,0,0,1,0,0};
    return create3DObject(GL_LINES,3,vertex_buffer_data,color_buffer_data,GL_LINE);
}
VAO* createSector(float R,int parts)
{
    float diff=360.0f/parts;
    float A1=formatAngle(-diff/2);
    float A2=formatAngle(diff/2);
    GLfloat vertex_buffer_data[]={0.0f,0.0f,0.0f,R*cos(D2R(A1)),R*sin(D2R(A1)),0.0f,R*cos(D2R(A2)),R*sin(D2R(A2)),0.0f};
    GLfloat color_buffer_data[]={1,0,0,1,0,0,1,0,0};
    //GLfloat color_buffer_data[]={1,1,1,0,0,0,0,0,0};
    return create3DObject(GL_TRIANGLES,3,vertex_buffer_data,color_buffer_data,GL_FILL);
}
void divideRect(int i,float width,float height)
{
    for(int j=0;j<(int)(width/(2.0*height));j++)
    {
        centre[i].pb(mp(mp(-(2.0f*j+1.0f)*height,0.0f),height));
    }
    for(int j=0;j<(int)(width/(2.0*height));j++)
    {
        centre[i].pb(mp(mp((2.0f*j+1.0f)*height,0.0f),height));
    }
}
void initGL(int width, int height)
{
    sector=createSector(30.0f,4);
    shoot=createRectangle(40.0f,12.0f);
    ball=createSector(12.0f,4);
    //bottom=createLine(0.0f,0.0f,1300.0f,0.0f);
    //  ball
    centre[0].pb(mp(mp(0.0f,0.0f),12.0f));
    trans[0]=glm::vec3(0.0f,0.0f,0.0f);
    currentX[0]=0.0f;
    currentY[0]=0.0f;
    rotat[0]=0.0f;
    movable[0]=true;
    Mass[0]=200.0f;
    Timer[0]=0.0f;
    velx[0]=0.0f;
    vely[0]=0.0f;
    // rectangle
    objects[1]=createRectangle(48.0f,12.0f);
    divideRect(1,48.0f,12.0f);
    trans[1]=glm::vec3(-100.0f,-100.0f,0.0f);
    startX[1]=-100.0f;
    startY[1]=-100.0f;
    currentX[1]=-100.0f;
    currentY[1]=-100.0f;
    rotat[1]=0.0f;
    movable[1]=true;
    Mass[1]=350.0f;
    Timer[1]=0.0f;
    velx[1]=0.0f;
    vely[1]=0.0f;
    // Ground
    objects[2]=createRectangle(650.0f,10.0f);
    divideRect(2,650.0f,10.0f);
    trans[2]=glm::vec3(0.0f,-340.0f,0.0f);
    rotat[2]=0.0f;
    movable[2]=false;
    // Side Wall
    objects[3]=createRectangle(350.0f,10.0f);
    divideRect(3,350.0f,10.0f);
    trans[3]=glm::vec3(640.0f,0.0f,0.0f);
    rotat[3]=90.0f;
    movable[3]=false;
    
    // objects
    objects[4]=createRectangle(100.0f,10.0f);
    divideRect(4,100.0f,10.0f);
    trans[4]=glm::vec3(300.0f,-280.0f,0.0f);
    rotat[4]=90.0f;
    currentX[4]=trans[4][0];
    currentY[4]=trans[4][1];
    movable[4]=true;
    Mass[4]=350.0f;
    Timer[4]=0.0f;
    velx[4]=0.0f;
    vely[4]=0.0f;

    programID=LoadShaders("Sample_GL.vert","Sample_GL.frag");
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    reshapeWindow (width, height);
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f);
    glClearDepth (1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
}
int main (int argc, char** argv)
{
    width = 1301;
    height = 701;
    initGLUT (argc, argv, width, height);
    initGL(width, height);
    glutMainLoop ();
    return 0;
}
