#include <GL/glut.h>
#include <algorithm>

#include "Map.h"
#include "Game.h"
#include "Agent.h"

int screenWidth = 800;
int screenHeight = 600;


const float divison = 0.7;
std::vector<float> vec;


Map map(10, 10);
Game game;
Agent agent;


void TeachAgent() {
	agent.Reset();
	agent.SetGame(&game);
	int numIterations = 10000;
	while (numIterations > 0) {
		game.NewGame();
		while (!game.Ended()) {
			agent.Step();
		}
		numIterations--;
	}
}


void DrawQuad(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
	glVertex2f(x - width / 2, y - height / 2);
	glVertex2f(x + width / 2, y - height / 2);
	glVertex2f(x + width / 2, y + height / 2);
	glVertex2f(x - width / 2, y + height / 2);
	glEnd();
}

void DrawMap() {
	float pixelPerField;
	pixelPerField = std::min((float)screenWidth / map.GetWidth(), (float)screenHeight*divison / map.GetHeight());

	float offx = pixelPerField / 2;
	float offy = pixelPerField / 2;

	for (int x = 0; x < map.GetWidth(); x++) {
		for (int y = 0; y < map.GetHeight(); y++) {
			glColor3f(0, 0, 0);
			DrawQuad(offx + x*pixelPerField, offy + y*pixelPerField, pixelPerField, pixelPerField);
			switch (map(x,y).type)
			{
				case Map::Field::FREE:
					glColor3f(0.3, 0.48, 0.1);
					break;
				case Map::Field::MINE:
					glColor3f(0.7, 0.1, 0.1);
					break;
				case Map::Field::WALL:
					glColor3f(0.15, 0.16, 0.22);
					break;
				case Map::Field::FINISH:
					glColor3f(0.1, 1.0, 0.15);
					break;
				default:
					glColor3f(1, 1, 1);
					break;
			}
			if (x == 0 && y == 0) {
				glColor3f(0.2, 0.2, 0.8);
			}
			DrawQuad(offx + x*pixelPerField, offy + y*pixelPerField, pixelPerField*.9f, pixelPerField*.9f);
		}
	}

}

void DrawGraph() {

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(0, screenHeight*divison);
	glVertex2f(screenWidth, screenHeight * divison);
	glVertex2f(screenWidth, screenHeight);
	glVertex2f(0, screenHeight);
	glEnd();

	if (vec.size() == 0)
		return;

	float minElement;
	float maxElement;

	minElement = vec[0];
	maxElement = vec[0];

	for (float value : vec) {
		if (value < minElement) minElement = value;
	}

	for (float value : vec) {
		if (value > maxElement) maxElement = value;
	}

	glBegin(GL_LINE_STRIP);
	float x = 0.0;
	for (float value : vec) {
		x += screenWidth / (float)vec.size();
		glColor3f(0, 0, 1);
		glVertex2f(x, screenHeight - (screenHeight*(1 - divison) / (maxElement - minElement) * (value - minElement)));
	}
	glEnd();

}


void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);
	gluOrtho2D(0, screenWidth, screenHeight, 0);

	map.Generate(5, 5);
	map(9, 9).type = Map::Field::FINISH;
	map(0, 0).type = Map::Field::FREE;


}

void onDisplay() {
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawMap();

	DrawGraph();

	glutSwapBuffers();
}


void onKeyboard(unsigned char key, int x, int y) {

}


void onKeyboardUp(unsigned char key, int x, int y) {

}


void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		glutPostRedisplay();
}


void onMouseMotion(int x, int y)
{

}


void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
}


void onReshape(GLint newWidth, GLint newHeight) {
	screenWidth = newWidth;
	screenHeight = newHeight;

	glViewport(0, 0, screenWidth, screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, screenHeight, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	glutCreateWindow("MI HF");

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	onInitialization();

	glutDisplayFunc(onDisplay);
	glutMouseFunc(onMouse);
	glutIdleFunc(onIdle);
	glutKeyboardFunc(onKeyboard);
	glutKeyboardUpFunc(onKeyboardUp);
	glutMotionFunc(onMouseMotion);
	glutReshapeFunc(onReshape);

	glutMainLoop();

	return 0;
}
