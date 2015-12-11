#include <GL/glut.h>
#include <GL/freeglut.h>
#include <algorithm>
#include <mutex>
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>
#include <iomanip>

#include "Map.h"
#include "Game.h"
#include "Agent.h"

using std::cout;
using std::endl;

int screenWidth = 800;
int screenHeight = 600;

// User interface elements.
struct UIElement {
	std::string name;
	int value;
};
UIElement uiElements[] = {
	{ "map width", 10 },
	{ "map height", 10 },
	{ "walls", 5 },
	{ "mines", 5 },
	{ "iterations", 10000 },
};
// Variables related to the user itnerface.
const int& mapWidth = uiElements[0].value;
const int& mapHeight = uiElements[1].value;
const int& numWalls = uiElements[2].value;
const int& numMines = uiElements[3].value;
volatile const int& numIterations = uiElements[4].value;
int activeUIElement = 0;
const int numUIElements = sizeof(uiElements) / sizeof(uiElements[0]);
volatile bool slowMotion = false;
const float divison = 0.7;
// Modified by the teaching session, required to display the progress bar.
volatile int currentIteration = 0;
volatile int teachingIterationCount = numIterations;

// The history of reward improvements over a teaching session.
std::vector<float> rewardHistory;
std::vector<float> rewardHistoryLowpass;

// The core of the whole game environment.
Map map(2, 2);
Game game;
Agent agent;

// Stuff for the teaching thread.
std::thread teachThread;
std::mutex mtx;
volatile bool runTeach = true;
volatile bool finished = false;
std::unique_ptr<volatile float[]> Q_values;
volatile float Q_min = -1, Q_max = 1.0;

/// Computes the color coding for utility values.
/// Maps utilities on a smooth scale from blue to red.
/// \param min Lowest end of the scale.
/// \param max Highest end of the scale.
/// \param r [output] R value of the resulting color.
/// \param g [output] G value of the resulting color.
/// \param b [output] B value of the resulting color.
void UtilityToColor(float utility, float min, float max, float& r, float& g, float& b) {
	float t = (utility - min) / (max - min);
	t = std::min(1.0f, std::max(0.0f, t)); // clamp to [0..1]
	static constexpr float table[5][3] = {
		{0,0,1},
		{0,1,1},
		{0,1,0},
		{1,1,0},
		{1,0,0},
	};
	int index1 = floor(t * 4) + 0.1;
	int index2 = ceil(t * 4) + 0.1;
	float ts = t * 4 - floor(t * 4);
	index1 = std::min(4, std::max(0, index1));
	index2 = std::min(4, std::max(0, index2));
	r = (1 - ts)*table[index1][0] + ts*table[index2][0];
	g = (1 - ts)*table[index1][1] + ts*table[index2][1];
	b = (1 - ts)*table[index1][2] + ts*table[index2][2];
}

/// Performs a teaching session of the agent.
/// Teaches the agent by playing a given number of episodes.
/// Puts the results in rewardHistory.
void TeachAgent() {
	cout << "teaching started..." << endl;

	// initalization
	Q_max = 1;
	Q_min = 0;

	agent.Reset();
	game.SetMap(&map);
	agent.SetGame(&game);

	mtx.lock();
	rewardHistory.clear();

	teachingIterationCount = numIterations;
	currentIteration = 0;

	rewardHistory.resize(teachingIterationCount);
	mtx.unlock();

	// teaching cycle
	while (currentIteration < teachingIterationCount && runTeach) {
		// play an episode
		game.NewGame();
		agent.StartEpisode();
		while (!game.Ended() && runTeach) {
			agent.Step();
			if (slowMotion) {
				float minq = 0;
				float maxq = 1;
				for (size_t x = 0; x < map.GetWidth(); x++) {
					for (size_t y = 0; y < map.GetHeight(); y++) {
						float value = agent.GetQMax(x, y);
						minq = std::min(value, minq);
						maxq = std::max(value, maxq);
						Q_values[x + y*map.GetWidth()] = value;
					}
				}
				Q_min = minq;
				Q_max = maxq;
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		}
		float reward = agent.EndEpisode();

		// update results after each episode
		rewardHistory[currentIteration] = reward;

		float minq = 0;
		float maxq = 1;
		for (size_t x = 0; x < map.GetWidth(); x++) {
			for (size_t y = 0; y < map.GetHeight(); y++) {
				float value = agent.GetQMax(x, y);
				minq = std::min(value, minq);
				maxq = std::max(value, maxq);
				Q_values[x + y*map.GetWidth()] = value;
			}
		}
		Q_min = minq;
		Q_max = maxq;

		// iteration finished
		currentIteration++;
		//cout << "iteration " << currentIteration << " finished: r = " << reward << endl;
	}

	finished = true;
	cout << "teaching finished!" << endl << endl;;
}

/// Draw a single axis-aligned rectangle on the screen.
/// \param x X coordinate of the center.
/// \param y Y coordinate of the center.
/// \param width Width of the rectangle.
/// \param height Height of the rectangle.
void DrawQuad(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
	glVertex2f(x - width / 2, y - height / 2);
	glVertex2f(x + width / 2, y - height / 2);
	glVertex2f(x + width / 2, y + height / 2);
	glVertex2f(x - width / 2, y + height / 2);
	glEnd();
}

/// Draws the game map, the agent, the utility-color-coded frames and utility texts.
void DrawMap() {
	float pixelPerField;
	pixelPerField = std::min((float)(screenWidth - 200) / map.GetWidth(), (float)(screenHeight*divison) / map.GetHeight());

	float offx = pixelPerField / 2;
	float offy = pixelPerField / 2;

	float min = Q_min;
	float max = Q_max;

	// draw fields and frames
	for (int x = 0; x < map.GetWidth(); x++) {
		for (int y = 0; y < map.GetHeight(); y++) {
			glColor3f(0, 0, 0);
			float cx = offx + x*pixelPerField;
			float cy = offy + (map.GetHeight() - y - 1)*pixelPerField;

			// frame with color coding
			float r, g, b;
			UtilityToColor(Q_values[x + y*map.GetWidth()], min, max, r, g, b);
			glColor3f(r, g, b);
			DrawQuad(cx, cy, pixelPerField, pixelPerField);

			// field
			switch (map(x, y).type)
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
			DrawQuad(cx, cy, pixelPerField*.9f, pixelPerField*.9f);
		}
	}


	// display Q table as text
	glColor3f(0, 0, 0);
	for (int x = 0; x < map.GetWidth(); x++) {
		for (int y = 0; y < map.GetHeight(); y++) {
			float cx = offx + x*pixelPerField;
			float cy = offy + (map.GetHeight() - y - 1)*pixelPerField;

			// q values
			glRasterPos2f(cx - pixelPerField*0.4, cy);
			glColor3f(0.0f, 0.0f, 0.0f);
			std::stringstream ss;
			ss << std::setprecision(2);
			ss << Q_values[x + y*map.GetWidth()];
			std::string s = ss.str();
			std::unique_ptr<unsigned char[]> buffer(new unsigned char[s.size() + 1]);
			for (size_t i = 0; i < s.size(); i++) {
				buffer[i] = s[i];
			}
			buffer[s.size()] = '\0';
			glutBitmapString(GLUT_BITMAP_HELVETICA_10, buffer.get());
		}
	}

	// draw agent position
	glColor3f(0.95, 0.9, 0.8);
	DrawQuad(offx + pixelPerField * game.GetCurrentX(),
			 offy + pixelPerField * (map.GetHeight() - 1 - game.GetCurrentY()),
			 pixelPerField / 2,
			 pixelPerField / 2);
}

/// Draws the graph which shows the improvement over iterations.
void DrawGraph() {

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(0, screenHeight*divison);
	glVertex2f(screenWidth, screenHeight * divison);
	glVertex2f(screenWidth, screenHeight);
	glVertex2f(0, screenHeight);
	glEnd();

	std::lock_guard<std::mutex> lk(mtx);
	size_t numItems = currentIteration + 1;
	if (rewardHistory.size() == 0) {
		numItems = 0;
	}

	if (numItems == 0)
		return;

	float minElement;
	float maxElement;
	minElement = rewardHistory[0];
	maxElement = rewardHistory[0];

	for (size_t i = 0; i < numItems; i++) {
		float value = rewardHistory[i];
		if (value < minElement)
			minElement = value;
		if (value > maxElement)
			maxElement = value;
	}

	// draw raw line
	glBegin(GL_LINE_STRIP);
	glLineWidth(1.0f);
	glColor3f(0, 0, 1);
	for (size_t i = 0; i < numItems; i++) {
		float value = rewardHistory[i];
		float x = i*(screenWidth / (float)numItems);
		++i;
		glVertex2f(x, screenHeight - (screenHeight*(1 - divison) / (maxElement - minElement) * (value - minElement)));
	}
	glEnd();

	// draw filtered line
	glColor3f(1, 0, 0);
	glBegin(GL_LINE_STRIP);
	glLineWidth(5.0f);
	for (size_t i = 0; i < rewardHistoryLowpass.size(); i++) {
		float x = i*(screenWidth / (float)numItems);
		glVertex2f(x, screenHeight - (screenHeight*(1 - divison) / (maxElement - minElement) * (rewardHistoryLowpass[i] - minElement)));
	}
	glEnd();

	// recaulculate filtered line
	static size_t lastRecalcSize = 0;
	if (numItems % 20 || (numItems == teachingIterationCount && lastRecalcSize != teachingIterationCount)) {
		lastRecalcSize = numItems;
		rewardHistoryLowpass.resize(numItems);
		intptr_t size = (intptr_t)numItems;
		for (intptr_t i = 0; i < numItems; i++) {
			float x = i*(screenWidth / (float)numItems);
			glColor3f(1, 0, 0);

			// sinc filter with a main bump of -4..4
			constexpr float spread = 1 / 4.0f;
			constexpr float pi_rec = 1 / 3.14159265f;
			float y = rewardHistory[i] * spread * pi_rec;
			float wt = spread * pi_rec;
			for (intptr_t filter = 1; filter < 30; filter++) {
				intptr_t sample1 = i + filter;
				intptr_t sample2 = i - filter;
				sample1 = std::max(0, std::min(size - 1, sample1));
				sample2 = std::max(0, std::min(size - 1, sample2));
				float w = (sin(filter*spread) / (filter*spread)) * spread * pi_rec;
				wt += w * 2;
				y += (rewardHistory[sample1] + rewardHistory[sample2]) * w;
			}
			y /= wt;

			rewardHistoryLowpass[i] = y;
		}
	}
}

/// Draws the user interface on the upper right.
void DrawUI() {
	float pixelPerField;
	pixelPerField = std::min((float)(screenWidth - 200) / map.GetWidth(), (float)(screenHeight*divison) / map.GetHeight());

	float offx = pixelPerField * map.GetWidth() + 10;

	// vertical offset index
	int i;

	// draw ui elements
	for (i = 0; i < numUIElements; ++i) {
		if (i == activeUIElement) {
			glColor3f(0.8f, 0.2f, 0.2f);
		}
		else {
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		glRasterPos2f(offx, 20+i*20);

		std::stringstream ss;
		ss << uiElements[i].name << " = " << uiElements[i].value;

		std::string s = ss.str();
		std::unique_ptr<unsigned char[]> buffer(new unsigned char[s.size() + 1]);
		for (size_t i = 0; i < s.size(); i++) {
			buffer[i] = s[i];
		}
		buffer[s.size()] = '\0';

		glutBitmapString(GLUT_BITMAP_HELVETICA_18, buffer.get());
	}

	// draw progress bar
	glColor3f(0.5, 0.5, 0.5);
	DrawQuad(offx + 90, 20 + i * 20, 180, 15);
	glColor3f(0.9, 0.2, 0.2);
	DrawQuad(offx + (float)currentIteration / teachingIterationCount * 0.5f * 180,
			 20 + i * 20,
			 (float)currentIteration / teachingIterationCount * 180,
			 15);
}

/// Launches a teaching session.
/// Note that it is launched in a new thread.
void StartTeaching() {
	if (finished && teachThread.joinable()) {
		teachThread.join();
	}
	if (!teachThread.joinable()) {
		runTeach = true;
		finished = false;
		teachThread = std::thread(TeachAgent);
	}
}

/// Cancels the currently running teaching session.
/// Kills it's thread, gracefully.
void CancelTeaching() {
	runTeach = false;
	finished = false;
	if (teachThread.joinable()) {
		teachThread.join();
	}
}

/// Create a map according to the parameters specified.
/// Also adds a start and a finish.
void CreateMap(int width, int height, int walls, int mines) {
	// restrict width & height
	width = std::max(2, width);
	height = std::max(2, height);

	// create the map
	map.Resize(width, height);
	map.Generate(walls, mines);
	map(map.GetWidth() - 1, map.GetHeight() - 1).type = Map::Field::FINISH;
	map(0, 0).type = Map::Field::FREE;

	// create table for Q values
	Q_values.reset(new volatile float[map.GetWidth()*map.GetHeight()]);
	for (size_t i = 0; i < map.GetWidth()*map.GetHeight(); ++i) {
		Q_values[i] = 0;
	}
}

/// Initializes OpenGL and stuff like that.
/// GLUT calls this at app start. 
void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);
	gluOrtho2D(0, screenWidth, screenHeight, 0);

	CreateMap(mapWidth, mapHeight, numWalls, numMines);
}

/// Redraws the entire OpenGL frame.
void onDisplay() {
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawMap();
	DrawGraph();
	DrawUI();

	glutSwapBuffers();
}


/// Handles GLUT keyboard events.
void onKeyboard(unsigned char key, int x, int y) {
	// teaching and map
	// teach
	if (key == 't') {
		StartTeaching();
	}
	// cancel
	if (key == 'c') {
		CancelTeaching();
	}
	// regenerate map
	if (key == 'r') {
		CancelTeaching();
		agent.Reset();
		CreateMap(mapWidth, mapHeight, numWalls, numMines);
	}
	// slow-motion toggle
	if (key == 'z') {
		slowMotion = !slowMotion;
	}

	// parameters menu
	// up
	if (key == 's') {
		activeUIElement = (activeUIElement + 1) % numUIElements;
	}
	// down
	if (key == 'w') {
		activeUIElement = (activeUIElement - 1);
		if (activeUIElement < 0) {
			activeUIElement = numUIElements - 1;
		}
	}
	// decrease
	if (key == 'a') {
		if (uiElements[activeUIElement].name == "iterations") {
			uiElements[activeUIElement].value/=10;
			if (uiElements[activeUIElement].value < 1) {
				uiElements[activeUIElement].value = 1;
			}
		}
		else {
			uiElements[activeUIElement].value--;
		}
	}
	// increase
	if (key == 'd') {
		if (uiElements[activeUIElement].name == "iterations") {
			if (uiElements[activeUIElement].value < 100000000) {
				uiElements[activeUIElement].value *= 10;
			}
		}
		else {
			uiElements[activeUIElement].value++;
		}
	}
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
	glutPostRedisplay();
}

/// Handles the resizing of the main window.
void onReshape(GLint newWidth, GLint newHeight) {
	screenWidth = newWidth;
	screenHeight = newHeight;

	glViewport(0, 0, screenWidth, screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, screenHeight, 0);
}

/// This function is to be registered with atexit() to clean up.
/// Required because GLUT can go fuck itself.
void CleanupOnExit() {
	runTeach = false;
	if (teachThread.joinable()) {
		teachThread.join();
	}
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

	atexit(CleanupOnExit);

	glutMainLoop();

	return 0;
}
