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

#include <FTGL/ftgl.h>
#include <SOIL/SOIL.h>

using namespace std;

float LEFT_BOUND = -72.0f;
float RIGHT_BOUND = 72.0f;
float TOP_BOUND = 34.0f;
float BOTTOM_BOUND = -34.0f;
float ZOOM_FACTOR = 2.0f;
float WINDOW_WIDTH = 1300;
float WINDOW_HEIGHT = 600;




struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
};
typedef struct GLMatrices GLMatrices;

GLMatrices Matrices;
GLuint programID, fontProgramID, textureProgramID;
GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;

class FTGLFont{
public:
	FTGLFont(GLMatrices *mtx, float* color, char* fontfile, char *word, float size, float x, float y, float scaleFactor);
	~FTGLFont();
	void draw();
	void setWord(char* word);
	void setScaleFactor(float scaleFactor);
	float getScaleFactor();
private:
	GLMatrices *mtx;
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
	float scaleFactor;
	float x;
	float y;
  	glm::vec3 fontColor; 
  	char* fontfile;
  	char* word;
};

void handleCollisionsBlock();

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

class Image{
public:
	Image(GLMatrices *mtx, GLuint textureID, float x, float y, float width, float height, float angle);
	void draw();
private:
  GLfloat *vertex_buffer_data;
  GLfloat *texture_buffer_data;
  GLuint textureID;
  GLMatrices *mtx;
  VAO *vaobj;
  float x;
  float y;
  float width;
  float height;
  float angle;
  glm::vec3 axis;
};

class Rectangle{
public:
  Rectangle(GLMatrices *mtx, float* color,float x = 0.0f, float y = 0.0f, float width = 2.0f, float height = 3.0f, float angle = 0.0f);
  Rectangle(const Rectangle& rect);
  ~Rectangle();
  VAO* getVAO();
  float getTopLeftX();
  float getTopLeftY();
  float getWidth();
  float getHeight();
  float getAngle();
  float getPosAngle();
  glm::vec3 getAxis();
  void setTopLeftX(float x);
  void setTopLeftY(float y);
  void setWidth(float w);
  void setHeight(float h);
  void setAngle(float angle);
  void setAxis(glm::vec3 &axis);
  virtual void draw();
private:
  GLfloat *vertex_buffer_data;
  GLfloat *color_buffer_data;
  GLMatrices *mtx;
  VAO *vaobj;
  float x;
  float y;
  float width;
  float height;
  float angle;
  glm::vec3 axis;
};
class Block;

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
  friend bool checkCollisionItem(Item &first, Item &second, bool& flag);
  friend void simulateCollisionItem(Item &first, Item &second);
  friend void handleCollisionsItem();
  
  friend bool checkCollisionBlock(Item& ball, Block& obs);
  friend void simulateCollisionBlock(Item& ball, Block &obs);
  friend void handleCollisionsBlock();

  friend bool checkCollisionWall(Item& ball);
  friend void simulateCollisionWall(Item& ball);
  friend void handleCollisionsWall(Item& ball);
protected:
  static const float GRAVITY = 75.0f;
  static const float BOUNCE_COF = 0.4f;
  static const float FRICTION_COF = 0.1f;
  static const float OBS_BOUNCE_COF = 0.6f;
  static const float WALL_BOUNCE_COF = 0.2f;
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
  bool collisionFlag;
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
  Cannon(GLMatrices *mtx, int x = LEFT_BOUND + 4, int y = BOTTOM_BOUND + 3);
  ~Cannon();

  void barrelUp();
  void barrelDown();
  void setBarrelAngle(float angle);
  void shoot();
  void draw();
  void increaseSpeed();
  void decreaseSpeed();
  void applyForces(float timeInstance);
  void setBombInitSpeed(float speed);
  float getBombInitSpeed();
  int getShotsLeft();
private:
  Circle *tank;
  Rectangle *barrel;
  GLMatrices *mtx;
  Bomb *ammo;
  float bombInitSpeed;
  int shotsLeft;
  bool ammoVisible;
};
//Rectangle(GLMatrices *mtx, float* color,int x = 0, 
//int y = 0, int width = 2, int height = 3, int angle = 0);
class Block
{
public:
  Block(GLMatrices *mtx, int x, int y, int width, int height, bool dynamic = false);
  float getPositionY();
  float getPositionX();
  float getHeight();
  float getWidth();
  void draw();
  void applyForces(float timeInstance);
  bool isDynamic();
  float getSpeed();
  float getLeftBound();
  float getRightBound();
  friend bool checkCollisionBlock(Item& ball, Block& obs);
  friend void simulateCollisionBlock(Item& ball, Block &obs);
  friend void handleCollisionsBlock();
private:
  Rectangle *rect;
  float leftBound;
  float rightBound;
  float speed;
  float time;
  bool dynamic;
};

class Target:public Item
{
public:
  Target(GLMatrices *mtx, Block* pillar);
  void draw();
  void applyOtherForces();
  void applyForces(float timeInstance);
  bool isInContact();
private:
  Circle *circ;
  Block *pillar;
};

bool gameSplash;
bool gameWin;
bool gameLoose;
int gameScore;

Image *background;
Circle *c;
Rectangle *r;
Cannon *can;
Block *b1;
Target *t1;
Block *b2;
Target *t2;
Block *b3;
Target *t3;
Block *b4;
Target *t4;
//Block *b5;
//Target *t5;
FTGLFont *f1;
FTGLFont *f2;
FTGLFont *f3;
FTGLFont *fLoose;
FTGLFont *fWin;
FTGLFont *fScore;
FTGLFont *fEnter;
FTGLFont *fIns1;
FTGLFont *fIns2;
FTGLFont *fIns3;
FTGLFont *fIns4;
std::vector<Item*> movableList;
std::vector<Block*> obstacleList;
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

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

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

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  2,                  // attribute 2. Textures
						  2,                  // size (s,t)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}

void draw3DTexturedObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
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

Image::Image(GLMatrices *mtx, GLuint textureID, float x, float y, float width, float height, float angle)
{
  vertex_buffer_data = new GLfloat[20];
  texture_buffer_data = new GLfloat[20];
  this->textureID = textureID;
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

  //Texture data
  texture_buffer_data[0] = 0;
  texture_buffer_data[1] = 1;
  texture_buffer_data[2] = 1;
  texture_buffer_data[3] = 1;
  texture_buffer_data[4] = 1;
  texture_buffer_data[5] = 0;
  texture_buffer_data[6] = 0;
  texture_buffer_data[7] = 1;
  texture_buffer_data[8] = 0;
  texture_buffer_data[9] = 0;
  texture_buffer_data[10] = 1;
  texture_buffer_data[11] = 0;


  vaobj = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}

void Image::draw(){
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
  glUniformMatrix4fv(mtx->TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(vaobj);

}

Rectangle::Rectangle(GLMatrices *mtx, float* color,float x, float y, float width, float height, float angle)
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
  vertex_buffer_data[0] = width/2;    //1
  vertex_buffer_data[1] = height/2;
  vertex_buffer_data[2] = 0;		

  vertex_buffer_data[3] = width/2;	   //2
  vertex_buffer_data[4] = -1*height/2;
  vertex_buffer_data[5] = 0;

  vertex_buffer_data[6] = -1*width/2;  //3
  vertex_buffer_data[7] = -1*height/2;
  vertex_buffer_data[8] = 0;

  //Second Triangle
  vertex_buffer_data[9] = width/2;	  //1
  vertex_buffer_data[10] = height/2;
  vertex_buffer_data[11] = 0;

  vertex_buffer_data[12] = -1*width/2;	//4
  vertex_buffer_data[13] = height/2;
  vertex_buffer_data[14] = 0;

  vertex_buffer_data[15] = -1*width/2;	//3
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

float Rectangle::getTopLeftX(){
  return x;
}

float Rectangle::getTopLeftY(){
  return y;
}

float Rectangle::getWidth(){
  return width;
}

float Rectangle::getHeight(){
  return height;
}

float Rectangle::getAngle(){
  return angle;
}

//Sets positive angle in degrees
float Rectangle::getPosAngle(){
  return angle + 90.0f;
}

glm::vec3 Rectangle::getAxis(){
  return axis;
}

void Rectangle::setTopLeftX(float x){
  this->x = x;
}

void Rectangle::setTopLeftY(float y){
  this->y = y;
}

void Rectangle::setWidth(float w){
  this->width = w;
}

void Rectangle::setHeight(float h){
  this->height = h;
}

//angle in degrees
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
  this->shotsLeft = 10;
  float *colorTank = new float[3];
  colorTank[0] = 1;
  colorTank[1] = 0.412;
  colorTank[2] = 0.270;
  tank = new Circle(mtx,colorTank,(float)x,(float)y,4.0f,100);

  float *colorBarrel = new float[3];
  colorBarrel[0] = 0.098;
  colorBarrel[1] = 0.098;
  colorBarrel[2] = 0.439;
  barrel = new Rectangle(mtx,colorBarrel,x, y, 4.0f, 16.0f, -70.0f);
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
  ammoVisible = false;
  delete colorTank;
  delete colorBarrel;
}

Cannon::~Cannon(){
  delete tank;
  delete barrel;
}

void Cannon::setBombInitSpeed(float speed){
	this->bombInitSpeed = speed;
}
void Cannon::draw(){
  if(ammoVisible){
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

float Cannon::getBombInitSpeed(){
	return bombInitSpeed;
}

void Cannon::setBarrelAngle(float angle){
	float currentAngle = angle;
	if(currentAngle >=0 )currentAngle = 0;
	else if(currentAngle <= -90 )currentAngle = -90;
	barrel->setAngle(angle);
}

void Cannon::increaseSpeed(){
  bombInitSpeed += 10.0f;
  cout<<"Bomb speed increased ->"<<bombInitSpeed<<endl;
}

void Cannon::decreaseSpeed(){
  bombInitSpeed -= 10.0f;
  cout<<"Bomb speed decreased ->"<<bombInitSpeed<<endl;
}

int Cannon::getShotsLeft(){
	return shotsLeft;

}


void Cannon::barrelDown(){
  int currentAngle = barrel->getAngle();
  currentAngle -= 2;
  if(currentAngle <= -90 )currentAngle = -90;
  barrel->setAngle(currentAngle);
}

void Cannon::shoot(){
  ammoVisible = true;
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
    cout<<"Bomb fired with speed ->"<<bombInitSpeed<<endl;
    float ux = bombInitSpeed*cosf(radAngle);
    float uy = bombInitSpeed*sinf(radAngle); 
    //cout<<"Bomb speed X "<<ux<<endl; 
    //cout<<"Bomb speed Y "<<uy<<endl; 
    this->ammo->setPosition(cx, cy);
    this->ammo->setSpeed(ux, uy);
    this->ammo->setTime(0.0f);
    this->ammo->setDynamic(true);
    shotsLeft--;
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
  colorCirc[0] = 0.545;
  colorCirc[1] = 0;
  colorCirc[2] = 0;
  this->circ = new Circle(mtx, colorCirc, cx, cy, radius, 50);
  this->dynamic = false;
  this->collisionFlag = true;
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
  if(tx < 0.01f && ty < 0.01f)
    {
      cout<<"returned TRUE"<<endl;
      return true;
    }
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

Block::Block(GLMatrices *mtx, int x, int y, int width, int height, bool dynamic){
  float *colorBlock = new float[3];
  colorBlock[0] = 0.6;
  colorBlock[1] = 0.298;
  colorBlock[2] = 0.0f;
  rect = new Rectangle(mtx,colorBlock,x, y, width, height, 0);
  this->dynamic = dynamic;
  this->time = 0.0f;
  if(dynamic){
  	//cout<<" Is dynamic "<<endl;
  	this->leftBound = x - 10.0f;
  	this->rightBound = x + 10.0f;
  	this->speed = 5.0f;
  	//cout<<"left bound -> "<<leftBound<<" Right Bound -> "<<rightBound<<endl;
  }
  else{
  	this->leftBound = x;
  	this->rightBound = x;
  	this->speed = 0.0f;
  }
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

void Block::applyForces(float timeInstance){
	float tx;
	if(dynamic){
		time += timeInstance;
		tx = getPositionX();
		tx += speed* timeInstance;
		rect->setTopLeftX(tx);
		if(tx < leftBound || tx > rightBound){
			speed *= -1.0f;
		}
		//cout<<"Force applied, tx = "<<tx<<endl;
	}
}

bool Block::isDynamic(){
	return dynamic;
}

float Block::getSpeed(){
	return speed;
}

float Block::getLeftBound(){
	return leftBound;
}

float Block::getRightBound(){
	return rightBound;
}

Target::Target(GLMatrices *mtx, Block* pillar)
  :Item(3.0f, pillar->getPositionX(), pillar->getPositionY() + pillar->getHeight()/2.0f + 2.5f, pillar->getSpeed(), 0.0f, 2.5f)
{
  float *colorTarget = new float[3];
  colorTarget[0] = 0.4f;
  colorTarget[1] = 0.0f;
  colorTarget[2] = 0.4f;
  float ty = pillar->getPositionY() + pillar->getHeight()/2.0f + radius;
  circ = new Circle(mtx, colorTarget, getPositionX(), ty, radius, 100);
  this->pillar = pillar;
  this->collisionFlag = false;
}

void Target::draw(){
  circ->draw();
}

void Target::applyForces(float timeInstance){
  Item::applyForces(timeInstance);
  float tx = getPositionX();
  float ty = getPositionY();
  if(pillar->isDynamic() && isInContact()){
  	if(tx < pillar->getLeftBound() || tx > pillar->getRightBound()){
  			this->ux *= -1.0f;
  		}
  	}
  circ->setCenter(tx,ty);
}

bool Target::isInContact(){
  float lb = pillar->getPositionX() - pillar->getWidth()/2.0f;
  float rb = pillar->getPositionX() + pillar->getWidth()/2.0f;
  float ub = pillar->getPositionY() + pillar->getHeight()/2.0f + circ->getRadius() + 1.0f;
  if(this->x >= lb && this->x <=rb && this->y <= ub)
  	return true;
  else return false;
}

void Target::applyOtherForces(){
  if(isInContact()){
    	this->forceY += this->mass * GRAVITY;
	}
}

FTGLFont::FTGLFont(GLMatrices *mtx, float* color, char* fontfile, char* word,float size, float x, float y, float scaleFactor)
{
	cout<<"Entered ftgl"<<endl;
	this->mtx = mtx;
	cout<<"mtx made"<<endl;
	fontColor = glm::vec3(color[0], color[1], color[2]);
	cout<<"vec color made"<<endl;
	this->fontfile = new char[20];
	strcpy(this->fontfile, fontfile);
	this->word = new char[100];
	strcpy(this->word, word);
	cout<<"str copied"<<endl;
	this->x = x;
	this->y = y;
	this->scaleFactor = scaleFactor;
	
	cout<<" Above this->font"<<endl;
	this->font = new FTExtrudeFont(fontfile); // 3D extrude style rendering
	if(this->font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	
	this->fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	this->fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	this->font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	this->font->FaceSize(size);
	this->font->Depth(0);
	this->font->Outset(0, 0);
	this->font->CharMap(ft_encoding_unicode);
}

void FTGLFont::setScaleFactor(float scaleFactor){
	this->scaleFactor = scaleFactor;
}

float FTGLFont::getScaleFactor(){
	return scaleFactor;
}

void FTGLFont::draw(){
  glm::mat4 MVP;
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(x,y,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(scaleFactor,scaleFactor,scaleFactor));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(this->fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(this->fontColorID, 1, &fontColor[0]); 

	// Render font
	this->font->Render(word);
}

void FTGLFont::setWord(char* word){
	strcpy(this->word, word);
}

float camera_position = 0.0f;

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

void panCameraLeft(){
  camera_position += 0.5f;
}

void panCameraRight(){
  camera_position -= 0.5f;
}

void zoomIn(){
  LEFT_BOUND += ZOOM_FACTOR * 2.0f;
  RIGHT_BOUND -= ZOOM_FACTOR * 2.0f;
  TOP_BOUND -= ZOOM_FACTOR;
  BOTTOM_BOUND += ZOOM_FACTOR;
  Matrices.projection = glm::ortho(LEFT_BOUND, RIGHT_BOUND, BOTTOM_BOUND, TOP_BOUND, 0.1f, 500.0f);
}

void zoomOut(){
  LEFT_BOUND -= ZOOM_FACTOR * 2.0f;
  RIGHT_BOUND += ZOOM_FACTOR * 2.0f;
  TOP_BOUND += ZOOM_FACTOR;
  BOTTOM_BOUND -= ZOOM_FACTOR;
  Matrices.projection = glm::ortho(LEFT_BOUND, RIGHT_BOUND, BOTTOM_BOUND, TOP_BOUND, 0.1f, 500.0f);
}


bool checkCollisionItem(Item &first, Item &second, bool &flag){
  float x12 = first.x - second.x;
  float y12 = first.y - second.y;
  float dist = x12*x12 + y12*y12 - 5.0f;
  float r12 = first.radius + second.radius;
  r12 = r12 * r12;
  if(dist <= r12 && flag)
    {
      //cout<<"*******COLLISION HAPPENED***********"<<endl;
      flag = false;
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

  float magFirstChangeX = (firstChange * unitX) > 0.0f ? (firstChange * unitX) : (-1.0f * firstChange * unitX);
  float magFirstChangeY = (firstChange * unitY) > 0.0f ? (firstChange * unitY) : (-1.0f * firstChange * unitY);
  
  float magSecondChangeX = (secondChange * unitX) > 0.0f ? (secondChange * unitX) : (-1.0f * secondChange * unitX);
  float magSecondChangeY = (secondChange * unitY) > 0.0f ? (secondChange * unitY) : (-1.0f * secondChange * unitY);

  if(true){
  	first.x += (firstChange * unitX)/magFirstChangeX * 1.0f;
  	first.y += (firstChange * unitY)/magSecondChangeX * 1.0f;
  	second.x += (secondChange * unitX)/magSecondChangeX * 1.0f;
  	second.y += (secondChange * unitY)/magSecondChangeY * 1.0f;
  }

  //cout<<" Change First Ux = "<< firstChange * unitX<<endl;
  //cout<<" Change First Uy = "<< firstChange * unitY<<endl;

  //cout<<" Change Second Ux = "<< secondChange * unitX<<endl;
  //cout<<" Change Second Uy = "<< secondChange * unitY<<endl;

}

void handleCollisionsItem(){
  //cout<<"Movable list size "<<movableList.size()<<endl;
  bool flag = true;
  for(int i = 0; i < movableList.size(); i++){
    for(int j = i + 1; j < movableList.size(); j++){
      flag = true;
      if(checkCollisionItem(*movableList[i], *movableList[j], flag)){
      	if(movableList[i]->collisionFlag == false){
      		movableList[i]->collisionFlag = true;
      		gameScore++;
      	}
      	if(movableList[j]->collisionFlag == false){
      		movableList[j]->collisionFlag = true;
      		gameScore++;
      	}
      	simulateCollisionItem(*movableList[i], *movableList[j]);
      }
    }
  }

}

bool checkCollisionBlock(Item& ball, Block& obs)
{
  float obsLeftBound = obs.rect->getTopLeftX() - obs.rect->getWidth()/2.0f - ball.radius;
  float obsRightBound = obs.rect->getTopLeftX() + obs.rect->getWidth()/2.0f + ball.radius;
  float obsTopBound = obs.rect->getTopLeftY() + obs.rect->getHeight()/2.0f + ball.radius;
  float obsBottomBound = obs.rect->getTopLeftY() - obs.rect->getHeight()/2.0f - ball.radius;
  if(ball.x > obsLeftBound && ball.x < obsRightBound && ball.y < obsTopBound && ball.y > obsBottomBound){
    return true;
  }
  else return false;
}

void simulateCollisionBlock(Item& ball, Block &obs){
  float obsLeftBound = obs.rect->getTopLeftX() - obs.rect->getWidth()/2.0f - ball.radius;
  float obsRightBound = obs.rect->getTopLeftX() + obs.rect->getWidth()/2.0f + ball.radius;
  float obsTopBound = obs.rect->getTopLeftY() + obs.rect->getHeight()/2.0f + ball.radius;
  float obsBottomBound = obs.rect->getTopLeftY() - obs.rect->getHeight()/2.0f - ball.radius;
  float obsTop = obs.rect->getTopLeftY() + obs.rect->getHeight()/2.0f;
  float obsBottom = obs.rect->getTopLeftY() - obs.rect->getHeight()/2.0f;
  if(ball.y < obsTop && ball.y > obsBottom){
    ball.ux = -1.0f * Item::OBS_BOUNCE_COF * ball.ux;
    if(ball.x < obs.rect->getTopLeftX())
      ball.x = obsLeftBound - 1.0f;
    else
      ball.x = obsRightBound + 1.0f;

  }
  else {
  	ball.uy = -1.0f * Item::OBS_BOUNCE_COF * ball.uy;
  	if(ball.y > obs.rect->getTopLeftY())
  		ball.y = obsTopBound + 1.0f;
  	else
  		ball.y = obsBottomBound - 1.0f;
  }
}

void handleCollisionsBlock(){
  for(int i = 0; i < movableList.size(); i++){
    for(int j = 0; j < obstacleList.size(); j++){
      if(checkCollisionBlock(*movableList[i], *obstacleList[j]))
        simulateCollisionBlock(*movableList[i], *obstacleList[j]);
    }
  }
}

bool checkCollisionWall(Item& ball){
	float ballRightSide = ball.x + ball.radius;
	float ballLeftSide = ball.x - ball.radius;
	float ballTopSide = ball.y + ball.radius;
	if(ballRightSide > RIGHT_BOUND || ballLeftSide < LEFT_BOUND || ballTopSide > TOP_BOUND){
		return true;
	}
	else return false;
}

void simulateCollisionWall(Item& ball){
	float ballRightSide = ball.x + ball.radius;
	float ballLeftSide = ball.x - ball.radius;
	float ballTopSide = ball.y + ball.radius;
	if(ballRightSide > RIGHT_BOUND || ballLeftSide < LEFT_BOUND){
		ball.ux = -1.0f * Item::WALL_BOUNCE_COF * ball.ux;
		if(ballRightSide > RIGHT_BOUND)
			ball.x -= (ball.radius + 0.01f);
		if(ballLeftSide < LEFT_BOUND)
			ball.x += (ball.radius + 0.01f);
	}
	if(ballTopSide > TOP_BOUND){
		ball.uy = -1.0f * Item::WALL_BOUNCE_COF * ball.uy;
		ball.y -= (ball.radius + 0.01f);
	}
}

void handleCollisionsWall(){
	for(int i = 0; i < movableList.size(); i++){
		if(checkCollisionWall(*movableList[i])){
			simulateCollisionWall(*movableList[i]);
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
            case GLFW_KEY_F:
                can->increaseSpeed();
                break;
            case GLFW_KEY_S:
                can->decreaseSpeed();
                break;
            case GLFW_KEY_LEFT:
                panCameraLeft();
                break;
            case GLFW_KEY_RIGHT:
                panCameraRight();
                break;
            case GLFW_KEY_UP:
                zoomIn();
                break;
            case GLFW_KEY_DOWN:
                zoomOut();
                break;
            case GLFW_KEY_SPACE:
                can->shoot();
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_ENTER:
                gameSplash = true;
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

bool mouseHold = false;
bool panFlag = false;
float prevX = 0.0f, prevY = 0.0f;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	double xpos, ypos;
	double result;
	float window_width = WINDOW_WIDTH;
	float window_height = WINDOW_HEIGHT;
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS)
                {
					glfwGetCursorPos(window, &xpos, &ypos);
					ypos = window_height - ypos;
					cout<<"Cursoe - Xpos -> "<<xpos<<" Ypos-> "<<ypos<<endl;
					result = atan(ypos/xpos) * 180 / M_PI;
					result = -1.0f * (90.0f - result);
					cout<<"Angle is "<<result<<endl;
					can->setBarrelAngle((float)result);
					if(xpos < window_width/4)can->setBombInitSpeed(80.0f);
					else if(xpos >= window_width/4 && xpos < window_width/2)can->setBombInitSpeed(100.0f);
					else if(xpos >= window_width/2 && xpos < window_width*3/4.0f)can->setBombInitSpeed(110.0f);
					else can->setBombInitSpeed(130.0f);
                }
            //else if(action == GLFW_RELEASE)
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
            	/*
            	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
            	glfwGetCursorPos(window, &xpos, &ypos);
            	if((float)xpos > prevX)panCameraRight();
            	else if((float)xpos < prevX)panCameraLeft();
            	prevX = (float)xpos;
            	*/
            	panFlag = true;

            }
            else if(action == GLFW_RELEASE){
            	//glfwSetInputMode(window, GLFW_CURSOR_NORMAL, 1);
            		prevX = 0.0f;
            		panFlag = false;


            	}
            break;
        default:
            break;
        
    }
}

void checkPan(GLFWwindow* window){
	double xpos, ypos;
	if(panFlag){
		glfwGetCursorPos(window, &xpos, &ypos);
		if((float)xpos > prevX)panCameraRight();
		else if((float)xpos < prevX)panCameraLeft();
		prevX = (float)xpos;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset > 0)zoomIn();
	else if(yoffset < 0)zoomOut();
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

  // Render with texture shaders now
  glUseProgram(textureProgramID);
  background->draw();
  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);
  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(camera_position,0,3), glm::vec3(camera_position,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  //static float c = 0;
	//c++;
	//Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(sinf(c*M_PI/180.0),3*cosf(c*M_PI/180.0),0)); // Fixed camera for 2D (ortho) in XY plane
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
  if(gameSplash){
	  	 if(gameLoose == false && gameWin == false){
	  	  can->draw();

		  b1->draw();
		  t1->draw();

		  b2->draw();
		  t2->draw();

		  b3->draw();
		  t3->draw();

		  b4->draw();
		  t4->draw();

		  //b5->draw();
		  //t5->draw();
	  }
	}

	glUseProgram(fontProgramID);

	if(gameSplash){
	  if(gameLoose == true){
	  	fLoose->draw();
	  	
	  }
	  if(gameWin == true){
	  	fWin->draw();
	  }
	  if(gameLoose == false && gameWin == false){
	  	// Render font on screen
		  //static int fontScale = 0;
		  /*float fontScaleValue = 5.0f;
		  glm::vec3 fontColor = glm::vec3(0,0,0);*/

		  glUseProgram(fontProgramID);
		  f1->draw();
		  f2->draw();
		  f3->draw();
		  fScore->draw();

	  }
  }
  static int fontScale = 0;
  float fontScaleValue;

  if(!gameSplash){
  	fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
    fEnter->setScaleFactor(fontScaleValue);
  	f1->draw();
  	fEnter->draw();
  	fIns1->draw();
  	fIns2->draw();
  	fIns3->draw();
  	fIns4->draw();
  	fontScale = (fontScale + 1) % 360;
  }

 
	  

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

    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */

void initGL (GLFWwindow* window, int width, int height)
{

	// Load Textures
	// Enable Texture0 as current texture memory
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	GLuint textureID = createTexture("backpic.png");
	// check for an error during the load process
	if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");

    /* Objects should be created before any other gl function and shaders */
//GLMatrices *mtx, GLuint textureID, float x, float y,
// float width, float height, float angle)

  background = new Image(&Matrices, textureID, 0.0f, 0.0f, LEFT_BOUND * 2.0f, TOP_BOUND * 2.0f, 0.0f);
  can = new Cannon(&Matrices);
  b1 = new Block(&Matrices, -2, BOTTOM_BOUND + 6, 5, 12);
  t1 = new Target(&Matrices, b1);

  b2 = new Block(&Matrices, -2, 6, 5, 12, true);
  t2 = new Target(&Matrices, b2);

  b3 = new Block(&Matrices, 30, 0, 5, 12);
  t3 = new Target(&Matrices, b3);

  b4 = new Block(&Matrices, -24, BOTTOM_BOUND + 6, 5, 12, true);
  t4 = new Target(&Matrices, b4);

  //b5 = new Block(&Matrices, 35, BOTTOM_BOUND + 6, 5, 12);
  //t5 = new Target(&Matrices, b5);

  movableList.push_back(t1);
  obstacleList.push_back(b1);
  movableList.push_back(t2);
  obstacleList.push_back(b2);
  movableList.push_back(t3);
  obstacleList.push_back(b3);
  movableList.push_back(t4);
  obstacleList.push_back(b4);
  //movableList.push_back(t5);
  //obstacleList.push_back(b5);
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
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Initialise FTGL stuff
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );

	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");

//FTGLFont(GLMatrices *mtx, float* color, char* fontfile, float size, float x, float y, float scaleFactor)
	float colArrayFont[3];
	colArrayFont[0] = 0;
	colArrayFont[1] = 0;
	colArrayFont[2] = 0;

	char fileString[20]; 
	strcpy(fileString, "kimberly.ttf");

	char fileString2[20];
	strcpy(fileString2, "fella.ttf");


	char wordName[50];
	strcpy(wordName, "The(_)Ball(_)Machine");

	char wordName2[50];
	strcpy(wordName2, "Speed:");

	char wordName3[50];
	strcpy(wordName3, "Shots:");

	char wordName4[50];
	strcpy(wordName3, "Score:");

	char looseName[50];
	strcpy(looseName, "\tYou Loose !!!\t");

	char winName[50];
	strcpy(winName, "\tYou Win !!!\t");

	char enterName[50];
	strcpy(enterName, "Press enter");

	char ins1Name[50];
	strcpy(ins1Name, "* Try to drop the balls within 10 chances. ");

	char ins2Name[50];
	strcpy(ins2Name, "~ Spacebar to shoot ");

	char ins3Name[50];
	strcpy(ins3Name, "~ A/B to tilt cannon. ");

	char ins4Name[50];
	strcpy(ins4Name, "~ F/S to alter bomb speed. ");


	fLoose = new FTGLFont(&Matrices, colArrayFont, fileString, looseName, 40.0f, -40.0f, 0.0f, 1.0f);
	fWin = new FTGLFont(&Matrices, colArrayFont, fileString, winName, 40.0f, -40.0f, 0.0f, 1.0f);
	f1 = new FTGLFont(&Matrices, colArrayFont, fileString, wordName, 20.0f, -50.0f, TOP_BOUND - 6.0f, 1.0f);
	f2 = new FTGLFont(&Matrices, colArrayFont, fileString, wordName2, 13.0f, LEFT_BOUND + 1.0f, TOP_BOUND - 10.0f, 1.0f);
	f3 = new FTGLFont(&Matrices, colArrayFont, fileString, wordName3, 13.0f, LEFT_BOUND + 1.0f, TOP_BOUND - 15.0f, 1.0f);
	fScore = new FTGLFont(&Matrices, colArrayFont, fileString, wordName4, 13.0f, LEFT_BOUND + 1.0f, TOP_BOUND - 20.0f, 1.0f);
	fEnter = new FTGLFont(&Matrices, colArrayFont, fileString, enterName, 13.0f, LEFT_BOUND + 60.0f, TOP_BOUND - 60.0f, 1.0f);
	fIns1 = new FTGLFont(&Matrices, colArrayFont, fileString2, ins1Name, 13.0f, LEFT_BOUND + 10.0f, TOP_BOUND - 20.0f, 0.7f);
    fIns2 = new FTGLFont(&Matrices, colArrayFont, fileString2, ins2Name, 13.0f, LEFT_BOUND + 10.0f, TOP_BOUND - 30.0f, 0.7f);
    fIns3 = new FTGLFont(&Matrices, colArrayFont, fileString2, ins3Name, 13.0f, LEFT_BOUND + 10.0f, TOP_BOUND - 40.0f, 0.7f);
   	fIns4 = new FTGLFont(&Matrices, colArrayFont, fileString2, ins4Name, 13.0f, LEFT_BOUND + 10.0f, TOP_BOUND - 50.0f, 0.7f);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	gameWin = false; 
	gameLoose = false;
	gameScore = 0;
	gameSplash = false;

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
        int tempB;
		char str[50];
		char strB[50];
		strcpy(strB,"Speed:");

		char strC[50];
		strcpy(strC,"Shots:");

		char strD[50];
		strcpy(strD,"Score:");

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.01) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
            can->applyForces(0.01f);
            b1->applyForces(0.01f);
            b2->applyForces(0.01f);
            b3->applyForces(0.01f);
            b4->applyForces(0.01f);
           	//b5->applyForces(0.01f);
            t1->applyForces(0.01f);
            t2->applyForces(0.01f);
            t3->applyForces(0.01f);
            t4->applyForces(0.01f);
            //t5->applyForces(0.01f);
            handleCollisionsItem();
 			handleCollisionsBlock();
 			handleCollisionsWall();
 			checkPan(window);
 			tempB = (int)can->getBombInitSpeed();
 			sprintf(str, "%d", tempB);
 			strcat(strB,str);
 			f2->setWord(strB);
 			tempB = can->getShotsLeft();
 			if(tempB == 0){
 				gameLoose = true;
 			}
 			sprintf(str, "%d", tempB);
 			strcat(strC,str);
 			f3->setWord(strC);
 			sprintf(str, "%d", gameScore);
 			strcat(strD,str);
 			fScore->setWord(strD);
 			if(gameScore == movableList.size()-1){
 				gameWin = true;
 			}
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
