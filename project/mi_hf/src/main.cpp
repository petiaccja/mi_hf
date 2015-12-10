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

static constexpr int mapWidth = 10, mapHeight = 10;


const float divison = 0.7;
std::vector<float> rewardHistory;
std::vector<float> rewardHistoryLowpass;

std::mutex mtx;

Map map(mapWidth, mapHeight);
volatile float Q_values[mapWidth][mapHeight];
volatile float Q_min = -1, Q_max = 1.0;
Game game;
Agent agent;
std::thread teachThread;
volatile bool runTeach = true;
volatile bool finished = false;


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
	float ts = t - floor(t);
	index1 = std::min(4, std::max(0, index1));
	index2 = std::min(4, std::max(0, index2));
	r = (1 - ts)*table[index1][0] + ts*table[index2][0];
	g = (1 - ts)*table[index1][1] + ts*table[index2][1];
	b = (1 - ts)*table[index1][2] + ts*table[index2][2];
}

void TeachAgent() {
	cout << "teaching started..." << endl;

	agent.Reset();
	game.SetMap(&map);
	agent.SetGame(&game);

	mtx.lock();
	rewardHistory.clear();
	mtx.unlock();

	int numIterations = 10000;
	int i = 0;

	while (numIterations > 0 && runTeach) {
		game.NewGame();
		agent.StartEpisode();
		while (!game.Ended() && runTeach) {
			agent.Step();

			int x = game.GetCurrentX();
			int y = game.GetCurrentY();

			float value = agent.GetQMax(x, y);
			float max = Q_max;
			float min = Q_min;
			Q_values[x][y] = value;

			Q_max = std::max(max, value);
			Q_min = std::min(min, value);

			//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		float reward = agent.EndEpisode();
		mtx.lock();
		rewardHistory.push_back(reward);
		mtx.unlock();
		numIterations--;
		cout << "iteration " << ++i << " finished: r = " << reward << endl;
	}
	finished = true;
	cout << "teaching finished!" << endl << endl;;
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

	float min = Q_min;
	float max = Q_max;
	for (int x = 0; x < map.GetWidth(); x++) {
		for (int y = 0; y < map.GetHeight(); y++) {
			glColor3f(0, 0, 0);
			float cx = offx + x*pixelPerField;
			float cy = offy + (map.GetHeight() - y - 1)*pixelPerField;

			// frame with color coding
			float r, g, b;
			UtilityToColor(Q_values[x][y], min, max, r, g, b);
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
			ss << Q_values[x][y];
			std::string s = ss.str();
			std::unique_ptr<unsigned char[]> buffer(new unsigned char[s.size() + 1]);
			for (size_t i = 0; i < s.size(); i++) {
				buffer[i] = s[i];
			}
			buffer[s.size()] = '\0';
			glutBitmapString(GLUT_BITMAP_HELVETICA_10, buffer.get());
		}
	}

	glColor3f(0.95, 0.9, 0.8);
	DrawQuad(offx + pixelPerField * game.GetCurrentX(),
			 offy + pixelPerField * (map.GetHeight() - 1 - game.GetCurrentY()),
			 pixelPerField / 2,
			 pixelPerField / 2);

	for (int i = 0; i <= 20; i++) {

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

	if (rewardHistory.size() == 0)
		return;

	float minElement;
	float maxElement;

	minElement = rewardHistory[0];
	maxElement = rewardHistory[0];

	std::lock_guard<std::mutex> lk(mtx);
	for (float value : rewardHistory) {
		if (value < minElement) minElement = value;
	}

	for (float value : rewardHistory) {
		if (value > maxElement) maxElement = value;
	}


	glBegin(GL_LINE_STRIP);
	size_t i = 0;
	glColor3f(0, 0, 1);
	for (float value : rewardHistory) {
		float x = i*(screenWidth / (float)rewardHistory.size());
		++i;
		glVertex2f(x, screenHeight - (screenHeight*(1 - divison) / (maxElement - minElement) * (value - minElement)));
	}
	glEnd();


	glColor3f(1, 0, 0);
	glBegin(GL_LINE_STRIP);
	i = 0;
	for (float value : rewardHistoryLowpass) {
		float x = i*(screenWidth / (float)rewardHistory.size());
		++i;
		glVertex2f(x, screenHeight - (screenHeight*(1 - divison) / (maxElement - minElement) * (value - minElement)));
	}
	glEnd();

	if (rewardHistory.size() % 20) {
		rewardHistoryLowpass.clear();
		intptr_t size = (intptr_t)rewardHistory.size();
		for (intptr_t i = 0; i < rewardHistory.size(); i++) {
			float x = i*(screenWidth / (float)rewardHistory.size());
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

			rewardHistoryLowpass.push_back(y);
		}
	}


}


void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);
	gluOrtho2D(0, screenWidth, screenHeight, 0);

	map.Generate(5, 5);
	map(mapWidth - 1, 4).type = Map::Field::FINISH;
	map(4, mapHeight - 1).type = Map::Field::FINISH;
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
	if (key == 'p') {
		if (finished && teachThread.joinable()) {
			teachThread.join();
		}
		if (!teachThread.joinable()) {
			runTeach = true;
			finished = false;
			teachThread = std::thread(TeachAgent);
		}
	}
	if (key == 'c') {
		runTeach = false;
		finished = false;
		if (teachThread.joinable()) {
			teachThread.join();
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


void onReshape(GLint newWidth, GLint newHeight) {
	screenWidth = newWidth;
	screenHeight = newHeight;

	glViewport(0, 0, screenWidth, screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, screenHeight, 0);
}


void CleanupOnExit() {
	runTeach = false;
	if (teachThread.joinable()) {
		teachThread.join();
	}
}


int main(int argc, char **argv) {
	for (auto& u : Q_values) {
		for (auto& v : u) {
			v = 0;
		}
	}

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
