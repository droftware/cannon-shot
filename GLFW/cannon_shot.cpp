#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

const float LEFT_BOUND = -52.0f;
const float RIGHT_BOUND = 52.0f;
const float TOP_BOUND = 24.0f;
const float BOTTOM_BOUND = -24.0f;


struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
};
typedef struct GLMatrices GLMatrices;

GLMatrices Matrices;
GLuint programID;


class Circle{
public:
  Circle(GLMatrices *mtx, float* color, float cx=0, float cy=0, float radius=3, int numPolygons =100);
  Circle(const Circle& cr);
  void swap(Circle &first, Circle &other);
  Circle& operator=(Circle other);
  ~Circle();
  void setCenter(float x, float y);
  void setRadius(float r);
  void setNumPolygons(int num);
  float getCenterX();
  float getCenterY();
  float getRadius();
  VAO* getVAO();
  void draw();
private:
  GLfloat *vertex_buffer_data;
  GLfloat *color_buffer_data;
  VAO *vaobj;
  float cx;
  float cy;
  float radius;
  int numPolygons;
  GLMatrices *mtx;
};

class Rectangle{
public:
  Rectangle(GLMatrices *mtx, float* color,int x = 0, int y = 0, int width = 2, int height = 3, int angle = 0);
  Rectangle(const Rectangle& rect);
  ~Rectangle();
  VAO* getVAO();
  int getTopLeftX();
  int getTopLeftY();
  int getWidth();
  int getHeight();
  float getAngle();
  float getPosAngle();
  glm::vec3 getAxis();
  void setTopLeftX(int x);
  void setTopLeftY(int y);
  void setWidth(int w);
  void setHeight(int h);
  void setAngle(float angle);
  void setAxis(glm::vec3 &axis);
  virtual void draw();
private:
  GLfloat *vertex_buffer_data;
  GLfloat *color_buffer_data;
  GLMatrices *mtx;
  VAO *vaobj;
  int x;
  int y;
  int width;
  int height;
  float angle;
  glm::vec3 axis;
};

class Item{
public:
  Item(float mass, float x, float y, float ux, float uy, float radius);
  virtual void applyForces(float timeInstance);
  void applyGravity();
  void applyNormalForce();
  void applyAcceleration();
  void applyPosition(float timeInstance);
  virtual void applyOtherForces();
  void applyCollisionGround();
  void applyFriction();
  bool checkCollisionGround();
  bool checkStoppage();
  void setSpeed(float ux, float uy);
  void setPosition(float x, float y);
  void setTime(float time);
  float getPositionX();
  float getPositionY();
  friend bool checkCollisionItem(Item &first, Item &second);
  friend void simulateCollisionItem(Item &first, Item &second);
  friend void handleCollisionsItem();
protected:
  static const float GRAVITY = 75.0f;
  static const float BOUNCE_COF = 0.4f;
  static const float FRICTION_COF = 0.1f;
  float mass;
  float forceX;
  float forceY;
  float x;
  float y;
  float prevX;
  float prevY;
  float ux;
  float uy;
  float ax;
  float ay;
  float time;
  float radius;
};

class Bomb : public Item{
public:
  Bomb(GLMatrices *mtx, float cx, float cy, float speedX, float speedY);
  bool getDynamic();
  void draw();
  void applyForces(float timeInstance);
  void setDynamic(bool value);
private:
  Circle *circ;
  bool dynamic;
};

class Cannon{
public:
  Cannon(GLMatrices *mtx, int x = -48, int y = -21);
  ~Cannon();

  void barrelUp();
  void barrelDown();
  void shoot();
  void draw();
  void applyForces(float timeInstance);
private:
  Circle *tank;
  Rectangle *barrel;
  GLMatrices *mtx;
  Bomb *ammo;
  float bombInitSpeed;
};
//Rectangle(GLMatrices *mtx, float* color,int x = 0, 
//int y = 0, int width = 2, int height = 3, int angle = 0);
class Block
{
public:
  Block(GLMatrices *mtx, int x, int y, int width, int height);
  float getPositionY();
  float getPositionX();
  float getHeight();
  float getWidth();
  void draw();
private:
  Rectangle *rect;
};

class Target:public Item
{
public:
  Target(GLMatrices *mtx, Block* pillar);
  void draw();
  void applyOtherForces();
  void applyForces(float timeInstance);
private:
  Circle *circ;
  Block *pillar;
};

Circle *c;
Rectangle *r;
Cannon *can;
Block *b1;
Target *t1;
std::vector<Item*> movableList;
void handleCollisionsItem();

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
                                        
	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
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

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

Circle::Circle(GLMatrices *mtx, float* color, float cx, float cy, float radius, int numPolygons)
{
  vertex_buffer_data = new GLfloat[500 * 9];
  color_buffer_data = new GLfloat[500 * 9];
  this->cx = cx;
  this->cy = cy;
  this->radius = radius;
  this->numPolygons = numPolygons;
  this->mtx = mtx;
  float theta;
  for(int i = 0; i <= numPolygons; i++)
  {
    //Adding vertices
    theta = 2.0f * 3.1415926f * float(i) / float(numPolygons);
    if(i==0){
      vertex_buffer_data[9*i + 0] = radius;
      vertex_buffer_data[9*i + 1] = 0;
      vertex_buffer_data[9*i + 2] = 0;
    }
    else{
      vertex_buffer_data[9*i + 0] = vertex_buffer_data[9*i - 3];
      vertex_buffer_data[9*i + 1] = vertex_buffer_data[9*i - 2];
      vertex_buffer_data[9*i + 2] = vertex_buffer_data[9*i - 1];
    }

    vertex_buffer_data[9*i + 3] =  0;
    vertex_buffer_data[9*i + 4] =  0;
    vertex_buffer_data[9*i + 5] =  0;

    vertex_buffer_data[9*i + 6] =  radius * cosf(theta);
    vertex_buffer_data[9*i + 7] =  radius * sinf(theta);
    vertex_buffer_data[9*i + 8] =  0;

    //Adding color
    for(int j=0;j<9;j+=3){
      color_buffer_data[9*i + j + 0] = color[0];
      color_buffer_data[9*i + j + 1] = color[1];
      color_buffer_data[9*i + j + 2] = color[2];
    }
  }
  vaobj = create3DObject(GL_TRIANGLES, 3*numPolygons + 3, vertex_buffer_data, color_buffer_data, GL_FILL);

}

//copy constructor
Circle::Circle(const Circle& circ){
  cout<<"Entered circle copy constructor"<<endl;
  vertex_buffer_data = new GLfloat[500 * 9];
  color_buffer_data = new GLfloat[500 * 9];
  this->cx = circ.cx;
  this->cy = circ.cy;
  this->radius = circ.radius;
  this->numPolygons = circ.numPolygons;
  this->mtx = circ.mtx;
  for(int i = 0; i <= 9*numPolygons; i++){
      vertex_buffer_data[i] = circ.vertex_buffer_data[i];
      color_buffer_data[i] = circ.color_buffer_data[i];
  }
  vaobj = create3DObject(GL_TRIANGLES, 3*numPolygons + 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void Circle::setCenter(float x, float y){
  cx = x;
  cy = y;
}

void Circle::setRadius(float r){
  radius = r;
}

void Circle::setNumPolygons(int x){
  numPolygons = x;
}

float Circle::getCenterX(){
  return cx;
}

float Circle::getCenterY(){
  return cy;
}

float Circle::getRadius(){
  return radius;
}

VAO* Circle::getVAO(){
  return vaobj;
}

void Circle::draw(){
  //std::cout<<"Entered draw funtion"<<cx<<endl;
  glm::mat4 MVP; // MVP = Projection * View * Model
  // Load identity to model matrix
  mtx->model = glm::mat4(1.0f);

  /* Render your scene */
  //std::cout<<"Centre = X - "<<cx<<endl;
  //std::cout<<"Centre = Y - "<<cy<<endl;
  glm::mat4 mtranslate = glm::translate (glm::vec3(cx, cy, 0.0f)); // glTranslatef
  //glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 mtransform = mtranslate;
  mtx->model *= mtransform;
  MVP =  mtx->projection * mtx->view * mtx->model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(mtx->MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(vaobj);

}

void Circle::swap(Circle &first, Circle &second){
  using std::swap;
  swap(first.vertex_buffer_data, second.vertex_buffer_data);
  swap(first.color_buffer_data, second.color_buffer_data);
  swap(first.vaobj, second.vaobj);
  swap(first.cx, second.cx);
  swap(first.cy, second.cy);
  swap(first.radius, second.radius);
  swap(first.numPolygons, second.numPolygons);
  swap(first.mtx, second.mtx);
}

Circle& Circle::operator=(Circle other) // (1)
{
    cout<<"Called circle operator="<<endl;
    swap(*this, other); // (2)

    return *this;
}

Circle::~Circle(){
  delete[] vertex_buffer_data;
  delete[] color_buffer_data;
}

Rectangle::Rectangle(GLMatrices *mtx, float* color,int x, int y, int width, int height, int angle)
{
  vertex_buffer_data = new GLfloat[20];
  color_buffer_data = new GLfloat[20];
  this->mtx = mtx;
  this->x = x;
  this->y = y;
  this->width = width;
  this->height = height;
  this->angle = angle;
  this->axis = glm::vec3(0.0f,0.0f,1.0f);

  //First triangle
  vertex_buffer_data[0] = width/2;
  vertex_buffer_data[1] = height/2;
  vertex_buffer_data[2] = 0;

  vertex_buffer_data[3] = width/2;
  vertex_buffer_data[4] = -1*height/2;
  vertex_buffer_data[5] = 0;

  vertex_buffer_data[6] = -1*width/2;
  vertex_buffer_data[7] = -1*height/2;
  vertex_buffer_data[8] = 0;

  //Second Triangle
  vertex_buffer_data[9] = width/2;
  vertex_buffer_data[10] = height/2;
  vertex_buffer_data[11] = 0;

  vertex_buffer_data[12] = -1*width/2;
  vertex_buffer_data[13] = height/2;
  vertex_buffer_data[14] = 0;

  vertex_buffer_data[15] = -1*width/2;
  vertex_buffer_data[16] = -1*height/2;
  vertex_buffer_data[17] = 0;

  //Coloring
  for(int j=0;j<18;j+=3){
    color_buffer_data[j + 0] = color[0];
    color_buffer_data[j + 1] = color[1];
    color_buffer_data[j + 2] = color[2];
  }
  vaobj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

Rectangle::Rectangle(const Rectangle& rect){
  vertex_buffer_data = new GLfloat[20];
  color_buffer_data = new GLfloat[20];
  this->mtx = rect.mtx;
  this->x = rect.x;
  this->y = rect.y;
  this->width = rect.width;
  this->height = rect.height;
  this->angle = rect.angle;
  this->axis = rect.axis;
  for(int i = 0; i<18; i++){
    vertex_buffer_data[i] = rect.vertex_buffer_data[i];
    color_buffer_data[i] = rect.vertex_buffer_data[i];
  }
  vaobj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* Rectangle::getVAO(){
  return this->vaobj;
}

int Rectangle::getTopLeftX(){
  return x;
}

int Rectangle::getTopLeftY(){
  return y;
}

int Rectangle::getWidth(){
  return width;
}

int Rectangle::getHeight(){
  return height;
}

float Rectangle::getAngle(){
  return angle;
}

float Rectangle::getPosAngle(){
  return angle + 90.0f;
}

glm::vec3 Rectangle::getAxis(){
  return axis;
}

void Rectangle::setTopLeftX(int x){
  this->x = x;
}

void Rectangle::setTopLeftY(int y){
  this->y = y;
}

void Rectangle::setWidth(int w){
  this->width = w;
}

void Rectangle::setHeight(int h){
  this->height = h;
}

void Rectangle::setAngle(float angle){
  this->angle = angle;
}

void Rectangle::setAxis(glm::vec3 &axis){
  this->axis = axis;
}

void Rectangle::draw(){
  glm::mat4 MVP; // MVP = Projection * View * Model
  // Load identity to model matrix
  mtx->model = glm::mat4(1.0f);

  /* Render your scene */
  glm::mat4 mrotate = glm::rotate((float)(angle*M_PI/180.0f), axis);  // rotate about vector axis
  glm::mat4 mtranslate = glm::translate (glm::vec3((float)x, (float)y, 0.0f)); // glTranslatef
  glm::mat4 mtransform = mtranslate * mrotate;
  mtx->model *= mtransform;
  MVP =  mtx->projection * mtx->view * mtx->model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(mtx->MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(vaobj);
}

Rectangle::~Rectangle(){
  delete[] vertex_buffer_data;
  delete[] color_buffer_data;
}

Cannon::Cannon(GLMatrices *mtx, int x, int y){
  //Circle(GLMatrices *mtx, int cx=0, int cy=0, int radius=3, 
  //int numPolygons =100, float color=0.5);
  //Rectangle(GLMatrices *mtx, int x, int y, 
  //int width, int height, int angle, int color)
  float *colorTank = new float[3];
  colorTank[0] = 0;
  colorTank[1] = 1;
  colorTank[2] = 0;
  tank = new Circle(mtx,colorTank,(float)x,(float)y,4.0f,100);

  float *colorBarrel = new float[3];
  colorBarrel[0] = 0;
  colorBarrel[1] = 0;
  colorBarrel[2] = 1;
  barrel = new Rectangle(mtx,colorBarrel,x, y, 4, 16, -70);
  glm::vec3 mtemp = glm::vec3(0, 0, 1);
  barrel->setAxis(mtemp);
  float radAngle = barrel->getPosAngle() * M_PI/180.0f;
  float cx = tank->getCenterX() + (barrel->getHeight()/2)*cosf(radAngle);
  float cy = tank->getCenterY() + (barrel->getHeight()/2)*sinf(radAngle);
  bombInitSpeed = 80.0f;
  float ux = bombInitSpeed*cosf(radAngle);
  float uy = bombInitSpeed*sinf(radAngle);  
  this->ammo = new Bomb(mtx, cx, cy, ux, uy); 
  movableList.push_back(this->ammo);
  delete colorTank;
  delete colorBarrel;
}

Cannon::~Cannon(){
  delete tank;
  delete barrel;
}

void Cannon::draw(){
  if(ammo){
    ammo->draw();
  }
  barrel->draw();
  tank->draw();
}

void Cannon::barrelUp(){
  int currentAngle = barrel->getAngle();
  currentAngle += 2;
  if(currentAngle >=0 )currentAngle = 0;
  barrel->setAngle(currentAngle);
}

void Cannon::barrelDown(){
  int currentAngle = barrel->getAngle();
  currentAngle -= 2;
  if(currentAngle <= -90 )currentAngle = -90;
  barrel->setAngle(currentAngle);
}

void Cannon::shoot(){
  if(!this->ammo->getDynamic()){
    //cout<<"Tank centre - (x,) = "<<tank->getCenterX()<<" , "<<tank->getCenterY()<<endl;
    //cout<<"Angle - "<<barrel->getPosAngle()<<endl;
    float radAngle = barrel->getPosAngle() * M_PI/180.0f;
    float xAdd = (barrel->getHeight()/2)*cosf(radAngle);
    float yAdd = (barrel->getHeight()/2)*sinf(radAngle);
    //cout<<"X addition - "<<xAdd<<endl;
    //cout<<"Y addition - "<<yAdd<<endl;
    float cx = tank->getCenterX() + xAdd;
    float cy = tank->getCenterY() + yAdd;
    //cout<<"Final bomb position X - "<<cx<<endl;
    //cout<<"Final bomb position Y - "<<cy<<endl;  
    float ux = bombInitSpeed*cosf(radAngle);
    float uy = bombInitSpeed*sinf(radAngle); 
    //cout<<"Bomb speed X "<<ux<<endl; 
    //cout<<"Bomb speed Y "<<uy<<endl; 
    this->ammo->setPosition(cx, cy);
    this->ammo->setSpeed(ux, uy);
    this->ammo->setTime(0.0f);
    this->ammo->setDynamic(true);
  }
  
}

void Cannon::applyForces(float timeInstance){
  if(ammo){
    ammo->applyForces(timeInstance);
  }    
}

Bomb::Bomb(GLMatrices *mtx, float cx, float cy, float speedX, float speedY)
  : Item(5.0f, cx, cy, speedX, speedY, 2.0f)
{
  float *colorCirc = new float[3];
  colorCirc[0] = 1;
  colorCirc[1] = 0;
  colorCirc[2] = 0;
  this->circ = new Circle(mtx, colorCirc, cx, cy, radius, 50);
  this->dynamic = false;
  delete[] colorCirc;
}


void Bomb::draw(){
  circ->draw();
}

void Bomb::applyForces(float timeInstance){
  if(dynamic){
    Item::applyForces(timeInstance);
    /*
    time += timeInstance;
    //cout<<"SpeedX = "<<speedX<<" SpeedY "<<speedY<<endl;
    //cout<<"Before (x,y) -> "<<circ->getCenterX()<<" , "<<circ->getCenterY()<<endl;
    float x = circ->getCenterX() + speedX * time;
    float y = circ->getCenterY() + speedY * time - (0.5f * GRAVITY * time * time);
    //cout<<"After (x,y) -> "<<x<<" , "<<y<<endl;
    setPosition(x,y);
    */
    float tx = getPositionX();
    float ty = getPositionY();
    circ->setCenter(tx,ty);
    float yrp = y - this->circ->getRadius();
    float xrp = x - this->circ->getRadius();

    if(xrp > RIGHT_BOUND || xrp < LEFT_BOUND || this->checkStoppage())
    {
      this->setDynamic(false);
    }
    
  }
}

void Bomb::setDynamic(bool value){
  this->dynamic = value;
}

bool Bomb::getDynamic(){
  return this->dynamic;
}

Item::Item(float mass, float x, float y, float ux, float uy, float radius){
  this->mass = mass;
  this->prevX = x;
  this->prevY = y;
  this->x = x;
  this->y = y;
  this->ux = ux;
  this->uy = uy;
  this->radius = radius;
  this->forceX = 0.0f;
  this->forceY = 0.0f;
  this->ax = 0.0f;
  this->ay = 0.0f;
  this->time = 0.0f;
}


void Item::applyForces(float timeInstance){
  this->forceX = 0.0f;
  this->forceY = 0.0f;
  applyGravity();
  //applyNormalForce();
  applyOtherForces();
  handleCollisionsItem();
  applyFriction();
  applyCollisionGround();
  applyAcceleration();
  applyPosition(timeInstance);
}
void Item::applyGravity(){
  this->forceY -= this->mass * GRAVITY;
}

bool Item::checkCollisionGround(){
  return (y - BOTTOM_BOUND <= radius);
}

void Item::applyOtherForces(){
  //To be implemented by other inherited classes
}
void Item::applyNormalForce(){
  if(checkCollisionGround()){
    this->forceY += this->mass * GRAVITY;
  }
}

void Item::applyFriction(){
  if(checkCollisionGround()){
    float direction;
    if(ux > 0){
      direction = -1.0f;
    }
    else direction = 1.0f;
    this->forceX += direction * this->mass * GRAVITY  * FRICTION_COF;
  }
}

void Item::applyCollisionGround(){
  if(checkCollisionGround()){
    //cout<<"Speed on collision uy = "<<uy<<endl;
    if(uy < -20.5f){
      uy = -1.0f * uy * BOUNCE_COF;
      //cout<<"Speed y here = "<<uy<<endl;
      y += radius + 0.01f; 
      //cout<<"y here = "<<y<<endl;
    }
    else{
      //cout<<"BAD LUCK"<<endl;
      uy = 0.0f;
      y = BOTTOM_BOUND + radius;
    }

  }
}

void Item::setTime(float time){
  this->time = time;
}

void Item::applyAcceleration(){
  this->ax = this->forceX / this->mass;
  this->ay = this->forceY / this->mass;
}

void Item::applyPosition(float timeInstance){
  time += timeInstance;
  prevX = x;
  prevY = y;
  x = x + ux * timeInstance + (0.5 * ax * timeInstance * timeInstance);
  y = y + uy * timeInstance + (0.5 * ay * timeInstance * timeInstance);
  ux = ux + ax * timeInstance;
  uy = uy + ay * timeInstance;
  //cout<<"Speed X -> "<<ux<<" Speed Y -> "<<uy<<endl;
  //cout<<"applyPosition speed Y -> "<<uy;
  //cout<<" applyPosition Y -> "<<y<<endl;
}

float Item::getPositionX(){
  return x;
}

float Item::getPositionY(){
  return y;
}

bool Item::checkStoppage(){
  float tx = x - prevX;
  float ty = y - prevY;
  if(tx < 0.0f)tx *= -1.0f;
  if(ty < 0.0f)ty *= -1.0f;
  if(tx < 0.1f && ty < 0.1f)
    return true;
  else return false;
}

void Item::setPosition(float x, float y){
  this->x = x;
  this->y = y;
}

void Item::setSpeed(float ux, float uy){
  this->ux = ux;
  this->uy = uy;
}

Block::Block(GLMatrices *mtx, int x, int y, int width, int height){
  float *colorBlock = new float[3];
  colorBlock[0] = 0.6;
  colorBlock[1] = 0.298;
  colorBlock[2] = 0.0f;
  rect = new Rectangle(mtx,colorBlock,x, y, width, height, 0);
}

void Block::draw(){
  rect->draw();
}

float Block::getPositionY(){
  return rect->getTopLeftY();
}
float Block::getPositionX(){
  return rect->getTopLeftX();
}
float Block::getHeight(){
  return rect->getHeight();
}
float Block::getWidth(){
  return rect->getWidth();
}

Target::Target(GLMatrices *mtx, Block* pillar)
  :Item(3.0f, pillar->getPositionX(), pillar->getPositionY() + pillar->getHeight()/2.0f + 2.5f, 0.0f, 0.0f, 2.5f)
{
  float *colorTarget = new float[3];
  colorTarget[0] = 0.4f;
  colorTarget[1] = 0.0f;
  colorTarget[2] = 0.4f;
  float ty = pillar->getPositionY() + pillar->getHeight()/2.0f + radius;
  circ = new Circle(mtx, colorTarget, getPositionX(), ty, radius, 100);
  this->pillar = pillar;
}

void Target::draw(){
  circ->draw();
}

void Target::applyForces(float timeInstance){
  Item::applyForces(timeInstance);
  float tx = getPositionX();
  float ty = getPositionY();
  circ->setCenter(tx,ty);
}

void Target::applyOtherForces(){
  float lb = pillar->getPositionX() - pillar->getWidth()/2.0f;
  float rb = pillar->getPositionX() + pillar->getWidth()/2.0f;
  float ub = pillar->getPositionY() + pillar->getHeight()/2.0f + circ->getRadius() + 1.0f;
  if(this->x >= lb && this->x <=rb && this->y <= ub)
    this->forceY += this->mass * GRAVITY;
}

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

bool checkCollisionItem(Item &first, Item &second){
  float x12 = first.x - second.x;
  float y12 = first.y - second.y;
  float dist = x12*x12 + y12*y12;
  float r12 = first.radius + second.radius;
  r12 = r12 * r12;
  if(dist <= r12)
    {
      cout<<"*******COLLISION HAPPENED***********"<<endl;
      return true;
    }
  else
    {
      //cout<<"####### NO COLLISION ####"<<endl;
      return false;
    }

}

void simulateCollisionItem(Item &first, Item &second){

  //Vector pointing in direction of collision
  float cx = first.x - second.x;
  float cy = first.y - second.y;

  //Distance of the above vector
  float distance = (float)sqrt(cx*cx + cy*cy);

  //Unit vector in direction of collision
  float unitX, unitY;
  if(distance == 0.0f){
    unitX = 1.0f;
    unitY = 0.0f;
  }
  else{
    unitX = cx/distance;
    unitY = cy/distance;
  }

  // Component of velocity of Item first,second in 
  // in direction of collision
  float firstInitComp = first.ux * unitX + first.uy * unitY;
  float secondInitComp = second.ux * unitX + second.uy * unitY;

  float firstFinalComp = (firstInitComp*(first.mass - second.mass) + 2*second.mass*secondInitComp)/(first.mass + second.mass);
  float secondFinalComp = (secondInitComp*(second.mass - first.mass) + 2*first.mass*firstInitComp)/(first.mass + second.mass);

  float firstChange = firstFinalComp - firstInitComp;
  float secondChange = secondFinalComp - secondInitComp;

  first.ux += firstChange * unitX;
  first.uy += firstChange * unitY;

  second.ux += secondChange * unitX;
  second.uy += secondChange * unitY;
}

void handleCollisionsItem(){
  //cout<<"Movable list size "<<movableList.size()<<endl;
  for(int i = 0; i < movableList.size(); i++){
    for(int j = i + 1; j < movableList.size(); j++){
      if(checkCollisionItem(*movableList[i], *movableList[j]))
        simulateCollisionItem(*movableList[i], *movableList[j]);
    }
  }

}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_A:
                can->barrelUp();
                break;
            case GLFW_KEY_B:
                can->barrelDown();
                break;
            case GLFW_KEY_SPACE:
                can->shoot();
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(LEFT_BOUND, RIGHT_BOUND, BOTTOM_BOUND, TOP_BOUND, 0.1f, 500.0f);
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  //r->draw();
  //c->draw();
  can->draw();
  b1->draw();
  t1->draw();

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */

void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  //c = new Circle(&Matrices);
  //r = new Rectangle(&Matrices);
  can = new Cannon(&Matrices);
  b1 = new Block(&Matrices, -2, BOTTOM_BOUND + 6, 5, 12);
  t1 = new Target(&Matrices, b1);
  movableList.push_back(t1);
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1300;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.01) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
            can->applyForces(0.01f);
            t1->applyForces(0.01f);
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
